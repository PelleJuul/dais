#pragma once

/// Class for computing the resulting values after calibrating to an
/// exponential curve.
class ExpCal
{
    public:

    /// Creates a new ExpCal which computes
    ///
    ///     oy + a * exp(b * (x - ox))
    ExpCal(float oy, float a, float b, float ox);

    ExpCal();

    void init(float oy, float a, float b, float ox);

    float compute(float x);

    private:
    float oy;
    float a;
    float b;
    float ox;
};