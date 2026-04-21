#pragma once

#include <QString>
#include "../core/Board.h"

class BoardSerializer
{
public:
    static bool saveToFile(const Board& board, int currentPlayer, const QString& filePath);
};
