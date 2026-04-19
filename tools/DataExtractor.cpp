#include "DataExtractor.h"
#include "../sgf/sgf_parser.h"
#include <fstream>
#include <iostream>

#include <QDir>
#include <QFileInfoList>

std::vector<float> DataExtractor::getBoardFeatures(const Game& game){
    const int planeSize = Board::SIZE *Board::SIZE ;
    std::vector<float> features(3 * planeSize , 0.0f) ;

    const Board& board = game.getBoard() ;
    Stone currentPlayer = game.getCurrentPlayer() ;
    Stone oppPlayer = board.opponent(currentPlayer) ;

    for (int x = 0 ; x < Board::SIZE ; ++x){
        for (int y = 0 ; y < Board::SIZE ; ++y){
            Stone s = board.get(x,y) ;
            int idx = x * Board::SIZE + y ;

            if (s == currentPlayer){
                features[idx] = 1.0f ;
            }

            else if(s == oppPlayer){
                features[planeSize + idx] = 1.0f ;
            }
        }
    }

    if (currentPlayer == Stone::BLACK){
        for (int i = 0 ; i < planeSize ; ++i){
            features[2 * planeSize + 1] == 1.0f ;
        }
    }

    return features ;
}

void DataExtractor::extractDirectory(const std::string& sgfDir, const std::string& outBinPath, int maxFiles){
    QDir dir(QString::fromStdString(sgfDir));
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.sgf", QDir::Files);

    std::ofstream fout(outBinPath, std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) {
        std::cerr << "无法打开输出文件: " << outBinPath << std::endl;
        return;
    }

    int total = files.size();
    int limit = (maxFiles < 0 || maxFiles > total) ? total : maxFiles;

    int successCount = 0;
    int failCount = 0;

    for (int i = 0; i < limit; ++i) {
        const QFileInfo& file = files[i];
        std::string path = file.absoluteFilePath().toStdString();

        if (extractSingleSGF(path, fout)) {
            ++successCount;
            std::cout << "已提取: " << successCount << " / " << limit << std::endl;
        } else {
            ++failCount;
            std::cerr << "跳过文件: " << path << std::endl;
        }
    }

    fout.close();

    std::cout << "目录提取完成。" << std::endl;
    std::cout << "成功: " << successCount << std::endl;
    std::cout << "失败: " << failCount << std::endl;
}


bool DataExtractor::extractSingleSGF(const std::string& sgfPath, std::ofstream& fout){
    SGFTree* tree = SGFParser::parse(sgfPath);
    if (!tree || !tree->root) {
        std::cerr << "SGF 解析失败: " << sgfPath << std::endl;
        if (tree) delete tree;
        return false;
    }

    std::vector<Move> moves = extractMainLine(tree->root);

    float blackWinFlag = 0.0f ; 
    if (tree->root->properties.count("RE") && !tree->root->properties["RE"].empty()) {
        std::string res = tree->root->properties["RE"][0];
        if (!res.empty()) {
            if (res[0] == 'B' || res[0] == 'b') {
                blackWinFlag = 1.0f;
            } else if (res[0] == 'W' || res[0] == 'w') {
                blackWinFlag = -1.0f;
            }
        }
    }

    Game game;

    for (const Move& move : moves){
        std::vector<float> X = getBoardFeatures(game) ;

        int Y = move.isPass ? 361 : (move.y * Board::SIZE + move.x) ;

        float Z = 0.0f ;
        if (blackWinFlag != 0.0f) {
            if ((game.getCurrentPlayer() == Stone::BLACK && blackWinFlag == 1.0f) ||
                (game.getCurrentPlayer() == Stone::WHITE && blackWinFlag == -1.0f)) {
                Z = 1.0f;
            } else {
                Z = -1.0f;
            }
        }

        fout.write(reinterpret_cast<const char*>(X.data()), static_cast<std::streamsize>(X.size() * sizeof(float)));
        fout.write(reinterpret_cast<const char*>(&Y), sizeof(int));
        fout.write(reinterpret_cast<const char*>(&Z), sizeof(float));

        bool ok = true ;
        if (move.isPass){
            game.playPass() ;
        }
        else{
            ok = game.playMove(move.x,move.y) ;
        }

        if (!ok) {
            std::cerr << "非法落子或规则不匹配，文件: " << sgfPath
                      << " 坐标=(" << move.x << ", " << move.y << ")" << std::endl;
            delete tree;
            return false;
        }
    }

    delete tree ;
    return true ;
}

void DataExtractor::extractFromFileList(const std::string& listPath,const std::string& outBinPath){
    std::ifstream fin(listPath);
    if (!fin.is_open()) {
        std::cerr << "无法打开列表文件: " << listPath << std::endl;
        return;
    }

    std::ofstream fout(outBinPath, std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) {
        std::cerr << "无法打开输出文件: " << outBinPath << std::endl;
        fin.close();
        return;
    }

    std::string sgfPath;
    int successCount = 0;
    int failCount = 0;
    int totalCount = 0;

    while (std::getline(fin, sgfPath)) {
        if (sgfPath.empty()) {
            continue;
        }

        ++totalCount;

        if (extractSingleSGF(sgfPath, fout)) {
            ++successCount;
            std::cout << "已提取: " << successCount
                      << " / 已处理列表项: " << totalCount << std::endl;
        } else {
            ++failCount;
            std::cerr << "跳过文件: " << sgfPath << std::endl;
        }
    }

    fin.close();
    fout.close();

    std::cout << "列表提取完成。" << std::endl;
    std::cout << "总处理: " << totalCount << std::endl;
    std::cout << "成功: " << successCount << std::endl;
    std::cout << "失败: " << failCount << std::endl;
}