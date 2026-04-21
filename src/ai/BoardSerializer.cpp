#include "BoardSerializer.h"

#include <QFile>
#include <QTextStream>

bool BoardSerializer::saveToFile(const Board& board, int currentPlayer, const QString& filePath){
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }

    QTextStream out(&file);
    
    out << currentPlayer << "\n";

    for (int y = 0; y < Board::SIZE; ++y)
    {
        for (int x = 0; x < Board::SIZE; ++x)
        {
            out << static_cast<int>(board.get(x, y));
            if (x != Board::SIZE - 1)
                out << ' ';
        }
        out << "\n";
    }

    return true;
}