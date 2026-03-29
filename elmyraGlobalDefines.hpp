#ifndef ELMYRAGLOBALDEFINES_HPP
#define ELMYRAGLOBALDEFINES_HPP

#define SERIAL_DEBUG
//#define SERIAL_DEBUG_TOUCH

// ── Audio ──────────────────────────────────────────────────────────────────
#define BIAS          512
#define AMP_MAX       511
#define POT_MAX      4095
#define LED_PIN_MAX   254
#define CLOCK_RATE  48000000
#define SAMPLE_RATE    24000
#define RELAX_TIME         5
#define IO_UPDATE_FREQ    25
#define IO_ADVANCE   (SAMPLE_RATE / IO_UPDATE_FREQ)   // samples per ioUpdate tick
#define NUM_VOICES         3
#define AUDIO_BUF_SIZE  1000
#define AUDIO_BUF_PREFILL 100
#define DELAY_BUF_SIZE 12000

// ── Input smoothing ────────────────────────────────────────────────────────
#define SMOOTHING_FACTOR_TUNE    1
#define SMOOTHING_FACTOR_NORMAL  5
#define SMOOTHING_FACTOR_TOUCH   5
#define POT_DEAD_ZONE_TUNE       1
#define POT_DEAD_ZONE_NORMAL    30

// ── Envelope ───────────────────────────────────────────────────────────────
#define ENV_MAX          500
#define ENV_MIN           50
#define ENV_ATTACK         2
#define ENV_RELEASE        2
#define ENV_SPEED_FACTOR  32
#define SPECIAL_MODE_ENABLE_TIME 1

// ── Oscillator ─────────────────────────────────────────────────────────────
#define OSC_WAVE_SLEW_LOW        1
#define OSC_WAVE_SLEW_HIGH      10
#define OSC_SLEW_SCALE           1
#define OSC_MOD_AMOUNT_MAX     128
#define POT_TUNE_BASE        20000
#define POT_TUNE_SCALE         100

// ── Delay ──────────────────────────────────────────────────────────────────
#define DELAY_POT_SCALE_MIX       4095
#define DELAY_POT_SCALE_TIME       500
#define DELAY_POT_SCALE_FEEDBACK  1023
#define DELAY_TIME_SLEW            500
#define DELAY_TIME_FINETUNE_POINT 1000
#define DELAY_TIME_FINETUNE_FACTOR   2

// ── LFO (slow pitch drift) ─────────────────────────────────────────────────
// Three voices get slightly different rates; irrational ratios ensure
// the beating pattern never repeats on a human timescale.
#define LFO_RATE_VOICE_0    80   // mHz  (0.080 Hz, ~12 s cycle)
#define LFO_RATE_VOICE_1   113   // mHz  (0.113 Hz, ratio ≈ √2 vs voice 0)
#define LFO_RATE_VOICE_2    67   // mHz  (0.067 Hz, ratio ≈ √(1/sqrt2) vs voice 0)
#define LFO_DEPTH_MAX    20000   // mHz  (max pitch offset at full mod knob)

// ── Euclidean percussion clock ─────────────────────────────────────────────
// Clock tempo is derived from the Delay Time knob.
// EUCLID_STEP_PERIOD_MIN = ~120 BPM at 16th notes (8 steps/s)
// EUCLID_STEP_PERIOD_MAX = ~0.5 steps/s (very slow clock)
#define EUCLID_STEP_PERIOD_MIN   3000L   // samples per step (fast)
#define EUCLID_STEP_PERIOD_MAX  48000L   // samples per step (slow)
#define EUCLID_STEP_PERIOD_DEFAULT 6000L // startup default

// Perc gate length in ioUpdate ticks (at 25 Hz):
#define PERC_GATE_SLOW  8   // ~320 ms  (use with slow env speed)
#define PERC_GATE_FAST  2   // ~ 80 ms  (use with fast env speed)

// ── Cross-FM (voice 0 amplitude modulates voice 1 pitch) ──────────────────
// Expressed as % of voice 1 base pitch; 15 means ±15% range.
#define CROSSFM_SCALE   15

// ── Special mode indices (wave button double-tap) ──────────────────────────
#define SPECIAL_MODE_LFO_STEPPED_NUM  0   // wave1 double-tap: triangle ↔ S&H LFO
#define SPECIAL_MODE_CROSSFM_NUM      1   // wave2 double-tap: cross-FM on / off
#define SPECIAL_MODE_EUCLID_NEXT_NUM  2   // wave3 double-tap: next Euclidean preset

// ── Pin definitions ────────────────────────────────────────────────────────
#define PIN_IN_GSR_1         A3
#define PIN_IN_GSR_2         A2
#define PIN_IN_GSR_3         A1
#define PIN_IN_TUNE_1        A5
#define PIN_IN_TUNE_2        20
#define PIN_IN_TUNE_3        21
#define PIN_IN_MOD           A4
#define PIN_IN_DELAY_FEEDBACK 22
#define PIN_IN_DELAY_TIME    23
#define PIN_IN_DELAY_MIX     25
#define PIN_IN_WAVE_1         7
#define PIN_IN_WAVE_2        12
#define PIN_IN_WAVE_3         2
#define PIN_IN_ENV_1         SCK
#define PIN_IN_ENV_2        MOSI
#define PIN_IN_ENV_3        MISO
#define PIN_OUT_AUDIO        A0
#define PIN_OUT_ENV_1        13
#define PIN_OUT_ENV_2        11
#define PIN_OUT_ENV_3        10

// ── Chromatic scale table (mHz, 76 notes) ─────────────────────────────────
#define SCALE_MAX 76

static const int scale[] = \
{ 20600, 21830, 23120, 24500, 25960, 27500, 29140, 30870, 32700, 34650, 36710, 38890, \
  41200, 43650, 46250, 49000, 51910, 55000, 58270, 61740, 65410, 69300, 73420, 77780, \
  82410, 87310, 92500, 98000, 103800, 110000, 116500, 123500, 130800, 138600, 146800, \
  155600, 164800, 174600, 185000, 196000, 207700, 220000, 233100, 246900, 261600, \
  277200, 293700, 311100, 329600, 349200, 370000, 392000, 415300, 440000, 466200, \
  493900, 523300, 554400, 587300, 622300, 659300, 698500, 740000, 784000, 830600, \
  880000, 932300, 987800, 1047000, 1109000, 1175000, 1245000, 1319000, 1397000, \
  1480000, 1568000, 1661000 };

#endif
