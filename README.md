# Elmyra — Evolving Drone + Algo Perc Firmware

Elmyra is a DIY drone synthesizer by Neutral Labs, inspired by the Soma Lyra-8.
It runs on the Adafruit ItsyBitsy M0 Express with added analog distortion and filtering.
Build docs and original design: https://neutral-labs.com/elmyra

---

## This firmware (v0.2)

A full redesign of the AE Alt Firmware, focused on two things:

- **Evolving drones** — three voices with per-voice LFOs at irrational rate ratios, creating slow harmonic beating that never repeats
- **Algorithmic percussion** — voice 3 is driven by a Euclidean rhythm engine locked to the delay time, while voices 1 & 2 stay as touch-gated drones

A single **Mod knob texture sweep** ties it together: one gesture simultaneously closes a resonant software LP filter and widens the LFO drift, taking the sound from clean and open to dark, wobbly, and screamy.

---

## Signal chain

```
Voice 1 ─┐
Voice 2 ─┼─ sum ──► Software LP Filter ──► Delay (wet/dry) ──► Output
Voice 3 ─┘              ▲                        ▲
                   Mod knob                 F.BACK knob
                (cutoff + resonance)      (self-oscillates at
                                          high res + feedback)
```

LFOs modulate each voice's pitch independently.
Euclidean clock gates voice 3's envelope.
Filter sits pre-delay — echoes carry the filtered timbre.

---

## Controls

### Global hardware (not in firmware — always active)

| Control | Function |
|---------|----------|
| POWER | On/off |
| VOL | Output volume (analog) |
| SCRATCH | Analog distortion / filter — the filthy one |
| BITE | Analog clipping |

### Global firmware

| Control | Function |
|---------|----------|
| **MOD** | One-knob texture sweep — see below |
| **TIME** | Delay echo length **+** Euclidean BPM (coupled) |
| **F.BACK** | Delay feedback — push high with MOD up for self-oscillation |
| **MIX** | Delay wet/dry |

### Per-voice (3 rows)

| Control | Voice 1 & 2 — drones | Voice 3 — percussion |
|---------|----------------------|----------------------|
| **TUNE knob** | Base pitch | Perc hit pitch |
| **ENV toggle** | Slow / fast envelope | Slow / fast gate length |
| **ENV double-tap** | Bypass — voice always fully on | Same |
| **SHAPE toggle** | Oscillator slew (saw ↔ softer) | Same |
| **SHAPE 1 double-tap** | LFO mode: triangle ↔ S&H | — |
| **SHAPE 2 double-tap** | Cross-FM on / off | — |
| **SHAPE 3 double-tap** | — | Next Euclidean pattern |
| **Touch pads** | Gate drone open/closed | Fire a manual perc hit |

### LEDs

| LED | Behaviour |
|-----|-----------|
| Voice 1 & 2 | On when envelope is open |
| Voice 3 | Flashes on every Euclidean beat |

---

## Features

### Mod knob — one-knob texture sweep

Three things move simultaneously with a single knob:

1. **Software LP filter cutoff** closes: ~7 kHz → ~150 Hz
2. **Filter resonance** builds: transparent → screamy (Q ≈ 0.5 → 6.4)
3. **LFO pitch drift depth** widens: 0 → ±20000 mHz

At zero: clean oscillators, full bandwidth, no drift.
At full: dark, muffled, resonant peak, wide pitch wander.

Stack SCRATCH (hardware distortion) on top and push F.BACK high — at high resonance + high delay feedback the filter self-oscillates.

### Per-voice LFOs — evolving drones

Each voice has an independent slow LFO:

| Voice | Rate | Period |
|-------|------|--------|
| 1 | 80 mHz | ~12 s |
| 2 | 113 mHz | ~9 s |
| 3 | 67 mHz | ~15 s |

The rate ratios are irrational (≈ √2 relationships) so the beating pattern between voices never repeats on a human timescale. Depth is set by the MOD knob.

**SHAPE 1 double-tap** switches all LFOs between:
- **Triangle** — smooth, continuous pitch glide
- **S&H** — stepped random jumps on each LFO cycle

### Euclidean percussion — voice 3

Voice 3 is driven by a Bjorklund Euclidean rhythm engine. The clock tempo is locked to the TIME knob, so the rhythm grid and the delay echo length move together — rhythmic delays line up with the pattern naturally.

**SHAPE 3 double-tap** cycles through 8 presets:

| # | Pattern | Character |
|---|---------|-----------|
| 0 | 4 in 16 | Even quarters — steady pulse |
| 1 | 3 in 8 | Tresillo — classic Afro-Cuban |
| 2 | 5 in 16 | Sparse push-pull |
| 3 | 5 in 8 | Dense syncopation |
| 4 | 7 in 16 | Hemiola feel |
| 5 | 7 in 12 | Latin 7 |
| 6 | 9 in 16 | Very dense |
| 7 | 3 in 16 | Wide gaps, sparse |

Touch pad 3 fires manual hits on top of whatever the Euclidean pattern is doing. ENV toggle sets gate length (short = percussive click, long = sustained hit).

The LFO on voice 3 also runs, adding slight pitch variation between hits — each strike can land on a slightly different frequency for a melodic perc feel.

### Cross-FM

**SHAPE 2 double-tap** routes voice 1's amplitude into voice 2's pitch (±15% of base pitch). When voice 1 is gated open and loud, voice 2 rises in pitch. Combines with both voices' LFOs for complex, organic timbral movement.

### Resonant LP filter

Two-pole state variable filter inserted before the delay. Parameters are set entirely by the MOD knob — there is no separate filter control. The filter is fully open (transparent) at MOD = 0 and fully engaged at MOD = max.

Because it sits pre-delay, every echo repeats the filtered, resonant timbre. At high resonance + high delay feedback the system can tip into self-oscillation — a continuous pitched ring that can be tuned via the TIME and F.BACK knobs.

---

## Changelog

**v0.2 — redesign (this version)**
- Added per-voice LFOs for evolving pitch drift
- Added Euclidean percussion engine on voice 3
- Added two-pole resonant LP filter (pre-delay)
- Added cross-FM (voice 1 → voice 2 pitch)
- Mod knob redesigned as one-knob texture sweep (LFO depth + filter cutoff + resonance)
- Manual sequencer removed

**v0.1 — AE Alt Firmware by @CharlesGershom**
- Extended pitch range to ultra-low frequency clicks
- Random oscillator noise removed from default; moved to Mod knob
- Mod max values increased
- Slew for note changes dramatically reduced

**Stock firmware — Neutral Labs**
- Original Elmyra firmware
