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
      modelPath(modelPath)
{
}

EvaluationResult PythonEvaluator::evaluate(const Board& board, int currentPlayer)
{
    EvaluationResult result;

    QString tempFilePath = QDir::temp().filePath("goengine_board_input.txt");
    if (!BoardSerializer::saveToFile(board, currentPlayer, tempFilePath))
        return result;

    QProcess process;
    QStringList args;
    args << scriptPath << tempFilePath << modelPath;

    process.start(pythonExe, args);
    if (!process.waitForFinished(10000))
        return result;

    QByteArray stdOut = process.readAllStandardOutput().trimmed();
    QByteArray stdErr = process.readAllStandardError().trimmed();

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return result;

    bool ok = false;
    float value = QString::fromUtf8(stdOut).toFloat(&ok);
    if (!ok) {
        return result;
    }
    result.value = value;
    result.valid = true;
    return result;
}