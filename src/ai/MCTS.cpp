#include "MCTS.h"
#include "MCTSNode.h"
#include "../core/Board.h" 

#include <algorithm>
#include <cmath>

MCTS::MCTS(int iter, PythonEvaluator* eval)
    : iteration(iter),
      evaluator(eval),
      rng(std::random_device{}())
{
}

bool MCTS::hasNearbyStone(const Board& board , int x , int y , int radius){
    for (int dx = -radius ; dx <= radius ; ++dx){
        for (int dy = -radius ; dy <= radius ; ++dy){
            if (dx == 0 && dy == 0){
                continue;
            }

            int nx = x + dx ;
            int ny = y + dy ;

            if (nx < 0 || nx >= Board::SIZE || ny < 0 || ny >= Board::SIZE) {
                continue;
            }

            if (board.get(nx, ny) != Stone::EMPTY) {
                return true;
            }
        }
    }

    return false ;
}

std::vector<Move> MCTS::getproperMoves(const Game &game){
    std::vector<Move> moves ;
    const Board board = game.getBoard() ;
    Stone player = game.getCurrentPlayer() ;

    std::vector<std::pair<int,int>> points = board.getLegalMoves(player) ;

    const std::vector<RecordMove>& history = game.getHistory();

    for (auto it : points){
        int x = it.first;
        int y = it.second;

        if (game.isKo(x, y, player)) {
            continue;
        }

        Move move ;
        move.x = it.first ;
        move.y = it.second ;
        move.stone = player ;
        move.isPass = false ;

        moves.push_back(move) ;
    }

    Move passMove;
    passMove.x = -1;
    passMove.y = -1;
    passMove.stone = player;
    passMove.isPass = true;
    moves.push_back(passMove);

    return moves ;
}

Move MCTS::getRandomMove(const std::vector<Move> &moves){
    Move move ;
    if (moves.empty()){
        Move passMove ;
        passMove.x = -1;
        passMove.y = -1;
        passMove.stone = Stone::EMPTY ;
        passMove.isPass = true; 
        return passMove ;
    }
    std::uniform_int_distribution<int> dist(0,moves.size() - 1) ;
    int index = dist(rng) ;

    return moves[index] ;
}

bool MCTS::isTerminal(const Game &game){
    const std::vector<RecordMove> history = game.getHistory() ;   
    
    if (history.size() >= 2){
        const RecordMove& last = history[history.size() - 1];
        const RecordMove& prev = history[history.size() - 2];
        if (last.isPass && prev.isPass) {
            return true;
        }
    }
    
    return false ;
}

double MCTS::evaluate(const Game &game , Stone rootPlayer){
    const Board& board = game.getBoard() ;

    double blackScore = 0.0;
    double whiteScore = 6.5;

    for (int x = 0 ; x < Board::SIZE ; ++x){
        for (int y = 0 ; y < Board::SIZE ; ++y){
            Stone s = board.get(x,y) ;

            if (s == Stone::BLACK) {
                blackScore += 1.0;
            }
            else if (s == Stone::WHITE) {
                whiteScore += 1.0;
            }
            else {
                bool nearBlack = false;
                bool nearWhite = false;

                int dx[4] = {1, -1, 0, 0};
                int dy[4] = {0, 0, 1, -1};

                for (int k = 0; k < 4; ++k) {
                    int nx = x + dx[k];
                    int ny = y + dy[k];

                    if (nx < 0 || nx >= Board::SIZE || ny < 0 || ny >= Board::SIZE) {
                        continue;
                    }

                    Stone ns = board.get(nx, ny);
                    if (ns == Stone::BLACK) {
                        nearBlack = true;
                    } else if (ns == Stone::WHITE) {
                        nearWhite = true;
                    }
                }

                if (nearBlack && !nearWhite) {
                    blackScore += 0.35;
                } else if (nearWhite && !nearBlack) {
                    whiteScore += 0.35;
                }
            }
        }
    }
    double score = blackScore - whiteScore;
    return rootPlayer == Stone::BLACK ? score : -score;
}

