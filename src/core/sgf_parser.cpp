#include "sgf_parser.h"
#include <fstream>
#include <iostream>

// 对于本段代码，首先需要明确的是围棋棋谱均以.sgf格式出现
// 所以我们需要使用相应的moves来记录所有的围棋的步数 
std::vector<Move> SGFParser::parse(const std::string& filename){
    std::vector<Move> moves ;

    // 这一步是读取文件
    std::ifstream fin(filename) ;
    if (!fin){
        std::cout << "无法识别文件" << std::endl ;
        return moves ;
    }

    // 这一步则是将文件编程字符串的形式，更容易读取
    std::string sgf((std::istreambuf_iterator<char>(fin)),
                     std::istreambuf_iterator<char>());

    // 接下来需要对输入的sgf文件进行扫描
    int i = 0 ;
    while (i < sgf.size()){

        // 找到对应的棋子的颜色以及左括号
        if ((sgf[i] == 'B' || sgf[i] == 'W') && i + 1 < sgf.size() && sgf[i+1] == '['){
            Stone stone ;

            if(sgf[i] == 'B'){
                stone = Stone::BLACK ;
            }
            else{
                stone = Stone::WHITE ;
            }

            // 找右括号
            int j = i + 2 ;
            while (j < sgf.size() && sgf[j] != ']'){
                j ++ ;
            }

            // 防止格式错误
            if (j >= sgf.size()){
                break ;
            }

            std::string coord = sgf.substr(i + 2, j - (i + 2)) ;

            // 解析坐标
            if (coord.size() == 2){
                char col = coord[0] ;
                char row = coord[1] ;

                // 跳过 tt 
                if (!(col == 't' && row == 't')){
                    Move m ;
                    m.x = col - 'a' ;
                    m.y = row - 'a' ;
                    m.stone = stone ;

                    moves.push_back(m) ;
                }
            }

            i = j ; 
        }
        i ++ ;
    }
    
    return moves ;
}