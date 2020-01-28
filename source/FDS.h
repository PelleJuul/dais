#pragma once

#include <vector>

#define pow2(x) (x * x)

float dxxLeft(float rh, const std::vector<float> &u);

float dxxMid(float rh, const std::vector<float> &u, int l);

float dxxRight(float rh, const std::vector<float> &u);