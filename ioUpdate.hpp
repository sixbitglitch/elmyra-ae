#ifndef IOUPDATE_HPP
#define IOUPDATE_HPP

#include "synthCtx.hpp"

#define ENV_BYPASS (-1)

// ── Low-level input helpers ────────────────────────────────────────────────

static inline int getValueFromTouchSensor(int input, int *oldValue, int smoothing_factor)
{
    int newValue = analogRead(input);
    *oldValue = (*oldValue * (smoothing_factor - 1) + newValue) / smoothing_factor;
    return *oldValue;
}

static inline int getValueFromPot(int input, int *oldValue, int dead_zone, int smoothing_factor)
{
    int newValue = analogRead(input);
    if (newValue < *oldValue - dead_zone || newValue > *oldValue + dead_zone)
    {
        *oldValue = (*oldValue * (smoothing_factor - 1) + newValue) / smoothing_factor;
    }
    return *oldValue;
}

static inline int getValueFromPotTune(int input, int *oldValue, int dead_zone, int smoothing_factor)
{
    int newValue = analogRead(input);
    int factor;
    if (newValue < *oldValue - 200 || newValue > *oldValue + 200)
    {
        factor = 5;
    }
    else
    {
        factor = 2000 / (abs(*oldValue - newValue) + 1) + 2;
    }
    *oldValue = (*oldValue * (factor - 1) + newValue) / factor;
    return *oldValue;
}

static inline int scalePotValue(int potValue, int scale)
{
    return (potValue * scale) / POT_MAX;
}

// ── Touch input ────────────────────────────────────────────────────────────
// Each pad gates its reed stop independently.

static inline void updateInputTouch(synthCtx *ctx)
{
    ctx->touch_value[0] = getValueFromTouchSensor(PIN_IN_GSR_1, &ctx->touch_value[0], SMOOTHING_FACTOR_TOUCH);
    ctx->touch_value[1] = getValueFromTouchSensor(PIN_IN_GSR_2, &ctx->touch_value[1], SMOOTHING_FACTOR_TOUCH);
    ctx->touch_value[2] = getValueFromTouchSensor(PIN_IN_GSR_3, &ctx->touch_value[2], SMOOTHING_FACTOR_TOUCH);

#ifdef SERIAL_DEBUG_TOUCH
    Serial.print("t0: "); Serial.print(ctx->touch_value[0]);
    Serial.print("\tt1: "); Serial.print(ctx->touch_value[1]);
    Serial.print("\tt2: "); Serial.println(ctx->touch_value[2]);
#endif
}

// ── ENV speed buttons ──────────────────────────────────────────────────────
// Single toggle: slow / fast envelope.
// Double-tap: bypass (voice stays fully on — good for open drone).

static inline int updateSingleEnvSpeed(synthCtx *ctx, int voice, int input)
{
    int stateChanged;
    int state = ((digitalRead(input) == HIGH) ? 1 : ENV_SPEED_FACTOR);

    if (ctx->envStateCountdown[voice] > 0)
    {
        ctx->envStateCountdown[voice]--;
    }

    if (ctx->envState[voice] != state)
    {
        stateChanged = 1;
        ctx->envState[voice] = state;
        ctx->envBypass[voice] = 0;
    }
    else
    {
        stateChanged = 0;
    }

    if (stateChanged)
    {
        if (ctx->envStateCountdown[voice] > 0)
        {
            ctx->envBypass[voice] = 1;
            ctx->envStateCountdown[voice] = 0;
            return ENV_BYPASS;
        }
        else
        {
            ctx->envStateCountdown[voice] = SPECIAL_MODE_ENABLE_TIME * IO_UPDATE_FREQ;
        }
    }
    else
    {
        if (ctx->envBypass[voice]) return ENV_BYPASS;
    }

    ctx->envBypass[voice] = 0;
    return state;
}

static inline void updateInputEnvSpeed(synthCtx *ctx)
{
    ctx->env_speed[0] = updateSingleEnvSpeed(ctx, 0, PIN_IN_ENV_1);
    ctx->env_speed[1] = updateSingleEnvSpeed(ctx, 1, PIN_IN_ENV_2);
    ctx->env_speed[2] = updateSingleEnvSpeed(ctx, 2, PIN_IN_ENV_3);
}

