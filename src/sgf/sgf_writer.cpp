#include "sgf_writer.h"
#include <fstream>

static std::string moveToSGFCoord(const Move& move){
    if (move.isPass){
        return "" ;
    }

    std::string coord ;
    coord += static_cast<char>('a' + move.x) ;
    coord += static_cast<char>('a' + move.y) ;

    return coord ;
}

bool SGFWriter::saveMainLine(const std::string& filename , const std::vector<Move>& moves){
    std::ofstream fout(filename);
    if (!fout) {
        return false;
    }

    fout << "(;GM[1]FF[4]SZ[19]\n";

    for (const auto& move : moves) {
        if (move.stone == Stone::BLACK) {
            fout << ";B[" << moveToSGFCoord(move) << "]\n";
        }
        else if (move.stone == Stone::WHITE) {
            fout << ";W[" << moveToSGFCoord(move) << "]\n";
        }
    }

    fout << ")\n";
    return true;
}

