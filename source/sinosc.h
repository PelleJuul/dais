#pragma once

#include <math.h>

class SinOsc
{
    public:
    float amp = 1;
    float freq = 440;
    float sampleRate = 44100;

    SinOsc(float amp, float freq, float sampleRate);
    float next();

    private:
    float phase = 0;
};