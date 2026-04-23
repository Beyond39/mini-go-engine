#include "PythonEvaluator.h"
#include "BoardSerializer.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QStringList>

PythonEvaluator::PythonEvaluator(const QString& pythonExe, const QString& scriptPath, const QString& modelPath)
    : pythonExe(pythonExe),
      scriptPath(scriptPath),
      modelPath(modelPath),
      process(nullptr)
{
}

PythonEvaluator::~PythonEvaluator()
{
    stop();
}

bool PythonEvaluator::start()
{
    if (process != nullptr) {
        return true;
    }

    process = new QProcess();
    QStringList args;
    args << scriptPath << modelPath;

    process->start(pythonExe, args);

    if (!process->waitForStarted(5000)) {
        qDebug() << "Python 进程启动失败";
        delete process;
        process = nullptr;
        return false;
    }

    qDebug() << "Python 推理进程启动成功";
    return true;
}

void PythonEvaluator::stop()
{
    if (process) {
        process->closeWriteChannel();
        process->terminate();

        if (!process->waitForFinished(2000)) {
            process->kill();
            process->waitForFinished(1000);
        }

        delete process;
        process = nullptr;
    }
}

QString PythonEvaluator::boardToMessage(const Board& board, int currentPlayer) const
{
    // 格式：
    // currentPlayer;361个点（0空1黑2白）
    // 例如：
    // 1;0,0,1,2,0,...
    QString msg;
    msg += QString::number(currentPlayer);
    msg += ";";

    for (int x = 0; x < Board::SIZE; ++x) {
        for (int y = 0; y < Board::SIZE; ++y) {
            int v = 0;
            Stone s = board.get(x, y);

            if (s == Stone::BLACK) {
                v = 1;
            }
            else if (s == Stone::WHITE) {
                v = 2;
            }

            msg += QString::number(v);

            if (!(x == Board::SIZE - 1 && y == Board::SIZE - 1)) {
                msg += ",";
            }
        }
    }

    msg += "\n";
    return msg;
}

EvaluationResult PythonEvaluator::evaluate(const Board& board, int currentPlayer)
{
    EvaluationResult result;

    if (!process) {
        qDebug() << "Python 进程未启动";
        return result;
    }

    QString message = boardToMessage(board, currentPlayer);

    process->write(message.toUtf8());
    if (!process->waitForBytesWritten(2000)) {
        qDebug() << "写入 Python 失败";
        return result;
    }
    
    if (!process->waitForReadyRead(5000)) {
        qDebug() << "等待 value 超时";
        qDebug() << process->readAllStandardError();
        return result;
    }

    QByteArray line1 = process->readLine().trimmed();
    bool ok = false;
    float value = QString::fromUtf8(line1).toFloat(&ok);
    if (!ok) {
        qDebug() << "value 解析失败:" << line1;
        qDebug() << process->readAllStandardError();
        return result;
    }

    if (!process->waitForReadyRead(5000)) {
        qDebug() << "等待 policy 超时";
        qDebug() << process->readAllStandardError();
        return result;
    }

    QByteArray line2 = process->readLine().trimmed();
    QList<QByteArray> parts = line2.split(' ');

    std::vector<float> policy;
    policy.reserve(parts.size());

    for (const QByteArray& p : parts) {
        bool ok2 = false;
        float prob = QString::fromUtf8(p).toFloat(&ok2);
        if (!ok2) {
            qDebug() << "policy 解析失败:" << p;
            return result;
        }
        policy.push_back(prob);
    }

    result.value = value;
    result.policy = std::move(policy);
    result.valid = true;

    return result;
}