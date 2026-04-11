#pragma once

#include <vector>
#include <random>

#include "../core/Game.h"
#include "MCTSNode.h"

class MCTS{
public:
    MCTS(int iter = 200) ;
    Move getbestMove(const Game &game) ;

private:
    int iteration ;
    std::mt19937 rng;

    std::vector<Move> getlegalMoves(const Game &game) ;
    Move getRandomMove(const std::vector<Move> &moves) ;

    MCTSNode* selectNode(MCTSNode* node) ;
    MCTSNode* expandNode(MCTSNode* node) ;
    double simulate(Game &game , Stone currentPlayer) ;
    void backpropagate(MCTSNode* node , double result) ;

    bool isTerminal(const Game &game) ;
    double evaluate(const Game &game , Stone currentPlayer) ;
};