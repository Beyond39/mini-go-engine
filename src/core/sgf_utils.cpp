#include "sgf_utils.h"

bool extractMoveFromNode(SGFNode* node, Move& move){
    move.x = -1;
    move.y = -1;
    move.stone = Stone::EMPTY;
    move.isPass = false;

    if (!node) {
        return false;
    }

    if (node->properties.count("B")) {
        if (node->properties["B"].empty()) {
            return false;
        }

        move.stone = Stone::BLACK;
        const std::string& coord = node->properties["B"][0];

        if (coord.empty()) {
            move.isPass = true;
            return true;
        }

        if (coord.size() == 2) {
            move.x = coord[0] - 'a';
            move.y = coord[1] - 'a';
            return true;
        }

        return false;
    }

    if (node->properties.count("W")) {
        if (node->properties["W"].empty()) {
            return false;
        }

        move.stone = Stone::WHITE;
        const std::string& coord = node->properties["W"][0];

        if (coord.empty()) {
            move.isPass = true;
            return true;
        }

        if (coord.size() == 2) {
            move.x = coord[0] - 'a';
            move.y = coord[1] - 'a';
            return true;
        }

        return false;
    }

    return false;
}

std::vector<Move> extractMainLine(SGFNode* node){
    std::vector<Move> moves ;
    SGFNode* current = node ;

    while (current){
        Move move ;
        if (extractMoveFromNode(current,move)){
            moves.push_back(move) ;
        }

        if (current->children.empty()){
            break;
        }

        current = current->children[0] ;
    }

    return moves ;
}

std::vector<Move> convertHistoryToMoves(const std::vector<RecordMove> history){
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