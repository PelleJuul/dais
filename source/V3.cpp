#include "V3.h"

V3::V3()
{
    x = 0.0;
    y = 0.0;
    z = 0.0;
}

V3::V3(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

V3 crossProduct(V3 a, V3 b)
{
    V3 result;
    
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

V3 getBarycenter(V3 a, V3 b, V3 c)
{
    V3 barycenter;

    barycenter.x = (a.x + b.x + c.x) / 3.0;
    barycenter.y = (a.y + b.y + c.y) / 3.0;
    barycenter.z = (a.z + b.z + c.z) / 3.0;

    return barycenter;
}

V3 operator-(const V3& a, const V3& b)
{
	V3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}