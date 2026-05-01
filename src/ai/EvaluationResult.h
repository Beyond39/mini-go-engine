#pragma once

#include <vector>

struct EvaluationResult
{
    float value = 0.0f;
    bool valid = false;
    std::vector<float> policy;
};
