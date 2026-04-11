#pragma once
#include <vector>
#include <cmath>

#include "../core/Game.h" 
#include "../sgf/sgf_utils.h"

class MCTSNode{
public:
    MCTSNode(const Game &game ,const Move &move ,MCTSNode* parent, const std::vector<Move>& moves) ;

    Game game ;
    Move move ;
    MCTSNode* parent ;
    
    std::vector<MCTSNode*> children ;
    std::vector<Move> untriedMoves ;

    int visits ;
    double wins ;

    ~MCTSNode() ;

    bool isFullyExpanded() const ;
    bool isLeaf() const ;
    MCTSNode* getBestUCBChild(double c) const ;
};
