#include "FDS.h"
#include <vector>

float dxxLeft(float rh, const std::vector<float> &u)
{
    return pow2(rh) * (-2 * u[0] + u[1]);
}

float dxxMid(float rh, const std::vector<float> &u, int l)
{
    return pow2(rh) * (u[l-1] - 2 * u[l] + u[l+1]);
}

float dxxRight(float rh, const std::vector<float> &u)
{
    return pow2(rh) * (u[u.size()-2] - 2 * u[u.size()-1]);
}