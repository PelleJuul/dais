#include "sinosc.h"
#include <math.h>

SinOsc::SinOsc(float amp, float freq, float sampleRate)
{
    this->amp = amp;
    this->freq = freq;
    this->sampleRate = sampleRate;
}

float SinOsc::next()
{
    float y = amp * sin(phase);
    
    phase += 2 * M_PI * freq / sampleRate;

    if (phase >= 2 * M_PI)
    {
        phase -= 2 * M_PI;
    }

    return y;
}