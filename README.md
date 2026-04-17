# Elmyra — Electronic Shruti Box

A portable electronic shruti box firmware for the Neutral Labs Elmyra.
Three capacitive touch pads become reed stops. Hold them like a real shruti box —
individually or in combination — to build chords of sustained drone tones that sit
warmly alongside ukuleles, fiddles, and acoustic shruti boxes in a folk session.

Original Elmyra hardware by Neutral Labs: https://neutral-labs.com/elmyra
Runs on the Adafruit ItsyBitsy M0 Express.

---

## What it is

A shruti box is a small bellows-driven reed instrument from Indian classical music,
used as a drone reference in folk and classical sessions. This firmware turns the
Elmyra into a handmade electronic equivalent: warm, harmonically present, and
slightly alive — never perfectly static.

---

## Signal chain

```
Touch pad 1 (root)  ─┐
Touch pad 2 (fifth) ─┼─ sum ──► LP Filter ──► Reverb ──► Output
Touch pad 3 (octave)─┘
      ↑
  Per-voice LFOs add
  subtle pitch drift
```

---

## Controls

### Hardware (analog circuits — always active)

| Control | Function |
|---------|----------|
| POWER | On/off |
| VOL | Output volume |
| SCRATCH | Analog distortion — adds harmonic grit on top of the digital voice |
| BITE | Analog hard clip |

### Firmware

**Tune knobs**

| Knob | Function |
|------|----------|
| TUNE 1 | Root note — one octave of chromatic pitches (12 semitones); octave band set by SHAPE 3 |
| TUNE 2 | Reed detuning — how wide the LFO pitch drift is (0 = dead still, full = ±2 Hz flutter) |
| TUNE 3 | Reed breathing speed — LFO rate (0.5 Hz slow tremolo → 3 Hz faster flutter) |

**Tone / space knobs**

| Knob | Function |
|------|----------|
| MOD | Instrument — selects one of 8 timbral presets (0 = Natural → 7 = Wild; see below) |
| TIME | Reverb amount — single knob; scales decay length and warmth together (low = no reverb, high = long warm room) |
| F.BACK | Tone brightness — LP filter cutoff + resonance sweep (low = open/bright, high = dark/resonant) |
| MIX | Reverb wet/dry |

**Per-voice toggles (3 rows)**

| Control | Function |
|---------|----------|
| SHAPE 1 toggle | Oscillator slew — saw (crisp) ↔ softer (rounder), affects all three stops |
| SHAPE 2 toggle | Middle stop interval — HIGH = perfect fifth, LOW = major third |
| SHAPE 3 toggle | Octave band — HIGH = normal (C4 range), LOW = low (C3 range) |
| ENV 1/2/3 toggle | Envelope speed per stop — slow (breath-like, ~2.5 s attack / 4 s release) ↔ fast |
| ENV double-tap | Bypass — stop stays fully on with no gate needed |

**Touch pads**

| Pad | Stop |
|-----|------|
| Touch pad 1 | Root |
| Touch pad 2 | Fifth (or major third — set by SHAPE 2) |
| Touch pad 3 | Octave |

Hold any combination simultaneously. Release to let the tone fade slowly.

**LEDs** — lit while that stop's envelope is open.

---

## Features

### Reed stops

Three independent drone voices tuned to the root, an interval above the root,
and an octave above the root. SHAPE 2 toggle switches the middle stop between
a perfect fifth (traditional shruti box interval) and a major third (for
major-key drones).

Pitches are quantized to the chromatic scale so the instrument stays in tune
with other players. TUNE 1 covers one octave of chromatic pitches. SHAPE 3
selects the octave band: normal (centred around C4, folk session range) or
low (one octave below, C3, for bass drone work).

### Breath-like envelopes

The default ENV speed gives a ~2.5 second attack and ~4 second release — long
enough to feel like breath, short enough to be responsive. Fast ENV mode gives
near-instant gating. Double-tap any ENV button to lock that stop open permanently.

### Reed detuning (organic aliveness)

Each voice has its own LFO running at a slightly different rate. The three rates
are in irrational ratios (×1.0 / ×1.13 / ×0.84) so they drift in and out of phase
in a pattern that never exactly repeats. TUNE 2 controls the depth (zero = perfectly
static, useful for checking tuning) and TUNE 3 controls the base speed.

### MOD — instrument selector

Eight preset timbres selected by a single knob. Each changes the oscillator
waveform, voice intervals, and LFO baseline character. TUNE 2/3 then sculpt
detuning and breathing speed on top of the preset character.

| # | Name | Character |
|---|------|-----------|
| 0 | Natural | Pure sawtooth, root/fifth/octave — most acoustic, user controls all texture |
| 1 | Harmonium | Rounded wave, gentle built-in flutter — classic shruti box |
| 2 | Reed | Mid-forward, chorus detune between voices for audible beating |
| 3 | Tanpura | Soft, voice 2 at fifth+octave (19 semitones) for shimmer |
| 4 | Bagpipe | Bright square-ish, fourth instead of fifth, faster tremolo |
| 5 | Temple | Round wave, root/fifth/double-octave, very slow LFO |
| 6 | Cluster | Root/third/fifth stack — close harmonic cluster, no octave |
| 7 | Wild | Minor seventh + major tenth, heavy detune, fast wavering |

On preset 0 (Natural), SHAPE 2 still switches the middle stop between fifth and
major third. On all other presets the intervals are fixed by the preset.

### F.BACK — tone brightness

Turns a single knob into a tone sweep: as F.BACK increases, a two-pole resonant LP
filter closes (from ~7 kHz down to ~150 Hz) and a resonance peak builds at the
cutoff. Low = open and bright. High = dark and resonant.
Stack SCRATCH (hardware distortion) for extra harmonic richness.

### Reverb

A Schroeder reverb (4 comb filters + 2 allpass filters) replaces the tape delay.
- **TIME** — single reverb amount knob; scales decay length and warmth together
- **MIX** — how much reverb is in the mix

---

## Changelog

**v0.3 — Shruti Box (this version)**
- Complete firmware replacement: three reed stops, chromatic root tuning
- Breath-like envelopes (2.5 s attack / 4 s release default)
- Per-voice LFO detuning with irrational rate ratios for organic aliveness
- SHAPE 2 toggle: perfect fifth ↔ major third (Natural preset only)
- Schroeder algorithmic reverb (replaces tape delay)
- MOD = 8 instrument presets; F.BACK = tone brightness; TIME = one-knob reverb

**v0.2 — Evolving Drone + Algo Perc**
- Per-voice drift LFOs, Euclidean percussion engine, cross-FM, tape delay

**v0.1 — AE Alt Firmware by @CharlesGershom**
- Extended pitch range, reduced slew, mod-knob noise control

**Stock — Neutral Labs**
- Original Elmyra firmware
