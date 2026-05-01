#include "MCTSNode.h" 

MCTSNode::MCTSNode(const Game &game ,const Move &move ,MCTSNode* parent, const std::vector<Move>& moves, double prob)
        :game(game) ,
         move(move) ,
         parent(parent) ,
         children() ,
         visits(0) ,
         wins(0.0) , 
         untriedMoves(moves) ,
         policy_prob(prob)
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

MCTSNode* MCTSNode::getBestPUCTChild(double c_puct) const{
    MCTSNode* bestChild = nullptr;
    double bestValue = -1e100;

    double parentVisits = std::max(1, visits);

    for (MCTSNode* child : children) {
        double q_value = 0.0;

        if (child->visits > 0) {
            q_value = child->wins / child->visits;
        }

        double u_value =
            c_puct *
            child->policy_prob *
            std::sqrt(parentVisits) /
            (1.0 + child->visits);

        double puctValue = q_value + u_value;

        if (puctValue > bestValue) {
            bestValue = puctValue;
            bestChild = child;
        }
    }

    return bestChild;
}