// ── SHAPE / Wave toggles ───────────────────────────────────────────────────
// SHAPE 1: oscillator slew (saw ↔ softer) — applies to ALL three voices.
// SHAPE 2: middle stop interval — read in updateShrutiPitch.
// SHAPE 3: octave band select   — read in updateShrutiPitch.

static inline void updateInputOscWave(synthCtx *ctx)
{
    int slew = (digitalRead(PIN_IN_WAVE_1) == HIGH) ? OSC_WAVE_SLEW_LOW : OSC_WAVE_SLEW_HIGH;
    ctx->osc_slew[0] = slew;
    ctx->osc_slew[1] = slew;
    ctx->osc_slew[2] = slew;
}

// ── Shruti pitch + filter sweep ────────────────────────────────────────────
// TUNE 1 → root note within one octave (12 semitones)
// TUNE 2 → LFO detuning depth (0 → SHRUTI_DETUNE_MAX mHz)
// TUNE 3 → LFO rate (SHRUTI_LFO_RATE_MIN → SHRUTI_LFO_RATE_MAX mHz)
// MOD    → filter cutoff + resonance sweep
// SHAPE 2 → interval for voice 1: HIGH = perfect fifth (+7), LOW = major third (+4)
// SHAPE 3 → octave band: HIGH = normal (C4), LOW = low (C3)

static inline void updateShrutiPitch(synthCtx *ctx)
{
    // Root note — one octave range, band selected by SHAPE 3
    int tune1Raw   = getValueFromPotTune(PIN_IN_TUNE_1, &ctx->tune_value[0], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE);
    int octaveBase = (digitalRead(PIN_IN_WAVE_3) == HIGH) ? SHRUTI_OCTAVE_NORMAL : SHRUTI_OCTAVE_LOW;
    int semitone   = ((long)tune1Raw * SHRUTI_OCTAVE_SIZE) / (POT_MAX + 1);  // 0–11
    ctx->shrutiRoot = octaveBase + semitone;

    // Voice 0: root
    ctx->osc_tune[0] = scale[ctx->shrutiRoot];

    // Voice 1: fifth or major third (SHAPE 2 toggle)
    int interval = (digitalRead(PIN_IN_WAVE_2) == HIGH) ? 7 : 4;
    ctx->osc_tune[1] = scale[ctx->shrutiRoot + interval];

    // Voice 2: octave
    ctx->osc_tune[2] = scale[ctx->shrutiRoot + 12];

    // LFO depth from TUNE 2
    int tune2Raw  = getValueFromPot(PIN_IN_TUNE_2, &ctx->tune_value[1], POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);
    int detuneDepth = (tune2Raw * SHRUTI_DETUNE_MAX) / POT_MAX;
    ctx->lfo[0].setDepth(detuneDepth);
    ctx->lfo[1].setDepth(detuneDepth);
    ctx->lfo[2].setDepth(detuneDepth);

    // LFO rate from TUNE 3 — apply irrational per-voice multipliers for beating
    int tune3Raw  = getValueFromPot(PIN_IN_TUNE_3, &ctx->tune_value[2], POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);
    int baseRate  = SHRUTI_LFO_RATE_MIN + ((tune3Raw * (SHRUTI_LFO_RATE_MAX - SHRUTI_LFO_RATE_MIN)) / POT_MAX);
    ctx->lfo[0].setRate(baseRate,              SAMPLE_RATE);   // ×1.00
    ctx->lfo[1].setRate((baseRate * 113) / 100, SAMPLE_RATE);  // ×1.13
    ctx->lfo[2].setRate((baseRate *  84) / 100, SAMPLE_RATE);  // ×0.84

    // MOD → filter cutoff + resonance (one-knob sweep)
    int modRaw = getValueFromPot(PIN_IN_MOD, &ctx->mod_value_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);
    ctx->mod_value = modRaw;
    int f_fixed    = FILTER_F_OPEN    - (int)((long)(FILTER_F_OPEN    - FILTER_F_CLOSED)    * modRaw / POT_MAX);
    int damp_fixed = FILTER_DAMP_OPEN - (int)((long)(FILTER_DAMP_OPEN - FILTER_DAMP_CLOSED) * modRaw / POT_MAX);
    ctx->flt.setParams(f_fixed, damp_fixed);
}

