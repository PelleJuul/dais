#include "BowedString.h"
#include "FDS.h"
#include <cmath>

BowedString::BowedString(int L, float fs) :
    ua(L, 0),
    ub(L, 0),
    uc(L, 0),
    dxxp(L, 0),
    u(ua),
    up(ub),
    un(uc)
{
    this->fs = fs;
    this->L = L;

    h = 1.0 / (float)L;
    k = 1.0 / fs;
    vrel = vb;

    c1 = 1 / (1 + k*sigma0);
    c2 = 1 / (2*k);
};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

float BowedString::theta(float a, float eta)
{
    // return sqrt(2 * a) * eta * exp(-a * pow2(eta) + 0.5);
    return sgn(eta) * (epsilon + (1 - epsilon) * exp(-a * fabs(eta)));
}

inline float thetad(float a, float eta)
{
    return sqrt(2 * a) * (exp(-a * pow2(eta) + 0.5) - 2 * a * pow2(eta) * exp(-a * pow2(eta) + 0.5));
}

inline float BowedString::update(
    const std::vector<float> &u,
    const std::vector<float> &up,
    int l,
    float vrel,
    float Fb,
    float k,
    float rh,
    float wavespeed,
    float a,
    float sigma0,
    float dxx,
    float thetaVal)
{
    return (
          pow2(k) * pow2(wavespeed) * dxx
        - pow2(k) * Fb * thetaVal
        + sigma0 * k * up[l]
        + 2 * sigma1 * k * (dxx - dxxp[l])
        + 2 * u[l]
        - up[l]);
        
    dxxp[l] = dxx;
}

float BowedString::getNextSample(float input)
{
    // update left edge
    float dxx = dxxLeft(L, u);
    un[0] = c1 * update(u, up, 0, vrel, 0, k, L, wavespeed, a, sigma0, dxx, 0);

    int lb = round(L * 0.11472387);
    
    // update up to bowing bowing point
    for (int l = 1; l < lb - 1; l++)
    {
		dxx = dxxMid(L, u, l);
		un[l] = c1 * update(u, up, l, vrel, 0, k, L, wavespeed, a, sigma0, dxx, 0);
    }
    
    dxx = dxxMid(L, u, lb-1);
    un[lb-1] = c1 * update(u, up, lb - 1, vrel, 1, k, L, wavespeed, a, sigma0, dxx, input);
    
	vrel = fs * (u[lb] - up[lb]) - vb;
	dxx = dxxMid(L, u, lb);
	float thetaVal = theta(a, vrel);
	un[lb] = c1 * update(u, up, lb, vrel, Fb, k, L, wavespeed, a, sigma0, dxx, thetaVal);

    for (int l = lb + 1; l < L-1; l++)
    {
		dxx = dxxMid(L, u, l);
		un[l] = c1 * update(u, up, l, vrel, 0, k, L, wavespeed, a, sigma0, dxx, 0);
    }

    // Update right edge
    dxx = dxxRight(L-1, u);
    un[L-1] = c1 * update(u, up, u.size()-1, vrel, 0, k, L-1, wavespeed, a, sigma0, dxx, 0);

    std::vector<float> &uswap = up;

    up = u;
    u = un;
    un = uswap;

    return u[round(L * 0.8)];
}