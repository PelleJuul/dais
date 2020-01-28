#include "integrator.h"
#include <cmath>

Integrator::Integrator(float t, float fs)
{
    this->fs = fs;
    setTimeConstant(t);
}

void Integrator::setTimeConstant(float t)
{
    this->a = exp(-1.0 / (t * fs));
}

float Integrator::process(float x)
{
    float y = a * yp + (1.0 - a) * fabs(x);
    yp = y;

    return y;
}