// ── LFOs ──────────────────────────────────────────────────────────────────
// Advance each LFO by one ioUpdate window.
// Rates and depths are already set in updateShrutiPitch.

static inline void updateLFOs(synthCtx *ctx)
{
    ctx->lfo_value[0] = ctx->lfo[0].tick(IO_ADVANCE);
    ctx->lfo_value[1] = ctx->lfo[1].tick(IO_ADVANCE);
    ctx->lfo_value[2] = ctx->lfo[2].tick(IO_ADVANCE);
}

// ── Reverb controls ────────────────────────────────────────────────────────
// F.BACK → reverb decay (feedback 0–REVERB_FEEDBACK_MAX)
// TIME   → reverb damping / HF rolloff (0–REVERB_DAMPING_MAX)
// MIX    → wet/dry (0–REVERB_MIX_MAX)

static inline void updateReverb(synthCtx *ctx)
{
    int fb  = getValueFromPot(PIN_IN_DELAY_FEEDBACK, &ctx->rev_feedback_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);
    int dmp = getValueFromPot(PIN_IN_DELAY_TIME,     &ctx->rev_damping_raw,  POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);
    int mix = getValueFromPot(PIN_IN_DELAY_MIX,      &ctx->rev_mix_raw,      POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);

    ctx->rev.setFeedback((fb  * REVERB_FEEDBACK_MAX) / POT_MAX);
    ctx->rev.setDamping ((dmp * REVERB_DAMPING_MAX)  / POT_MAX);
    ctx->rev.setMix     ((mix * REVERB_MIX_MAX)      / POT_MAX);
}

// ── Voice updates ─────────────────────────────────────────────────────────
// All three voices use the shruti breath envelope.
// Frequency = base pitch from scale[] + LFO offset.
// Touch pad gates envelope; ENV toggle sets speed; double-tap = bypass.

static inline void updateVoices(synthCtx *ctx)
{
    int i;

    for (i = 0; i < NUM_VOICES; i++)
    {
        // Oscillator
        ctx->osc[i].setSlew(ctx->osc_slew[i]);
        ctx->osc[i].setModAmount(0);   // clean oscillators; texture comes from filter
        int freq = ctx->osc_tune[i] + ctx->lfo_value[i];
        if (freq < 1) freq = 1;
        ctx->osc[i].setFreq(freq);

        // Envelope
        if (ctx->env_speed[i] == ENV_BYPASS)
        {
            ctx->env_value[i] = AMP_MAX;
        }
        else
        {
            int attack  = SHRUTI_ENV_ATTACK  * ctx->env_speed[i];
            int release = SHRUTI_ENV_RELEASE * ctx->env_speed[i];
            ctx->env[i].setAttack(attack);
            ctx->env[i].setRelease(release);
            ctx->env_value[i] = ctx->env[i].getLevel(ctx->touch_value[i]);
        }
    }
}

// ── LEDs ───────────────────────────────────────────────────────────────────
// On when the stop's envelope is open (touching that pad).

static inline void updateOutputEnvLED(synthCtx *ctx)
{
    digitalWrite(PIN_OUT_ENV_1, ctx->env_value[0] ? HIGH : LOW);
    digitalWrite(PIN_OUT_ENV_2, ctx->env_value[1] ? HIGH : LOW);
    digitalWrite(PIN_OUT_ENV_3, ctx->env_value[2] ? HIGH : LOW);
}

// ── Main IO update entry point ─────────────────────────────────────────────

void ioUpdate(synthCtx *ctx)
{
    updateInputTouch(ctx);
    updateInputEnvSpeed(ctx);
    updateInputOscWave(ctx);
    updateShrutiPitch(ctx);
    updateLFOs(ctx);
    updateReverb(ctx);
    updateVoices(ctx);
    updateOutputEnvLED(ctx);
}

#endif
