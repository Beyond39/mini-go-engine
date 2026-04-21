#pragma once

#include <QString>
#include "EvaluationResult.h"
#include "../core/Board.h"

class PythonEvaluator
{
public:
    PythonEvaluator(const QString& pythonExe,
                    const QString& scriptPath,
                    const QString& modelPath);

    EvaluationResult evaluate(const Board& board, int currentPlayer);

private:
    QString pythonExe;
    QString scriptPath;
    QString modelPath;
};