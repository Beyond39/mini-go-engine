#pragma once

#include <QString>
#include <QProcess>
#include "EvaluationResult.h"
#include "../core/Board.h"

class PythonEvaluator
{
public:
    PythonEvaluator(const QString& pythonExe, const QString& scriptPath, const QString& modelPath);
    ~PythonEvaluator();

    bool start();
    void stop();
    EvaluationResult evaluate(const Board& board, int currentPlayer);

private:
    QString pythonExe;
    QString scriptPath;
    QString modelPath;

    QProcess* process;

    QString boardToMessage(const Board& board, int currentPlayer) const;
    QByteArray readLineWithTimeout(int timeoutMs);
};
