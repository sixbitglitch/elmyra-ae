#ifndef LFO_HPP
#define LFO_HPP

// Per-voice LFO for slow pitch drift.
// Two modes: triangle (smooth glide) and S&H (stepped jumps).
// Designed to be ticked at IO_UPDATE_FREQ (25 Hz), advancing by
// SAMPLE_RATE / IO_UPDATE_FREQ samples per call.
class LFO
{
private:
    long index;
    long period;
    int  depth;
    int  mode;
    int  shValue;

public:
    static const int MODE_TRIANGLE = 0;
    static const int MODE_SH       = 1;

    LFO() : index(0), period(24000L), depth(0), mode(MODE_TRIANGLE), shValue(0) {}

    // rateMilliHz: e.g. 80 = 0.08 Hz, 500 = 0.5 Hz
    void setRate(int rateMilliHz, int sampleRate)
    {
        if (rateMilliHz < 1) rateMilliHz = 1;
        period = ((long)sampleRate * 1000L) / rateMilliHz;
        if (period < 2L) period = 2L;
    }

    void setDepth(int d) { depth = d; }
    void setMode(int m)  { mode  = m; }
    int  getMode()       { return mode; }

    // Advance by n samples, return current output in range [-depth, +depth].
    int tick(int n)
    {
        index += n;
        if (index >= period)
        {
            index -= period;
            if (mode == MODE_SH && depth > 0)
            {
                shValue = (int)(random(-depth, depth + 1));
            }
        }

        if (mode == MODE_SH)
        {
            return shValue;
        }

        // Triangle wave
        long half = period / 2L;
        if (index < half)
        {
            return -depth + (int)(((long)index * 2L * depth) / half);
        }
        else
        {
            return depth - (int)((((long)index - half) * 2L * depth) / half);
        }
    }
};

#endif
