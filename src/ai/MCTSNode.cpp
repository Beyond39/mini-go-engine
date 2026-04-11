#include "MCTSNode.h" 

MCTSNode::MCTSNode(const Game &game ,const Move &move ,MCTSNode* parent, const std::vector<Move>& moves)
        :game(game) ,
         move(move) ,
         parent(parent) ,
         visits(0) ,
         wins(0.0) , 
         untriedMoves(moves)
{
}

MCTSNode::~MCTSNode(){
    for (MCTSNode* child : children){
        delete child ;
    }
    children.clear() ;
}

bool MCTSNode::isFullyExpanded() const{
    return untriedMoves.empty() ;
}

bool MCTSNode::isLeaf() const{
    return children.empty() ;
}

MCTSNode* MCTSNode::getBestUCBChild(double c) const{
    MCTSNode* bestChild = nullptr;
    double bestValue = -1e100;

    for (MCTSNode* child : children) {
        if (child->visits == 0) {
            return child;
        }

        double exploit = child->wins / child->visits;
        double explore = c * std::sqrt(std::log((double)visits) / child->visits);
        double ucbValue = exploit + explore;

        if (ucbValue > bestValue) {
            bestValue = ucbValue;
            bestChild = child;
        }
    }

    return bestChild;
}