#ifndef SYNTHCTX_HPP
#define SYNTHCTX_HPP

#include "Delay.hpp"
#include "slewEnvelope.hpp"
#include "Oscillator.hpp"
#include "LFO.hpp"
#include "Euclidean.hpp"
#include "Filter.hpp"

class synthCtx
{
public:
    // ── Core DSP objects ───────────────────────────────────────────────────
    Oscillator     osc[NUM_VOICES];
    Delay          dly;
    slewEnvelope   env[NUM_VOICES];
    LFO            lfo[NUM_VOICES];    // one slow LFO per voice for pitch drift
    EuclideanClock euclid;             // Euclidean rhythm engine for voice 2 (perc)
    Filter         flt;               // resonant LP filter, pre-delay

    // ── Per-voice state ────────────────────────────────────────────────────
    int touch_value[NUM_VOICES];
    int env_led_level[NUM_VOICES];
    int env_value[NUM_VOICES];
    int tune_value[NUM_VOICES];
    int env_speed[NUM_VOICES];
    int osc_slew[NUM_VOICES];
    int osc_tune[NUM_VOICES];          // base pitch from tune pot (mHz)
    int lfo_value[NUM_VOICES];         // current LFO output (mHz offset)

    // ── Delay / mod state ──────────────────────────────────────────────────
    int delay_feedback, delay_time, delay_wet_raw, mod_value, mod_value_raw;
    int delay_wet;

    // ── Envelope / wave button state ───────────────────────────────────────
    int envBypass[NUM_VOICES];
    int envState[NUM_VOICES];
    int envStateCountdown[NUM_VOICES];
    int waveState[NUM_VOICES];
    int waveSpecialMode[NUM_VOICES];
    int waveSpecialModeCountdown[NUM_VOICES];

    // ── Percussion (voice 2) ───────────────────────────────────────────────
    int percTriggerCount;   // ioUpdate ticks remaining for the perc gate

    // ── Cross-FM ──────────────────────────────────────────────────────────
    int crossFM;            // 0 = off, 1 = voice 0 amplitude → voice 1 pitch

    synthCtx()
    {
        int i;

        for (i = 0; i < NUM_VOICES; i++)
        {
            osc_slew[i] = OSC_WAVE_SLEW_LOW;
            osc[i].setSampleRate(SAMPLE_RATE);
            osc[i].setSlew(osc_slew[i]);
            env[i].setMax(ENV_MAX);
            env[i].setMin(ENV_MIN);
            env[i].setAttack(ENV_ATTACK);
            env[i].setRelease(ENV_RELEASE);
            envBypass[i]            = 0;
            envState[i]             = 0;
            envStateCountdown[i]    = 0;
            waveSpecialMode[i]      = 0;
            waveState[i]            = 0;
            waveSpecialModeCountdown[i] = 0;
            tune_value[i]           = 0;
            env_speed[i]            = 1;
            lfo_value[i]            = 0;
        }

        // Slightly different rates per voice → slow, never-repeating beating
        lfo[0].setRate(LFO_RATE_VOICE_0, SAMPLE_RATE);
        lfo[1].setRate(LFO_RATE_VOICE_1, SAMPLE_RATE);
        lfo[2].setRate(LFO_RATE_VOICE_2, SAMPLE_RATE);

        euclid.setPreset(0);
        euclid.setStepPeriod(EUCLID_STEP_PERIOD_DEFAULT);

        percTriggerCount = 0;
        crossFM          = 0;

        dly.setSampleRate(SAMPLE_RATE);
        dly.setTime(delay_time = 0);
        dly.setFeedback(delay_feedback = 0);
        delay_wet = 0;
    }
};

#endif
