#pragma once
#include <string>
#include <vector>
#include "Game.h"
#include "../sgf/sgf_utils.h"

class DataExtractor {
public:
    static void extractDirectory(const std::string& sgfDir, const std::string& outBinPath,int maxFiles);
    static bool extractSingleSGF(const std::string& sgfPath, std::ofstream& fout);
    static std::vector<float> getBoardFeatures(const Game& game);
    static void extractFromFileList(const std::string& listPath,const std::string& outBinPath);
};