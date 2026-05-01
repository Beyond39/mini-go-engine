#pragma once

#include <vector>
#include <random>

#include "../core/Game.h"
#include "MCTSNode.h"
#include "PythonEvaluator.h"

class MCTS{
public:
    MCTS(int iter = 600 , PythonEvaluator* eval = nullptr) ;
    Move getbestMove(const Game &game) ;

private:
    int iteration ;
    std::mt19937 rng;
    PythonEvaluator* evaluator = nullptr ;

    std::vector<Move> getproperMoves(const Game &game) ;
    Move getRandomMove(const std::vector<Move> &moves) ;

    MCTSNode* selectNode(MCTSNode* node) ;
    MCTSNode* expandNode(MCTSNode* node) ;
    double simulate(Game &game , Stone currentPlayer) ;
    void backpropagate(MCTSNode* node , double result) ;

    bool hasNearbyStone(const Board& board, int x ,int y ,int radius) ;
    bool isTerminal(const Game &game) ;
    double evaluate(const Game &game , Stone currentPlayer) ;

    int moveToPolicyIndex(const Move& move) const;
    std::vector<Move> sortMovesByPolicy(const std::vector<Move>& moves, const std::vector<float>& policy) const;
};
