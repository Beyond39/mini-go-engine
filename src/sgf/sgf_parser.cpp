#include "sgf_parser.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <iterator>

void SGFParser::skipWhitespace(const std::string& sgf , int& pos){
    while (pos < (int)sgf.size() && std::isspace((unsigned char)sgf[pos])){
        ++pos ;
    }
}

std::string SGFParser::parsePropIdent(const std::string& sgf , int& pos){
    std::string ident ;
    while (pos < (int)sgf.size() && std::isupper((unsigned char)sgf[pos])){
        ident += sgf[pos++] ;
    }
    return ident ;
}

std::string SGFParser::parsePropValue(const std::string& sgf ,int& pos){
    std::string value ;

    if (pos >= (int)sgf.size() || sgf[pos] != '['){
        return value ;
    }

    ++pos ;

    while(pos < (int)sgf.size()){
        if (sgf[pos] == '\\'){
            ++pos ;
            if (pos < (int)sgf.size()){
                value += sgf[pos++] ;
            }
        }
        else if(sgf[pos] == ']'){
            ++pos ;
            break ;
        }
        else{
            value += sgf[pos++] ;
        }
    }

    return value ;
}

void SGFParser::parseProperties(const std::string& sgf , int& pos , SGFNode* node){
    while (pos < (int)sgf.size()){
        skipWhitespace(sgf,pos) ;
        
        if (pos >= (int)sgf.size()){
            return ;
        }

        if (sgf[pos] == ';' || sgf[pos] == '(' || sgf[pos] == ')'){
            return ;
        }

        std::string ident = parsePropIdent(sgf,pos) ;
        if (ident.empty()){
            ++pos ;
            continue; 
        }

        skipWhitespace(sgf,pos) ;

        while (pos < (int)sgf.size() && sgf[pos] == '['){
            std::string value = parsePropValue(sgf,pos) ;
            node->properties[ident].push_back(value) ;
            skipWhitespace(sgf,pos) ;
        }
    }
}

SGFNode* SGFParser::parseSequence(const std::string& sgf ,int& pos , SGFNode* parent){
    SGFNode* first = nullptr ;
    SGFNode* prev = parent ;

    while (pos < (int)sgf.size()){
        skipWhitespace(sgf,pos) ;

        if (pos >= (int)sgf.size() || sgf[pos] != ';' ){
            break;
        }

        ++pos ;

        SGFNode* node = new SGFNode() ;
        node->parent = prev ;
        parseProperties(sgf,pos,node) ;

        if (prev){
            prev->children.push_back(node) ;
        }

        if (!first){
            first = node ;
        }

        prev = node ; 
    }

    return first ;
}

SGFNode* SGFParser::parseTree(const std::string& sgf , int& pos ,SGFNode* parent){
    skipWhitespace(sgf,pos) ;

    if (pos >= (int)sgf.size() || sgf[pos] != '(') {
        return nullptr;
    }

    ++pos ;

    SGFNode* sequenceStart = parseSequence(sgf,pos,parent) ;
    SGFNode* current = sequenceStart ;

    while (current && !current->children.empty()){
        current = current->children.back();
    }

    while (pos < (int)sgf.size()){
        skipWhitespace(sgf,pos) ;

        if (pos < (int)sgf.size()){
            skipWhitespace(sgf,pos);

            if (pos < (int)sgf.size() && sgf[pos] == '('){
                parseTree(sgf,pos,current) ;
            }
            else{
                break; 
            }
        }
    }

    skipWhitespace(sgf,pos) ;

    if (pos < (int)sgf.size() && sgf[pos] == ')'){
        ++pos ;
    }

    return sequenceStart ;
}

SGFTree* SGFParser::parse(const std::string& filename){
    std::ifstream fin(filename) ;
    if (!fin){
        std::cout << "无法打开SGF文件" << filename << std::endl ;
        return nullptr ;
    }

    std::string sgf((std::istreambuf_iterator<char>(fin)),std::istreambuf_iterator<char>()) ;

    int pos = 0 ;
    SGFTree* tree = new SGFTree();
    tree->root = parseTree(sgf,pos,nullptr);

    return tree ;
}

