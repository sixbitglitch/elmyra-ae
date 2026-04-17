#ifndef REVERB_HPP
#define REVERB_HPP

// Schroeder algorithmic reverb.
// Architecture: 4 parallel comb filters → summed → 2 series allpass filters.
// Buffers use `short` (2 bytes each) matching the existing Delay convention.
// Total SRAM: (607+743+877+1013+347+113) × 2 = ~7.4 KB
//
// Three parameters (all set from ioUpdate, not per-sample):
//   feedback — reverb decay length    (0–950,  maps from F.BACK pot)
//   damping  — HF rolloff in tail     (0–900,  maps from TIME pot;  low=bright, high=warm/dark)
//   mix      — wet/dry blend          (0–1024, maps from MIX pot)
//
// process(in) is called every audio sample from fillBuffer().

#define REVERB_COMB1_SIZE   607
#define REVERB_COMB2_SIZE   743
#define REVERB_COMB3_SIZE   877
#define REVERB_COMB4_SIZE  1013
#define REVERB_AP1_SIZE     347
#define REVERB_AP2_SIZE     113

class Reverb
{
private:
    // Comb filter buffers + indices
    short comb1[REVERB_COMB1_SIZE];
    short comb2[REVERB_COMB2_SIZE];
    short comb3[REVERB_COMB3_SIZE];
    short comb4[REVERB_COMB4_SIZE];
    int   c1i, c2i, c3i, c4i;

    // Allpass buffers + indices
    short ap1[REVERB_AP1_SIZE];
    short ap2[REVERB_AP2_SIZE];
    int   a1i, a2i;

    // 1-pole LP state for HF damping inside each comb
    int   damp1, damp2, damp3, damp4;

    // Parameters (fixed-point, scale 1024)
    int feedback;   // comb feedback coefficient × 1024
    int damping;    // HF damping coefficient × 1024  (0=none, 900=heavy)
    int mix;        // wet amount × 1024  (dry = 1024 - mix)

    inline int clamp(int v)
    {
        if (v >  AMP_MAX) return  AMP_MAX;
        if (v < -AMP_MAX) return -AMP_MAX;
        return v;
    }

    inline int tickComb(short *buf, int size, int *idx, int *dampState, int in)
    {
        int out = buf[*idx];
        // 1-pole LP damping: damp_state = damp*damp_state + (1024-damp)*out) >> 10
        *dampState = ((damping * *dampState) + ((1024 - damping) * out)) >> 10;
        buf[*idx]  = (short)clamp(in + ((feedback * *dampState) >> 10));
        if (++(*idx) >= size) *idx = 0;
        return out;
    }

    inline int tickAllpass(short *buf, int size, int *idx, int in)
    {
        int b        = buf[*idx];
        buf[*idx]    = (short)clamp(in + ((512 * b) >> 10));
        if (++(*idx) >= size) *idx = 0;
        return b - ((512 * in) >> 10);
    }

public:
    Reverb()
        : c1i(0), c2i(0), c3i(0), c4i(0),
          a1i(0), a2i(0),
          damp1(0), damp2(0), damp3(0), damp4(0),
          feedback(700), damping(500), mix(512)
    {
        int i;
        for (i = 0; i < REVERB_COMB1_SIZE; i++) comb1[i] = 0;
        for (i = 0; i < REVERB_COMB2_SIZE; i++) comb2[i] = 0;
        for (i = 0; i < REVERB_COMB3_SIZE; i++) comb3[i] = 0;
        for (i = 0; i < REVERB_COMB4_SIZE; i++) comb4[i] = 0;
        for (i = 0; i < REVERB_AP1_SIZE;   i++) ap1[i]   = 0;
        for (i = 0; i < REVERB_AP2_SIZE;   i++) ap2[i]   = 0;
    }

    void setFeedback(int fb)  { feedback = fb; }   // 0–950
    void setDamping(int d)    { damping  = d;  }   // 0–900
    void setMix(int m)        { mix      = m;  }   // 0–1024

    // Call every audio sample.
    int process(int in)
    {
        // 4 parallel combs
        int wet = tickComb(comb1, REVERB_COMB1_SIZE, &c1i, &damp1, in)
                + tickComb(comb2, REVERB_COMB2_SIZE, &c2i, &damp2, in)
                + tickComb(comb3, REVERB_COMB3_SIZE, &c3i, &damp3, in)
                + tickComb(comb4, REVERB_COMB4_SIZE, &c4i, &damp4, in);

        wet >>= 2;  // average 4 combs

        // 2 series allpass
        wet = tickAllpass(ap1, REVERB_AP1_SIZE, &a1i, wet);
        wet = tickAllpass(ap2, REVERB_AP2_SIZE, &a2i, wet);

        wet = clamp(wet);

        // Wet/dry mix (fixed-point, scale 1024)
        return clamp(((mix * wet) + ((1024 - mix) * in)) >> 10);
    }
};

#endif
