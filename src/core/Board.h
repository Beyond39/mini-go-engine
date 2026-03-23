#pragma once
#include <iostream>
#include <vector>

enum class Stone { 
    EMPTY = 1,
    BLACK = 2,
    WHITE = 3 
};

class Board {
public:
    static const int SIZE = 19;

    Board();
    
    // 下棋函数：不仅判断合法性，还会处理吃子和自杀规则
    bool playMove(int x, int y, Stone color);

    // 获取某点状态
    Stone get(int x, int y) const;

    // 打印棋盘
    void printBoard() const;

private:
    friend bool operator==(const Board &a , const Board &b) ;

    Stone board[SIZE][SIZE];

    // 判断是否在棋盘内
    bool inBounds(int x, int y) const;

    // 核心算法：判断一块棋是否有气 
    bool hasLiberty(int x, int y, bool visited[SIZE][SIZE]) const;

    // 核心算法：把一块被吃掉的棋子从棋盘上抹去
    int removeGroup(int x, int y, Stone color);

    // 辅助工具：获取对手颜色
    Stone opponent(Stone color) const;
};