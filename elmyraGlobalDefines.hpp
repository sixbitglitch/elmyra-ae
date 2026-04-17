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

// ── Input smoothing ────────────────────────────────────────────────────────
#define SMOOTHING_FACTOR_TUNE    1
#define SMOOTHING_FACTOR_NORMAL  5
#define SMOOTHING_FACTOR_TOUCH   5
#define POT_DEAD_ZONE_TUNE       1
#define POT_DEAD_ZONE_NORMAL    30

// ── Envelope ───────────────────────────────────────────────────────────────
#define ENV_MAX               500
#define ENV_MIN                50
#define ENV_ATTACK              2   // stock default (overridden per mode)
#define ENV_RELEASE             2
#define ENV_SPEED_FACTOR       32
#define SPECIAL_MODE_ENABLE_TIME 1

// ── Shruti box envelope ────────────────────────────────────────────────────
// Called at IO_UPDATE_FREQ (25 Hz); ENV_MAX = 500
//   attack  8  → 500/8  = 62 updates × 40ms ≈ 2.5 s  (breath in)
//   release 5  → 500/5  = 100 updates × 40ms ≈ 4 s   (slow exhale)
// Fast ENV mode (ENV_SPEED_FACTOR = 32) gives near-instant response.
#define SHRUTI_ENV_ATTACK       8
#define SHRUTI_ENV_RELEASE      5

// ── Shruti pitch ───────────────────────────────────────────────────────────
// TUNE 1 covers exactly one octave (12 semitones) within a selected octave band.
// SHAPE 3 toggle selects the band:
//   HIGH = normal range, root at C4 (scale index 36)
//   LOW  = low range,    root at C3 (scale index 24)
// root + 12 (octave voice) must stay within scale[] (max index 75) — both bands satisfy this.
#define SHRUTI_OCTAVE_SIZE     12   // one octave = 12 semitones
#define SHRUTI_OCTAVE_NORMAL   36   // C4 ≈ 262 Hz
#define SHRUTI_OCTAVE_LOW      24   // C3 ≈ 131 Hz

// ── Shruti LFO (reed detuning / breathing) ─────────────────────────────────
// TUNE 3 sweeps rate across this range (mHz).
// TUNE 2 sets depth up to SHRUTI_DETUNE_MAX (mHz).
// Per-voice rate multipliers create slightly different rates → beating.
#define SHRUTI_LFO_RATE_MIN   500    // mHz  (0.5 Hz — slow flutter)
#define SHRUTI_LFO_RATE_MAX  3000    // mHz  (3.0 Hz — faster tremolo)
#define SHRUTI_DETUNE_MAX    2000    // mHz  (±2 Hz max pitch offset)

// ── Oscillator ─────────────────────────────────────────────────────────────
#define OSC_WAVE_SLEW_LOW        1
#define OSC_WAVE_SLEW_HIGH      10
#define OSC_SLEW_SCALE           1

// ── Resonant LP filter (MOD knob one-sweep) ───────────────────────────────
// Coefficients × 256 fixed-point.
// OPEN  ≈ 7 kHz  (f = 2sin(π·7000/24000)·256 ≈ 380)
// CLOSED≈ 150 Hz (f = 2sin(π·150/24000)·256  ≈ 8)
#define FILTER_F_OPEN      380
#define FILTER_F_CLOSED      8
#define FILTER_DAMP_OPEN   512   // 1/Q=2.0, no resonance
#define FILTER_DAMP_CLOSED  40   // 1/Q≈0.16, resonant peak

// ── Reverb pot scaling ─────────────────────────────────────────────────────
// F.BACK → feedback 0–950 (feedback inside comb, ×1024 fixed-point)
// TIME   → damping  0–900 (HF rolloff inside comb, ×1024 fixed-point)
// MIX    → mix      0–1024 (wet/dry, ×1024 fixed-point)
#define REVERB_FEEDBACK_MAX   950
#define REVERB_DAMPING_MAX    900
#define REVERB_MIX_MAX       1024

// ── SHAPE toggle special meanings (SHAPE 2 only — physical toggle switch) ──
// SHAPE 2 HIGH = perfect fifth (+7 semitones)
// SHAPE 2 LOW  = major third  (+4 semitones)
// SHAPE 1 = oscillator slew (saw ↔ softer) — same as always
// SHAPE 3 = unused / available

// ── Pin definitions ────────────────────────────────────────────────────────
#define PIN_IN_GSR_1         A3
#define PIN_IN_GSR_2         A2
#define PIN_IN_GSR_3         A1
#define PIN_IN_TUNE_1        A5
#define PIN_IN_TUNE_2        20
#define PIN_IN_TUNE_3        21
#define PIN_IN_MOD           A4
#define PIN_IN_DELAY_FEEDBACK 22   // → reverb decay
#define PIN_IN_DELAY_TIME    23    // → reverb damping
#define PIN_IN_DELAY_MIX     25    // → reverb wet/dry
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

// ── Chromatic scale table (mHz, 76 notes C0–E7) ───────────────────────────
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
