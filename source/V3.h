#pragma once

struct V3
{
    V3();
    V3(float x, float y, float z);

    float x;
    float y;
    float z;
};

V3 crossProduct(V3 a, V3 b);
V3 getBarycenter(V3 a, V3 b, V3 c);
V3 operator-(const V3& a, const V3& b);