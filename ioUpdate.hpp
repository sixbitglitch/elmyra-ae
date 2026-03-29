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
        factor = 2000 / abs(*oldValue - newValue) + 2;
    }
    *oldValue = (*oldValue * (factor - 1) + newValue) / factor;
    return *oldValue;
}

static inline int scalePotValue(int potValue, int scale)
{
    return (potValue * scale) / POT_MAX;
}

// ── Touch input ────────────────────────────────────────────────────────────
// Voices 0 & 1: gate drones (same as stock).
// Voice 2:      touch fires a manual perc trigger (does NOT gate voice 2 directly;
//               the euclidean engine owns voice 2 – touch adds extra hits on top).

static inline void updateInputTouch(synthCtx *ctx)
{
    ctx->touch_value[0] = getValueFromTouchSensor(PIN_IN_GSR_1, &ctx->touch_value[0], SMOOTHING_FACTOR_TOUCH);
    ctx->touch_value[1] = getValueFromTouchSensor(PIN_IN_GSR_2, &ctx->touch_value[1], SMOOTHING_FACTOR_TOUCH);
    ctx->touch_value[2] = getValueFromTouchSensor(PIN_IN_GSR_3, &ctx->touch_value[2], SMOOTHING_FACTOR_TOUCH);

    // Manual perc trigger: sustained touch on sensor 3 keeps the gate open.
    if (ctx->touch_value[2] > ENV_MIN)
    {
        int gateLen = (ctx->env_speed[2] == 1) ? PERC_GATE_SLOW : PERC_GATE_FAST;
        ctx->percTriggerCount = gateLen;
    }

#ifdef SERIAL_DEBUG_TOUCH
    Serial.print("t0: "); Serial.print(ctx->touch_value[0]);
    Serial.print("\tt1: "); Serial.print(ctx->touch_value[1]);
    Serial.print("\tt2: "); Serial.println(ctx->touch_value[2]);
#endif
}

// ── Envelope speed buttons ─────────────────────────────────────────────────
// Unchanged from stock: toggle slow/fast envelope; double-tap = bypass.

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

// ── Wave / slew buttons ────────────────────────────────────────────────────
// Single press: toggle oscillator slew (same as stock).
// Double-tap special modes (new meanings):
//   Voice 0 (wave1): toggle LFO mode  triangle ↔ S&H  [SPECIAL_MODE_LFO_STEPPED_NUM]
//   Voice 1 (wave2): toggle cross-FM  on / off         [SPECIAL_MODE_CROSSFM_NUM]
//   Voice 2 (wave3): advance Euclidean pattern preset   [SPECIAL_MODE_EUCLID_NEXT_NUM]

static inline int updateSingleOscWave(synthCtx *ctx, int voice, int input)
{
    int stateChanged;
    int state = ((digitalRead(input) == HIGH) ? OSC_WAVE_SLEW_LOW : OSC_WAVE_SLEW_HIGH);

    if (ctx->waveSpecialModeCountdown[voice] > 0)
    {
        ctx->waveSpecialModeCountdown[voice]--;
    }

    if (ctx->waveState[voice] != state)
    {
        stateChanged = 1;
        ctx->waveState[voice] = state;
    }
    else
    {
        stateChanged = 0;
    }

    if (stateChanged)
    {
        if (ctx->waveSpecialModeCountdown[voice] > 0)
        {
            ctx->waveSpecialMode[voice] = ctx->waveSpecialMode[voice] ? 0 : 1;
            ctx->waveSpecialModeCountdown[voice] = 0;
            return state;
        }
        else
        {
            ctx->waveSpecialModeCountdown[voice] = SPECIAL_MODE_ENABLE_TIME * IO_UPDATE_FREQ;
        }
    }

    return state;
}

static inline void updateInputOscWave(synthCtx *ctx)
{
    ctx->osc_slew[0] = updateSingleOscWave(ctx, 0, PIN_IN_WAVE_1);
    ctx->osc_slew[1] = updateSingleOscWave(ctx, 1, PIN_IN_WAVE_2);
    ctx->osc_slew[2] = updateSingleOscWave(ctx, 2, PIN_IN_WAVE_3);
}

// ── Special mode responses ─────────────────────────────────────────────────

