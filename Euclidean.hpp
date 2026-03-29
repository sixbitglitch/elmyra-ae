#ifndef EUCLIDEAN_HPP
#define EUCLIDEAN_HPP

// Eight preset Euclidean patterns {steps, beats}.
// Beat placement uses the Bresenham formula: step i fires when (i*beats)%steps < beats.
// This distributes beats as evenly as possible around the cycle.
#define EUCLID_NUM_PRESETS 8

static const int euclidPresets[EUCLID_NUM_PRESETS][2] = {
    {16, 4},   // even quarters       – steady pulse
    { 8, 3},   // tresillo             – classic afro-cuban
    {16, 5},   // 5-over-16            – sparse push-pull
    { 8, 5},   // 5-over-8             – syncopated dense
    {16, 7},   // 7-over-16            – hemiola feel
    {12, 7},   // 7-over-12            – latin 7
    {16, 9},   // 9-over-16            – very dense
    {16, 3},   // 3-over-16            – wide gaps, sparse
};

class EuclideanClock
{
private:
    long clockPeriod;  // samples per step
    long clockIndex;   // sample position within current step
    int  step;         // current step index (0 to steps-1)
    int  steps;        // total steps in pattern
    int  beats;        // active beats in pattern
    int  preset;       // index into euclidPresets[]

public:
    EuclideanClock()
        : clockPeriod(6000L), clockIndex(0L),
          step(0), steps(16), beats(4), preset(0)
    {}

    void setStepPeriod(long samplesPerStep)
    {
        if (samplesPerStep < 200L) samplesPerStep = 200L;
        clockPeriod = samplesPerStep;
    }

    void setPreset(int p)
    {
        preset = ((p % EUCLID_NUM_PRESETS) + EUCLID_NUM_PRESETS) % EUCLID_NUM_PRESETS;
        steps  = euclidPresets[preset][0];
        beats  = euclidPresets[preset][1];
        step   = 0;
        clockIndex = 0;
    }

    void nextPreset() { setPreset(preset + 1); }

    int getPreset() { return preset; }
    int getStep()   { return step;   }
    int getSteps()  { return steps;  }
    int getBeats()  { return beats;  }

    bool isBeat(int s)
    {
        if (beats <= 0)     return false;
        if (beats >= steps) return true;
        return ((s * beats) % steps) < beats;
    }

    // Advance by n samples. Returns true if at least one beat fires.
    bool tick(long n)
    {
        bool hit = false;
        clockIndex += n;
        while (clockIndex >= clockPeriod)
        {
            clockIndex -= clockPeriod;
            if (isBeat(step)) hit = true;
            if (++step >= steps) step = 0;
        }
        return hit;
    }
};

#endif
