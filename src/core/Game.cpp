#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include "Board.h"
#include "Game.h"

Game::Game(){
    currentBoard = Board() ;
    currentPlayer = Stone::BLACK ;
}

void Game::switchPlayer() {
    if (currentPlayer == Stone::BLACK) currentPlayer = Stone::WHITE;
    else currentPlayer = Stone::BLACK;
}

bool Game::isKo(int x , int y ,Stone color) const{
    Board nextBoard = Game::currentBoard ;

    if (!nextBoard.playMove(x,y,color)){
        return false ;
    }

    if (history.size() > 1){

        // 此时可判为打劫，所以不能这么下
        // 如果是像三劫循环， 就应该是认为判断
        if (nextBoard == history[history.size() - 2].boards ){
            return true ;
        }

        return false ;
    }
}

bool Game::playMove(int x, int y){
    if (isKo(x,y,currentPlayer)){
        return false ;
    }

    if (!currentBoard.playMove(x,y,currentPlayer)){
        return false ;
    }

    RecordMove record ;
    record.x = x ;
    record.y = y ;
    record.boards = currentBoard ;
    record.color = currentPlayer ;

    history.push_back(record) ;
    switchPlayer() ;

    return true ;
}
// 处理悔棋函数
bool Game::undo(){
    if (history.empty()){
        return false ;
    }

    history.pop_back() ;
    if (history.empty()){
        currentBoard = Board() ;
    }

    else{
        currentBoard = history[history.size() - 1].boards ;
    }
    currentPlayer = history[history.size() - 1].color ;

    return true ;
}

const Board& Game::getBoard() const {
    return currentBoard;
}

Stone Game::getCurrentPlayer() const {
    return currentPlayer;
}