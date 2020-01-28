#include "ExpCal.h"
#include <cmath>

ExpCal::ExpCal(float oy, float a, float b, float ox)
{
    init(oy, a, b, ox);
}

ExpCal::ExpCal()
{

}

void ExpCal::init(float oy, float a, float b, float ox)
{
    this->oy = oy;
    this->a = a;
    this->b = b;
    this->ox = ox;
}

float ExpCal::compute(float x)
{
    return oy + a * expf(b * (x - ox));
}