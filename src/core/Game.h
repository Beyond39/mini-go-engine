#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include "Board.h"

// 记录落子的函数，为了防止打劫的时候出现问题
struct RecordMove{
    int x ;
    int y ;
    Stone color ;
    Board boards ;
};

// 在Game类里面，我需要实现围棋的特殊的一些功能，比如打劫之类的实现
// 还有就是需要记录，暂时不去使用zobirsh hash 
class Game {
public :
    Game() ;
    // 下棋
    bool playMove(int x, int y);

    // 悔棋
    bool undo();

    // 简单对打劫进行判断
    bool isKo(int x, int y, Stone color) const;

    // 获取当前棋盘
    const Board& getBoard() const;

    // 获取当前该谁下
    Stone getCurrentPlayer() const;
private:
    Board currentBoard;
    Stone currentPlayer;
    std::vector<RecordMove> history;

    void switchPlayer();
};