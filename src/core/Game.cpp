#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include "Board.h"
#include "Game.h"

bool Game::isKo(int x , int y ,Stone color){
    Board preboard = curborad ;

    if (!preboard.playMove(x,y,color)){
        return false ;
    }

    if (record.size() > 1){

        // 此时可判为打劫，所以不能这么下
        // 如果是像三劫循环， 就应该是认为判断
        if (preboard == record[record.size() - 2].boards ){
            std::cout << "wrong play\n" ;
            return false ;
        }

        curborad = preboard ;
        record.push_back({x,y,color,curborad}) ;
        return true ;
    }
}

// 处理悔棋函数
bool Game::undo(int x , int y , Stone color){
    if (record.empty()){
        return false ;
    }

    record.pop_back() ;
    if (record.empty()){
        curborad = Board() ;
    }

    else{
        curborad = record[record.size() - 1].boards ;
    }
    std::cout << "you've undone your play\n" ;
    return true ;
}