MCTSNode* MCTS::selectNode(MCTSNode* node)
{
    while (node && !isTerminal(node->game)) {
        if (!node->isFullyExpanded()) {
            return node;
        }

        if (node->isLeaf()) {
            return node;
        }

        node = node->getBestPUCTChild(1.5);

        if (node == nullptr) {
            break;
        }
    }

    return node;
}

MCTSNode* MCTS::expandNode(MCTSNode* node){
    if (node->untriedMoves.empty()){
        return node ;
    }

    std::uniform_int_distribution<int> dist(0, node->untriedMoves.size() - 1);
    int idx = dist(rng);
    Move move = node->untriedMoves[idx];

    node->untriedMoves[idx] = node->untriedMoves.back() ;
    node->untriedMoves.pop_back() ;

    Game nextgame = node->game ;

    const bool ok = move.isPass ? nextgame.playPass() : nextgame.playMove(move.x, move.y);
    if (!ok) {
        return node;
    }

    const double prior = 1.0;

    MCTSNode* child = new MCTSNode(nextgame, move, node, getproperMoves(nextgame), prior);
    node->children.push_back(child);

    return child;
}

double MCTS::simulate(Game& gameState, Stone rootPlayer){
    return evaluate(gameState, rootPlayer) / 50.0;
}

void MCTS::backpropagate(MCTSNode* node , double result){
    while (node != nullptr){
        node->visits++ ;
        node->wins+=result ;
        node = node->parent ;
    }
}

Move MCTS::getbestMove(const Game &game){
    Move dummyMove ;
    dummyMove.x = -1 ;
    dummyMove.y = -1 ;
    dummyMove.isPass = true ;
    dummyMove.stone = Stone::EMPTY ;

    std::vector<Move> rootMoves = getproperMoves(game) ;

    if (evaluator) {
        const int currentPlayerInt = (game.getCurrentPlayer() == Stone::BLACK) ? 1 : 2;
        const EvaluationResult evalResult = evaluator->evaluate(game.getBoard(), currentPlayerInt);

        if (evalResult.valid && evalResult.policy.size() == 362) {
            rootMoves = sortMovesByPolicy(rootMoves, evalResult.policy);

            const int keepTopK = 30;
            if (static_cast<int>(rootMoves.size()) > keepTopK) {
                rootMoves.resize(keepTopK);
            }
        }
    }

    if (rootMoves.empty()) {
        return dummyMove;
    }

    MCTSNode* root = new MCTSNode(game, dummyMove, nullptr, rootMoves, 1.0);
    const Stone rootPlayer = game.getCurrentPlayer();

    for (int i = 0; i < iteration; ++i) {
        MCTSNode* node = selectNode(root);
        if (node == nullptr) {
            continue;
        }

        if (!isTerminal(node->game) && !node->isFullyExpanded()) {
            node = expandNode(node);
        }

        if (node == nullptr) {
            continue;
        }

        double result = simulate(node->game, rootPlayer);
        backpropagate(node, result);
    }

    MCTSNode* bestChild = nullptr;
    int maxVisits = -1;

    for (MCTSNode* child : root->children) {
        if (child && child->visits > maxVisits) {
            maxVisits = child->visits;
            bestChild = child;
        }
    }

    Move bestMove;
    if (bestChild) {
        bestMove = bestChild->move;
    } else {
        bestMove = rootMoves.front();
    }

    delete root;
    return bestMove;
}

int MCTS::moveToPolicyIndex(const Move& move) const
{
    if (move.isPass) {
        return 361;
    }
    return move.x * Board::SIZE + move.y;
}

std::vector<Move> MCTS::sortMovesByPolicy(const std::vector<Move>& moves, const std::vector<float>& policy) const
{
    std::vector<Move> sortedMoves = moves;

    std::sort(sortedMoves.begin(), sortedMoves.end(),
        [this, &policy](const Move& a, const Move& b) {
            int ia = moveToPolicyIndex(a);
            int ib = moveToPolicyIndex(b);

            float pa = (ia >= 0 && ia < (int)policy.size()) ? policy[ia] : 0.0f;
            float pb = (ib >= 0 && ib < (int)policy.size()) ? policy[ib] : 0.0f;

            return pa > pb;
        });

    return sortedMoves;
}
