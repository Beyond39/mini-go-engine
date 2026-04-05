#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include "Board.h"
#include "Game.h"

Game::Game(){
    reset();
}

void Game::reset(){
    currentBoard = Board();
    currentPlayer = Stone::BLACK;
    history.clear();
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

    if (history.size() == 1 || history.empty()){
        return false ;
    }

    if (history.size() > 1){
        if (nextBoard == history[history.size() - 2].boards ){
            return true ;
        }
    }

    return false ;
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

bool Game::playPass(){
    RecordMove record;
    record.x = -1;
    record.y = -1;
    record.isPass = true;
    record.boards = currentBoard;
    record.color = currentPlayer;

    history.push_back(record);
    switchPlayer();
    return true;
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
    currentPlayer = history[history.size() - 2].color ;

    return true ;
}

const Board& Game::getBoard() const {
    return currentBoard;
}

Stone Game::getCurrentPlayer() const {
    return currentPlayer;
}

const std::vector<RecordMove>& Game::getHistory() const{
    return history;
}

bool Game::canUndo() const{
    return !history.empty();
}