static inline void updateSpecialModes(synthCtx *ctx)
{
    // LFO mode: waveSpecialMode[0] toggles triangle / S&H for all voices
    int lfoMode = ctx->waveSpecialMode[SPECIAL_MODE_LFO_STEPPED_NUM]
                  ? LFO::MODE_SH : LFO::MODE_TRIANGLE;
    ctx->lfo[0].setMode(lfoMode);
    ctx->lfo[1].setMode(lfoMode);
    ctx->lfo[2].setMode(lfoMode);

    // Cross-FM: waveSpecialMode[1]
    ctx->crossFM = ctx->waveSpecialMode[SPECIAL_MODE_CROSSFM_NUM];

    // Euclidean next preset: rising edge of waveSpecialMode[2]
    static int prevEuclidSpecial = 0;
    int curEuclidSpecial = ctx->waveSpecialMode[SPECIAL_MODE_EUCLID_NEXT_NUM];
    if (curEuclidSpecial && !prevEuclidSpecial)
    {
        ctx->euclid.nextPreset();
    }
    prevEuclidSpecial = curEuclidSpecial;
}

// ── Tune pots ──────────────────────────────────────────────────────────────
// All three voices: pot → base pitch.  LFO offset is applied later in updateVoices.
// Mod knob: controls LFO depth (replaces "noise amount"; low mod = subtle drift,
// high mod = wide pitch wander).  The OSC mod amount is kept at zero by default
// so oscillators are clean; dial in mod for deliberate noise if desired.

