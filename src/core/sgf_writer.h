#pragma once

#include <string>
#include <vector>
#include "sgf_utils.h"

class SGFWriter
{
public:
    static bool saveMainLine(const std::string& filename, const std::vector<Move>& moves);
};