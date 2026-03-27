#pragma once
#include <iostream>
#include <vector>

enum class Stone { 
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2 
};

class Board {
public:
    static const int SIZE = 19;

    Board();
    
    // 下棋函数：不仅判断合法性，还会处理吃子和自杀规则
    bool playMove(int x, int y, Stone color);

    // 判断某手在纯棋盘规则下是否合法(此处不包含打劫的判断)
    bool isLegalMove(int x, int y, Stone color) const;

    // 获取所有合法落子
    std::vector<std::pair<int, int>> getLegalMoves(Stone color) const;

    // 获取某点状态
    Stone get(int x, int y) const;

    // 比较棋盘局面
    bool operator==(const Board& other) const;
    bool operator!=(const Board& other) const;

    // 辅助工具：获取对手颜色
    Stone opponent(Stone color) const;

    // 打印棋盘
    void printBoard() const;

private:
    Stone board[SIZE][SIZE];

    // 判断是否在棋盘内
    bool inBounds(int x, int y) const;

    // 核心算法：判断一块棋是否有气 
    bool hasLiberty(int x, int y, bool visited[SIZE][SIZE]) const;

    // 收集整个连通块
    void collectGroup(int x, int y, Stone color,
                      bool visited[SIZE][SIZE],
                      std::vector<std::pair<int, int>>& group) const;

    // 核心算法：把一块被吃掉的棋子从棋盘上抹去
    int removeGroup(int x, int y, Stone color);

};