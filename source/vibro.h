#pragma once

#include <sinosc.h>
#include "filter.h"
#include "integrator.h"

class Vibro
{
    public:
    Vibro(float fc, float fs);

    float processForward(float x);

    float processBackward(float x);

    private:
    BandPassFilter bandPass;
    BandPassFilter bandReject;
    Integrator integrator;
    SinOsc osc;
};