/*  Elmyra — Electronic Shruti Box
 *
 *  Three capacitive touch pads act as reed stops: root, fifth (or major
 *  third), octave.  Long breath-like envelopes, per-voice LFO detuning
 *  for organic aliveness, a resonant LP filter, and a Schroeder reverb.
 *
 *  Signal chain:
 *    Voices (×3) → sum → LP Filter → Reverb → PWM output
 *
 *  Original Elmyra hardware by Neutral Labs.
 *  Firmware by Charles Gershom / sixbitglitch.
 */

#include "elmyraGlobalDefines.hpp"
#include "synthCtx.hpp"
#include "audioBuffer.hpp"
#include "Oscillator.hpp"
#include "slewEnvelope.hpp"
#include "LFO.hpp"
#include "Filter.hpp"
#include "Reverb.hpp"
#include "ioUpdate.hpp"

int io_update_countdown, tmp_sample;
bool tmp_sample_waiting;
audioBuffer sampleBuffer;
synthCtx ctx;

void setupTimerInterrupt()
{
    REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1);
    while (GCLK->STATUS.bit.SYNCBUSY == 1);
    Tcc* TC = (Tcc*) TCC1;
    TC->CTRLA.reg &= ~TCC_CTRLA_ENABLE;
    while (TC->SYNCBUSY.bit.ENABLE == 1);
    TC->CTRLA.reg |= TCC_CTRLA_PRESCALER_DIV1;
    TC->WAVE.reg   |= TCC_WAVE_WAVEGEN_NFRQ;
    while (TC->SYNCBUSY.bit.WAVE == 1);
    TC->PER.reg = CLOCK_RATE / SAMPLE_RATE;
    while (TC->SYNCBUSY.bit.PER == 1);
    TC->CC[0].reg = 0xFFF;
    while (TC->SYNCBUSY.bit.CC0 == 1);
    TC->INTENSET.reg = 0;
    TC->INTENSET.bit.OVF = 1;
    TC->INTENSET.bit.MC0 = 1;
    NVIC_EnableIRQ(TCC1_IRQn);
    TC->CTRLA.reg |= TCC_CTRLA_ENABLE;
    while (TC->SYNCBUSY.bit.ENABLE == 1);
}

void TCC1_Handler()
{
    Tcc* TC = (Tcc*) TCC1;
    int sample;

    if (TC->INTFLAG.bit.OVF == 1)
    {
        io_update_countdown--;

        if (sampleBuffer.getSample(&sample))
        {
            analogWrite(PIN_OUT_AUDIO, BIAS + sample);
        }

        TC->INTFLAG.bit.OVF = 1;
    }

    if (TC->INTFLAG.bit.MC0 == 1)
    {
        TC->INTFLAG.bit.MC0 = 1;
    }
}

static inline int getOscSample(int voice)
{
    return ctx.osc[voice].getSample() * ctx.env_value[voice];
}

void fillBuffer()
{
    while (true)
    {
        if (!tmp_sample_waiting)
        {
            // Sum three voices
            tmp_sample = (getOscSample(0) + getOscSample(1) + getOscSample(2)) / AMP_MAX / NUM_VOICES;

            // Resonant LP filter (MOD knob)
            tmp_sample = ctx.flt.process(tmp_sample);

            // Schroeder reverb (F.BACK / TIME / MIX knobs)
            tmp_sample = ctx.rev.process(tmp_sample);

            // Hard clip
            if (tmp_sample >  AMP_MAX) tmp_sample =  AMP_MAX;
            if (tmp_sample < -AMP_MAX) tmp_sample = -AMP_MAX;
        }

        if (!sampleBuffer.putSample(tmp_sample))
        {
            tmp_sample_waiting = true;
            break;
        }
        else
        {
            tmp_sample_waiting = false;
        }
    }
}

void loop()
{
    if (io_update_countdown <= 0)
    {
        ioUpdate(&ctx);
        io_update_countdown = SAMPLE_RATE / IO_UPDATE_FREQ;
    }

    fillBuffer();

    delayMicroseconds(RELAX_TIME);
}

void setup()
{
    int i;

#ifdef SERIAL_DEBUG
    Serial.begin(9600);
#endif

    io_update_countdown = 0;
    tmp_sample_waiting  = false;

    pinMode(PIN_IN_GSR_1, INPUT);
    pinMode(PIN_IN_GSR_2, INPUT);
    pinMode(PIN_IN_GSR_3, INPUT);
    pinMode(PIN_IN_WAVE_1, INPUT_PULLUP);
    pinMode(PIN_IN_WAVE_2, INPUT_PULLUP);
    pinMode(PIN_IN_WAVE_3, INPUT_PULLUP);
    pinMode(PIN_IN_ENV_1, INPUT_PULLUP);
    pinMode(PIN_IN_ENV_2, INPUT_PULLUP);
    pinMode(PIN_IN_ENV_3, INPUT_PULLUP);
    pinMode(PIN_IN_TUNE_1, INPUT);
    pinMode(PIN_IN_TUNE_2, INPUT);
    pinMode(PIN_IN_TUNE_3, INPUT);
    pinMode(PIN_IN_MOD, INPUT);
    pinMode(PIN_IN_DELAY_FEEDBACK, INPUT);
    pinMode(PIN_IN_DELAY_TIME, INPUT);
    pinMode(PIN_IN_DELAY_MIX, INPUT);

    pinMode(PIN_OUT_ENV_1, OUTPUT);
    pinMode(PIN_OUT_ENV_2, OUTPUT);
    pinMode(PIN_OUT_ENV_3, OUTPUT);

    analogWriteResolution(10);
    analogReadResolution(12);

    for (i = 0; i < AUDIO_BUF_PREFILL; i++)
    {
        sampleBuffer.putSample(0);
    }

    setupTimerInterrupt();
}
