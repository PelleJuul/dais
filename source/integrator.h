#pragma once

class Integrator
{
    public:
    Integrator(float t, float fs);

    float process(float x);

    void setTimeConstant(float t);

    private:
    float fs;
    float a = 0;
    float yp = 0;
};