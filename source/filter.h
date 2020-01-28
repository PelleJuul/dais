#pragma once

class BandPassFilter
{
    public:
    enum Type
    {
        BandPass,
        BandReject
    };

    BandPassFilter(float fc, float fb, float fs);

    void setFc(float value);

    void setFb(float value);

    void setType(BandPassFilter::Type type);

    float process(float x);

    private:
    void calculateCoefficients();

    BandPassFilter::Type type = BandPassFilter::Type::BandPass;
    float fc;
    float fb;
    float fs;
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
    float xhp = 0;
    float xhpp = 0;
};