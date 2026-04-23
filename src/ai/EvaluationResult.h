#pragma once

struct EvaluationResult
{
    float value = 0.0f;
    bool valid = false;
    std::vector<float> policy;
};
