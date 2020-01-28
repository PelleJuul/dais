#pragma once

#include <vector>

struct BowedString
{
    // Basic parameters
    float fs;

    // String parameters
    int L;
    float wavespeed = 200;
    float sigma0 = 1.0;
    float sigma1 = 0.005;

    // Bow parameters
    float Fb = 1000;
    float vb = 0.2;
    float a = 1;
    float Fin = 0;
    float epsilon = 0.5;

    // Derived parameters
    float h;
    float k;
    float c1;
    float c2;

    // State variables
    std::vector<float> ua;
    std::vector<float> ub;
    std::vector<float> uc;
    std::vector<float> dxxp;
    std::vector<float> &u;
    std::vector<float> &up;
    std::vector<float> &un;
    float vrel;

    BowedString(int L, float fs);
    
    float update(
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
	    float thetaVal);
	    
 	float theta(float a, float eta);

    float getNextSample(float input);
};