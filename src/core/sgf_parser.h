#pragma once

#include <vector>
#include <string>
#include "Board.h"

struct Move {
    int x;
    int y;
    Stone stone ;
};

class SGFParser {
public:
    static std::vector<Move> parse(const std::string& filename);

};
