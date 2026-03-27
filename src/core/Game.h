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
    // 对打劫进行判断 ， 最基础的判断方式
    bool isKo(int x , int y , Stone color) ;
    // 提供悔棋操作
    bool undo(int x , int y , Stone color) ;
private:
    std::vector<RecordMove> record ;
    Board curborad ;
};