static inline void updateInputOscTune(synthCtx *ctx)
{
    ctx->osc_tune[0] = 1 + getValueFromPotTune(PIN_IN_TUNE_1, &ctx->tune_value[0], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 200;
    ctx->osc_tune[1] = 1 + getValueFromPotTune(PIN_IN_TUNE_2, &ctx->tune_value[1], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 200;
    ctx->osc_tune[2] = 1 + getValueFromPotTune(PIN_IN_TUNE_3, &ctx->tune_value[2], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 200;

    // Mod knob → LFO depth (0 to LFO_DEPTH_MAX)
    ctx->mod_value = scalePotValue(
        getValueFromPot(PIN_IN_MOD, &ctx->mod_value_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL),
        LFO_DEPTH_MAX);
}

// ── Delay + Euclidean tempo ────────────────────────────────────────────────
// Delay knob values are unchanged.
// Additionally the raw delay-time pot value drives the Euclidean step period,
// so the rhythm and the delay echoes share a common "pulse width" feeling.

static inline void updateInputDelay(synthCtx *ctx)
{
    int new_time = getValueFromPot(PIN_IN_DELAY_TIME, &ctx->delay_time, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);

    // Euclidean step period (samples): map pot full range → slow…fast
    long euclidPeriod = EUCLID_STEP_PERIOD_MIN +
        ((long)(EUCLID_STEP_PERIOD_MAX - EUCLID_STEP_PERIOD_MIN) * (POT_MAX - new_time)) / POT_MAX;
    ctx->euclid.setStepPeriod(euclidPeriod);

    // Delay time with fine-tune at low end (unchanged from stock)
    if (new_time < DELAY_TIME_FINETUNE_POINT)
    {
        new_time /= DELAY_TIME_FINETUNE_FACTOR;
    }
    else
    {
        new_time = DELAY_POT_SCALE_TIME + (new_time - DELAY_TIME_FINETUNE_POINT);
    }
    new_time = (DELAY_POT_SCALE_TIME * new_time) / (POT_MAX - (DELAY_TIME_FINETUNE_POINT / DELAY_TIME_FINETUNE_FACTOR));
    ctx->dly.setTime(new_time);

    ctx->dly.setFeedback(scalePotValue(
        getValueFromPot(PIN_IN_DELAY_FEEDBACK, &ctx->delay_feedback, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL),
        DELAY_POT_SCALE_FEEDBACK));
    ctx->delay_wet = scalePotValue(
        getValueFromPot(PIN_IN_DELAY_MIX, &ctx->delay_wet_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL),
        DELAY_POT_SCALE_MIX);
}

// ── LFOs ──────────────────────────────────────────────────────────────────
// Tick each LFO forward by one ioUpdate window (IO_ADVANCE samples).
// Depth is set by the mod knob.  Each voice has a different base rate,
// creating slow harmonic beating that never repeats.

static inline void updateLFOs(synthCtx *ctx)
{
    int depth = ctx->mod_value;   // 0 … LFO_DEPTH_MAX

    ctx->lfo[0].setDepth(depth);
    ctx->lfo[1].setDepth(depth);
    ctx->lfo[2].setDepth(depth);

    ctx->lfo_value[0] = ctx->lfo[0].tick(IO_ADVANCE);
    ctx->lfo_value[1] = ctx->lfo[1].tick(IO_ADVANCE);
    ctx->lfo_value[2] = ctx->lfo[2].tick(IO_ADVANCE);
}

// ── Euclidean clock ────────────────────────────────────────────────────────
// Ticked every ioUpdate.  A beat fires → set percTriggerCount on voice 2.

static inline void updateEuclidean(synthCtx *ctx)
{
    if (ctx->euclid.tick((long)IO_ADVANCE))
    {
        int gateLen = (ctx->env_speed[2] == 1) ? PERC_GATE_SLOW : PERC_GATE_FAST;
        ctx->percTriggerCount = gateLen;
    }
}

// ── Voice updates ─────────────────────────────────────────────────────────
// Voices 0 & 1: evolving drones – base pitch + LFO drift + optional cross-FM.
// Voice  2:     percussive – base pitch + LFO variation, envelope gated by
//               euclidean triggers (or manual touch on sensor 3).

static inline void updateVoices(synthCtx *ctx)
{
    int i;

    for (i = 0; i < NUM_VOICES; i++)
    {
        ctx->osc[i].setSlew(ctx->osc_slew[i]);
        ctx->osc[i].setModAmount(0);   // oscillators are clean; mod goes to LFO
    }

    // ── Voice 0: drone with LFO drift ─────────────────────────────────────
    {
        int freq = ctx->osc_tune[0] + ctx->lfo_value[0];
        if (freq < 1) freq = 1;
        ctx->osc[0].setFreq(freq);

        if (ctx->env_speed[0] == ENV_BYPASS)
        {
            ctx->env_value[0] = AMP_MAX;
        }
        else
        {
            ctx->env[0].setAttack(ENV_ATTACK * ctx->env_speed[0]);
            ctx->env[0].setRelease(ENV_RELEASE * ctx->env_speed[0]);
            ctx->env_value[0] = ctx->env[0].getLevel(ctx->touch_value[0]);
        }
    }

    // ── Voice 1: drone with LFO drift + optional cross-FM from voice 0 ───
    {
        int freq = ctx->osc_tune[1] + ctx->lfo_value[1];
        if (ctx->crossFM)
        {
            // voice 0 amplitude shifts voice 1 pitch by up to ±CROSSFM_SCALE %
            int fm = (ctx->env_value[0] * ctx->osc_tune[1] * CROSSFM_SCALE) / (AMP_MAX * 100);
            freq += fm;
        }
        if (freq < 1) freq = 1;
        ctx->osc[1].setFreq(freq);

        if (ctx->env_speed[1] == ENV_BYPASS)
        {
            ctx->env_value[1] = AMP_MAX;
        }
        else
        {
            ctx->env[1].setAttack(ENV_ATTACK * ctx->env_speed[1]);
            ctx->env[1].setRelease(ENV_RELEASE * ctx->env_speed[1]);
            ctx->env_value[1] = ctx->env[1].getLevel(ctx->touch_value[1]);
        }
    }

    // ── Voice 2: Euclidean percussion ─────────────────────────────────────
    // LFO adds pitch variation between hits (each hit can land on a
    // slightly different frequency, creating a melodic perc feel).
    {
        int freq = ctx->osc_tune[2] + ctx->lfo_value[2];
        if (freq < 1) freq = 1;
        ctx->osc[2].setFreq(freq);

        // Gate: euclidean trigger or manual touch both load percTriggerCount.
        int touchVal;
        if (ctx->percTriggerCount > 0)
        {
            ctx->percTriggerCount--;
            touchVal = ENV_MAX + 10;   // hard gate open
        }
        else
        {
            touchVal = 0;              // gate closed → envelope releases
        }

        if (ctx->env_speed[2] == ENV_BYPASS)
        {
            ctx->env_value[2] = AMP_MAX;
        }
        else
        {
            ctx->env[2].setAttack(ENV_ATTACK  * ctx->env_speed[2]);
            ctx->env[2].setRelease(ENV_RELEASE * ctx->env_speed[2]);
            ctx->env_value[2] = ctx->env[2].getLevel(touchVal);
        }
    }
}

// ── LEDs ───────────────────────────────────────────────────────────────────
// Voices 0 & 1: light when envelope is open (shows drone activity).
// Voice  2: flashes on each euclidean trigger (shows rhythm).

static inline void updateOutputEnvLED(synthCtx *ctx)
{
    digitalWrite(PIN_OUT_ENV_1, ctx->env_value[0] ? HIGH : LOW);
    digitalWrite(PIN_OUT_ENV_2, ctx->env_value[1] ? HIGH : LOW);
    digitalWrite(PIN_OUT_ENV_3, ctx->percTriggerCount > 0 ? HIGH : LOW);
}

// ── Main IO update entry point ─────────────────────────────────────────────

void ioUpdate(synthCtx *ctx)
{
    updateInputTouch(ctx);
    updateInputEnvSpeed(ctx);
    updateInputOscWave(ctx);
    updateSpecialModes(ctx);
    updateInputOscTune(ctx);
    updateLFOs(ctx);
    updateInputDelay(ctx);
    updateEuclidean(ctx);
    updateVoices(ctx);
    updateOutputEnvLED(ctx);
}

#endif
