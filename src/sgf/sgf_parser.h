#pragma once

#include <vector>
#include <string>
#include "Board.h"
#include <map>

struct SGFNode{
    std::map<std::string,std::vector<std::string>> properties ;
    SGFNode* parent = nullptr ;
    std::vector<SGFNode*> children ;

    ~SGFNode(){
        for (SGFNode* child : children){
            delete child ;
        }
    }
};

struct SGFTree{
    SGFNode* root = nullptr ;

    ~SGFTree(){
        delete root ;
    }
};

class SGFParser{
public:
    static SGFTree* parse(const std::string &filename) ;

private:
    static void skipWhitespace(const std::string& sgf, int& pos);
    static SGFNode* parseTree(const std::string& sgf, int& pos, SGFNode* parent);
    static SGFNode* parseSequence(const std::string& sgf, int& pos, SGFNode* parent);
    static void parseProperties(const std::string& sgf, int& pos, SGFNode* node);
    static std::string parsePropIdent(const std::string& sgf, int& pos);
    static std::string parsePropValue(const std::string& sgf, int& pos); 
};