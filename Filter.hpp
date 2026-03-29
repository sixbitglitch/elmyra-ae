#ifndef FILTER_HPP
#define FILTER_HPP

// Two-pole state variable filter (resonant low-pass).
//
// Coefficients are fixed-point, scaled by 256:
//   f_fixed    = 2*sin(π*fc/fs) * 256    (cutoff)
//   damp_fixed = (1/Q)          * 256    (resonance damping; lower = more resonance)
//
// Mod-knob one-knob sweep (set from ioUpdate):
//   Mod = 0    → high cutoff, no resonance  (transparent, full bandwidth)
//   Mod = full → low cutoff, high resonance (dark, screamy)
//   LFO depth follows the same knob, so all three effects scale together.
//
// Filter sits before the delay: echoes carry the filtered timbre, and
// at high resonance + high delay feedback the system can self-oscillate.
class Filter
{
private:
    int low;
    int band;
    int f_fixed;
    int damp_fixed;

public:
    Filter() : low(0), band(0), f_fixed(FILTER_F_OPEN), damp_fixed(FILTER_DAMP_OPEN) {}

    void setParams(int f, int damp)
    {
        f_fixed    = f;
        damp_fixed = damp;
    }

    int process(int input)
    {
        // State variable filter tick
        int high = input - low - ((damp_fixed * band) >> 8);
        band     = ((f_fixed * high) >> 8) + band;
        low      = ((f_fixed * band) >> 8) + low;

        // Hard clamp – prevents runaway at extreme resonance / delay feedback
        if (low  >  AMP_MAX) low  =  AMP_MAX;
        if (low  < -AMP_MAX) low  = -AMP_MAX;
        if (band >  AMP_MAX) band =  AMP_MAX;
        if (band < -AMP_MAX) band = -AMP_MAX;

        return low;
    }
};

#endif
