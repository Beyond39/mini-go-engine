#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include "Board.h"
#include "Game.h"
#include "sgf/sgf_utils.h"

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

    if (history.size() < 2) {
        return false;
    }

    return nextBoard == history[history.size() - 2].boards;
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
        currentPlayer = Stone::BLACK;
        return true;
    }

    currentBoard = history.back().boards;
    currentPlayer = currentBoard.opponent(history.back().color);
    return true;
}

bool Game::loadFromMoves(const std::vector<Move>& moves){
    reset() ;

    for (const auto& move : moves){
        if (move.isPass){
            if (!playPass()){
                return false ;
            }
        }
        else{
            if (!playMove(move.x,move.y)){
                return false ;
            }
        }
    }

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

std::vector<Move> convertHistoryToMoves(const std::vector<RecordMove>& history){
    std::vector<Move> moves ;

    for (const auto& record : history){
        Move move ;
        move.stone = record.color ;
        move.isPass = record.isPass ;

        if (record.isPass){
            move.x = -1 ;
            move.y = -1 ;
        }
        else{
            move.x = record.x ;
            move.y = record.y ;
        }

        moves.push_back(move) ;
    }

    return moves ;
}