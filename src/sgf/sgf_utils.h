#pragma once

#include "sgf_parser.h"
#include "core/Board.h"
#include <vector>
struct Move {
    int x = -1;
    int y = -1;
    Stone stone = Stone::EMPTY;
    bool isPass = false;
};

// 判断一个节点是不是一步棋，并解析出来
bool extractMoveFromNode(SGFNode* node, Move& move);
std::vector<Move> extractMainLine(SGFNode* root);