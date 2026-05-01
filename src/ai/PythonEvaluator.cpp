#include "PythonEvaluator.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QStringList>

PythonEvaluator::PythonEvaluator(const QString& pythonExe, const QString& scriptPath, const QString& modelPath)
    : pythonExe(pythonExe),
      scriptPath(scriptPath),
      modelPath(modelPath),
      process(nullptr)
{
}

PythonEvaluator::~PythonEvaluator(){
    stop();
}

bool PythonEvaluator::start(){
    if (process != nullptr && process->state() == QProcess::Running) {
        return true;
    }

    if (!QFileInfo::exists(pythonExe)) {
        qDebug() << "Python 解释器不存在:" << pythonExe;
        return false;
    }

    if (!QFileInfo::exists(scriptPath)) {
        qDebug() << "Python 推理脚本不存在:" << scriptPath;
        return false;
    }

    if (!QFileInfo::exists(modelPath)) {
        qDebug() << "模型文件不存在:" << modelPath;
        return false;
    }

    process = new QProcess();
    QStringList args;
    args << scriptPath << modelPath;

    process->start(pythonExe, args);

    if (!process->waitForStarted(5000)) {
        qDebug() << "Python 进程启动失败:" << process->errorString();
        delete process;
        process = nullptr;
        return false;
    }

    qDebug() << "Python 推理进程启动成功";
    return true;
}

void PythonEvaluator::stop(){
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

QString PythonEvaluator::boardToMessage(const Board& board, int currentPlayer) const{
    QString msg;
    msg += QString::number(currentPlayer);
    msg += ";";

    for (int x = 0; x < Board::SIZE; ++x) {
        for (int y = 0; y < Board::SIZE; ++y) {
            int v = 0;
            Stone s = board.get(x, y);

            if (s == Stone::BLACK) {
                v = 1;
            } else if (s == Stone::WHITE) {
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

QByteArray PythonEvaluator::readLineWithTimeout(int timeoutMs)
{
    if (!process) {
        return QByteArray();
    }

    QElapsedTimer timer;
    timer.start();

    while (!process->canReadLine() && timer.elapsed() < timeoutMs) {
        process->waitForReadyRead(50);
    }

    if (!process->canReadLine()) {
        return QByteArray();
    }

    return process->readLine().trimmed();
}

EvaluationResult PythonEvaluator::evaluate(const Board& board, int currentPlayer){
    EvaluationResult result;

    if (!process || process->state() != QProcess::Running) {
        if (!start()) {
            return result;
        }
    }

    const QString message = boardToMessage(board, currentPlayer);

    process->write(message.toUtf8());
    if (!process->waitForBytesWritten(2000)) {
        qDebug() << "写入 Python 失败:" << process->errorString();
        stop();
        return result;
    }
    
    if (!process->waitForReadyRead(5000)) {
        qDebug() << "等待 value 超时";
        qDebug() << process->readAllStandardError();
        return result;
    }

    const QByteArray line1 = readLineWithTimeout(20000);
    if (line1.isEmpty()) {
        qDebug() << "等待 value 超时或 Python 已退出";
        qDebug() << process->readAllStandardError();
        stop();
        return result;
    }

    bool ok = false;
    const float value = QString::fromUtf8(line1).toFloat(&ok);
    if (!ok) {
        qDebug() << "value 解析失败:" << line1;
        qDebug() << process->readAllStandardError();
        stop();
        return result;
    }

    const QByteArray line2 = readLineWithTimeout(20000);
    if (line2.isEmpty()) {
        qDebug() << "等待 policy 超时或 Python 已退出";
        qDebug() << process->readAllStandardError();
        stop();
        return result;
    }

    const QList<QByteArray> parts = line2.split(' ');
    if (parts.size() != 362) {
        qDebug() << "policy 长度异常:" << parts.size();
        qDebug() << process->readAllStandardError();
        return result;
    }

    std::vector<float> policy;
    policy.reserve(parts.size());

    for (const QByteArray& p : parts) {
        bool ok2 = false;
        const float prob = QString::fromUtf8(p).toFloat(&ok2);
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
 