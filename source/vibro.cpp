#include "vibro.h"

Vibro::Vibro(float fc, float fs) :
    bandPass(fc, 0.125 * fc, fs),
    bandReject(fc, 1, fs),
    integrator(10 * (1.0 / fc), fs),
    osc(1.0, fc, fs)
{
    bandReject.setType(BandPassFilter::Type::BandReject);
    bandPass.setType(BandPassFilter::Type::BandPass);
}

float Vibro::processForward(float x)
{
    float xbp = bandPass.process(x);
    float env = integrator.process(xbp);

    float o = osc.next();
    float y = env * osc.next();

    return y;
}

float Vibro::processBackward(float x)
{
    return bandReject.process(x);
}