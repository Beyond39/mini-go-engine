#include "MCTS.h"
#include "MCTSNode.h"
#include "../core/Board.h" 

#include <algorithm>

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
    bool earlyOpening = (history.size() < 2);

    for (auto it : points){
        int x = it.first;
        int y = it.second;
        
        if (!earlyOpening){
            if (!hasNearbyStone(board, x, y, 2)) {
                continue;
            }
        }
        Move move ;
        move.x = it.first ;
        move.y = it.second ;
        move.stone = player ;
        move.isPass = false ;

        moves.push_back(move) ;
    }

    if (moves.empty()){
        for (auto it : points) {
            Move move;
            move.x = it.first;
            move.y = it.second;
            move.stone = player;
            move.isPass = false;
            moves.push_back(move);
        }
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

double MCTS::evaluate(const Game &game , Stone currentPlayer){
    const Board& board = game.getBoard() ;

    int blackStones = 0 ;
    int whiteStones = 0 ;
    int blackTerritory = 0 ;
    int whiteTerritory = 0 ; 

    for (int x = 0 ; x < Board::SIZE ; ++x){
        for (int y = 0 ; y < Board::SIZE ; ++y){
            Stone s = board.get(x,y) ;

            if (s == Stone::BLACK) {
                    blackStones++;
                }
            else if (s == Stone::WHITE) {
                whiteStones++;
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
                    }
                    else if (ns == Stone::WHITE) {
                        nearWhite = true;
                    }
                }

                if (nearBlack && !nearWhite) {
                    blackTerritory++;
                }
                else if (nearWhite && !nearBlack) {
                    whiteTerritory++;
                }
            }
        }
    }

    double blackScore = blackStones * 1.0 + blackTerritory * 2.5;
    double whiteScore = whiteStones * 1.0 + whiteTerritory * 2.5;

    double score = blackScore - whiteScore;

    if (currentPlayer == Stone::BLACK) {
        return score;
    } 
    else {
        return -score;
    }
}

MCTSNode* MCTS::selectNode(MCTSNode* node)
{
    while (!isTerminal(node->game)) {
        if (!node->isFullyExpanded()) {
            return node;
        }

        if (node->isLeaf()) {
            return node;
        }

        node = node->getBestUCBChild(1.414);

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

    if (move.isPass){
        nextgame.playPass() ;
    }
    else{
        nextgame.playMove(move.x,move.y) ;
    }

    MCTSNode* child = new MCTSNode(nextgame, move, node, getproperMoves(nextgame)) ;
    node->children.push_back(child) ;

    return child ;
}

double MCTS::simulate(Game& gameState, Stone currentPlayer)
{
    if (evaluator != nullptr) {
        EvaluationResult result = evaluator->evaluate(
            gameState.getBoard(),
            static_cast<int>(currentPlayer)
        );

        if (result.valid) {
            return static_cast<double>(result.value);
        }
    }

    return evaluate(gameState, currentPlayer);
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
    dummyMove.isPass = false ;
    dummyMove.stone = Stone::EMPTY ;

    std::vector<Move> rootMoves = getproperMoves(game) ;

    if (rootMoves.empty()){
        return dummyMove ;
    }

    MCTSNode* root = new MCTSNode(game, dummyMove, nullptr , rootMoves) ;
    Stone currentPlayer = game.getCurrentPlayer() ;

    for (int i = 0 ; i < iteration ; ++i){
        MCTSNode* node = selectNode(root) ;

        if (node == nullptr){
            continue; 
        }

        if (!isTerminal(node->game) && !node->isFullyExpanded()){
            node = expandNode(node) ;
        }

        double result = simulate(node->game ,currentPlayer) ;
        backpropagate(node,result) ;
    }

    MCTSNode* bestChild = nullptr ;
    int maxVisits = -1 ;

    for (MCTSNode* child : root->children){
        if (child->visits > maxVisits){
            maxVisits = child->visits ;
            bestChild = child ;
        }
    }

    Move bestMove =  bestChild ? bestChild->move : dummyMove ;

    delete root ;
    return bestMove ;
}

MCTS::MCTS(PythonEvaluator* evaluator)
    : evaluator(evaluator)
{
}