#include "filter.h"
#include <cmath>

#define sqr(x) (x * x)

BandPassFilter::BandPassFilter(float fc, float fb, float fs)
{
    this->fc = fc;
    this->fb = fb;
    this->fs = fs;

    calculateCoefficients();
}

void BandPassFilter::setFc(float value)
{
    this->fc = value;
    calculateCoefficients();
}

void BandPassFilter::setFb(float value)
{
    this->fb = value;
    calculateCoefficients();
}

void BandPassFilter::setType(BandPassFilter::Type type)
{
    this->type = type;
    calculateCoefficients();
}

float BandPassFilter::process(float x)
{
    float xh = x - a1 * xhp - a2 * xhpp;
    float y = b0 * xh + b1 * xhp + b2 * xhpp;

    xhpp = xhp;
    xhp = xh;

    return y;
}

void BandPassFilter::calculateCoefficients()
{
    float k = tan(M_PI * fc / fs);
    float q = fc / fb;

    switch(type)
    {
        case Type::BandPass:
            b0 = k / (sqr(k) * q + k + q);
            b1 = 0;
            b2 = -b0;
            a1 = (2 * q * (sqr(k) - 1)) / (sqr(k) * q +k + q);
            a2 = (sqr(k) * q - k + q) / (sqr(k) * q + k + q);
        break;
        case Type::BandReject:
            b0 = (q * (1 + sqr(k))) / (sqr(k) * q + k + q);
            b1 = (2 * q * (sqr(k) - 1)) / (sqr(k) * q + k + q);
            b2 = b0;
            a1 = b1;
            a2 = (sqr(k) * q - k + q) / (sqr(k) * q + k + q);
        break;
    }
}