#ifndef SYNTHCTX_HPP
#define SYNTHCTX_HPP

#include "slewEnvelope.hpp"
#include "Oscillator.hpp"
#include "LFO.hpp"
#include "Filter.hpp"
#include "Reverb.hpp"

class synthCtx
{
public:
    // ── Core DSP objects ───────────────────────────────────────────────────
    Oscillator    osc[NUM_VOICES];
    slewEnvelope  env[NUM_VOICES];
    LFO           lfo[NUM_VOICES];
    Filter        flt;
    Reverb        rev;

    // ── Per-voice state ────────────────────────────────────────────────────
    int touch_value[NUM_VOICES];
    int env_value[NUM_VOICES];
    int tune_value[NUM_VOICES];
    int env_speed[NUM_VOICES];
    int osc_slew[NUM_VOICES];
    int osc_tune[NUM_VOICES];      // base pitch in mHz (from scale[])
    int lfo_value[NUM_VOICES];     // current LFO output (mHz offset)

    // ── Envelope / wave button state ───────────────────────────────────────
    int envBypass[NUM_VOICES];
    int envState[NUM_VOICES];
    int envStateCountdown[NUM_VOICES];
    int waveState[NUM_VOICES];
    int waveSpecialMode[NUM_VOICES];       // unused in shruti but mechanism kept
    int waveSpecialModeCountdown[NUM_VOICES];

    // ── Shruti pitch state ─────────────────────────────────────────────────
    int shrutiRoot;         // index into scale[] (0–SHRUTI_ROOT_MAX)

    // ── Raw pot values (for smoothing) ────────────────────────────────────
    int mod_value_raw;       // MOD knob — instrument selector
    int brightness_raw;      // F.BACK knob — tone brightness / filter sweep
    int rev_damping_raw;     // TIME knob  — reverb amount (feedback + damping)
    int rev_mix_raw;         // MIX knob   — reverb wet/dry

    synthCtx()
    {
        int i;

        for (i = 0; i < NUM_VOICES; i++)
        {
            osc_slew[i] = OSC_WAVE_SLEW_LOW;
            osc[i].setSampleRate(SAMPLE_RATE);
            osc[i].setSlew(osc_slew[i]);
            osc[i].setModAmount(0);
            env[i].setMax(ENV_MAX);
            env[i].setMin(ENV_MIN);
            env[i].setAttack(SHRUTI_ENV_ATTACK);
            env[i].setRelease(SHRUTI_ENV_RELEASE);
            envBypass[i]             = 0;
            envState[i]              = 0;
            envStateCountdown[i]     = 0;
            waveSpecialMode[i]       = 0;
            waveState[i]             = 0;
            waveSpecialModeCountdown[i] = 0;
            tune_value[i]            = 0;
            env_speed[i]             = 1;
            lfo_value[i]             = 0;
            touch_value[i]           = 0;
            env_value[i]             = 0;
            osc_tune[i]              = scale[36]; // C4 default
        }

        shrutiRoot      = 36;   // C4
        mod_value_raw   = 0;
        brightness_raw  = 0;
        rev_damping_raw = 0;
        rev_mix_raw     = 0;

        // LFO rates: per-voice multipliers ×1.0 / ×1.13 / ×0.84 of base rate.
        // Base rate set in ioUpdate from TUNE 3; hardcode reasonable startup default.
        lfo[0].setRate(1000, SAMPLE_RATE);
        lfo[1].setRate(1130, SAMPLE_RATE);
        lfo[2].setRate( 840, SAMPLE_RATE);
        for (i = 0; i < NUM_VOICES; i++)
        {
            lfo[i].setDepth(0);
            lfo[i].setMode(LFO::MODE_TRIANGLE);
        }
    }
};

#endif
