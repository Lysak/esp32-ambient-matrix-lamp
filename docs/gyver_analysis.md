# GyverLamp2 Analysis & Effect Roadmap

> Written after reviewing GyverLamp2 app screenshots and comparing with the current effects-core implementation.  
> Source app: https://github.com/AlexGyver/GyverLamp2

---

## 0. Why GyverLamp v1 → v2 Redesign: The "Ocean" Story

### The v1 problem

In GyverLamp v1 every visual "look" was a separate named effect: Ocean, Lava, Forest, Rainbow, Boomerang, etc.

Under the hood, Ocean / Lava / Forest were all Perlin noise with different hardcoded color tables — the same algorithm duplicated three times. If the user wanted "Ocean but faster" there was no option; a new hardcoded effect "Ocean Fast" would have to be written. Effect count grew without bound as users asked for variations.

### The v2 solution: separate algorithm from parameters

GyverLamp2 made a clean split:

```
Algorithm (HOW pixels move)  ←  fixed code, small set
    +
Parameters (HOW it looks)    ←  speed, scale, palette
    =
Preset (a named combination) ←  user-facing label
```

"Ocean" in v1 = Perlin algorithm + blue color table hardcoded inside.  
"Ocean" in v2 = Perlin algorithm + Ocean palette **as a parameter**.

This means:
- 1 Perlin algorithm × 8 palettes × many speed/scale values = hundreds of looks with zero code duplication
- "Perlin Тепло 1", "Perlin Тепло 2", "Perlin Океан" are all presets over one algorithm

The Scanner / Boomerang effect (from v1) disappeared in v2 because it was a niche look that did not generalise well into the algorithm+parameter model. It survives in our project as a custom addition.

### What this means for our project

We follow the same principle:
- Keep a **small set of effect algorithms** in `effects-core`
- Define **named presets** as static C++ structs: `{name, effect_id, speed, scale, palette}`
- Expose **only the preset list** as the primary HA select entity
- Speed/Scale/Palette sliders remain available for manual fine-tuning on top of preset defaults but are not the primary UX

---

## 1. GyverLamp2 UI Structure (from screenshots)

The app has five main tabs: **УПРАВЛЕНИЕ**, **КОНФИГ**, **РЕЖИМЫ**, **ПАЛИТРА**, **РАССВЕТ**, **НАСТРОЙКИ**, **ИНФО**.

### УПРАВЛЕНИЕ (Control)

- Power buttons: off / on / timer
- `<` `>` arrows to cycle through presets
- Dropdown showing the **current preset name** (e.g. "Перлин Тепло 1")
- The main screen shows **presets**, not raw effects

### РЕЖИМЫ (Presets editor)

This is where presets are defined. One preset = a named combination of parameters:

| Field | Type | Notes |
|---|---|---|
| Name | text | User-defined, e.g. "Перлин Тепло 1" |
| Эффект | dropdown | Selects the effect algorithm |
| Уменьшить яркость | toggle | Reduce brightness flag |
| Дополнительно | dropdown | Mic/ADC modulation mode |
| Скорость | slider | Speed (0–255) |
| Палитра | dropdown | Color palette |
| Масштаб | slider | Scale (0–255) |

Buttons: **+** (add preset), **-** (remove preset), **ИМПОРТ/ЭКСПОРТ**

### Effect list in GyverLamp2 (from dropdown in РЕЖИМЫ)

| # | Name (RU) | Name (EN) |
|---|---|---|
| 1 | Перлин | Perlin Noise |
| 2 | Цвет | Solid Color |
| 3 | Смена | Color Transition |
| 4 | Градиент | Gradient |
| 5 | Частицы | Particles |
| 6 | Костёр | Campfire |
| 7 | Огонь | Fire |
| 8 | Конфетти | Confetti |
| 9 | Смерч | Tornado |
| 10 | Часы | Clock |
| 11 | Погода | Weather |

**Boomerang/Scanner** — NOT in GyverLamp2. It was in GyverLamp v1 (old version).  
**Fire 2020** — NOT in GyverLamp2 UI either. It's a hidden/separate variant.  
**Океан (Ocean)** — NOT a separate effect. "Ocean" = Perlin effect + Ocean palette. It's a preset, not an effect.

### Дополнительно (Additional / Mic modulation)

Each preset can optionally link a mic/ADC channel to some parameter:

| Option | Meaning |
|---|---|
| Нет | No mic modulation |
| Громкость | Modulate by total volume |
| Низкие | Modulate by bass frequencies (FFT low band) |
| Высокие | Modulate by treble frequencies (FFT high band) |
| Часы | Clock display overlay mode |

This is per-preset. The modulated parameter depends on the effect (brightness, scale, or speed).

### РАССВЕТ (Sunrise alarm)

- Enable/disable per weekday (7 toggles)
- Alarm time per day
- Max brightness slider
- Minutes before sunrise (fade-in duration)
- Minutes after sunrise (hold duration)
- UPLOAD button to save settings to device

---

## 2. Key Architectural Insight

In GyverLamp2: **presets are the primary unit of UI, not effects**.

```
Preset "Перлин Тепло 1" = {
    effect   = Perlin,
    speed    = ~80%,
    palette  = Heat (Тепло),
    scale    = ~30%,
    dim      = false,
    extra    = None
}

Preset "Перлин Океан" = {
    effect   = Perlin,
    speed    = ~50%,
    palette  = Ocean,
    scale    = ~60%,
    extra    = None
}
```

The user browses and selects **presets**, not raw effects. Effects are just the algorithm behind a preset.

Up to 40 presets, 13 bytes each, stored in EEPROM.

---

## 3. Our Project vs GyverLamp2 — Effect Mapping

| GyverLamp2 Effect | Our Implementation | Status |
|---|---|---|
| Перлин (Perlin) | `EffectPerlin` | ✅ implemented |
| Цвет (Color) | `EffectColor` | ✅ implemented |
| Смена (Color Transition) | `EffectColorShift` | ✅ implemented |
| Градиент (Gradient) | `EffectGradient` | ✅ implemented |
| Частицы (Particles) | `EffectParticles` | ✅ implemented |
| Костёр (Campfire) | `EffectCampfire` | ✅ implemented |
| Огонь (Fire) | `EffectFire` | ⚠️ bug: shows mostly white |
| Конфетти (Confetti) | `EffectConfetti` | ✅ implemented |
| Смерч (Tornado) | `EffectTornado` | ✅ implemented |
| Часы (Clock) | — | ❌ not yet |
| Погода (Weather) | — | ❌ planned Phase 4 |
| Scanner / Boomerang | `EffectScanner` | ⚠️ our addition; speed bug |
| Огонь 2020 (Fire2020) | `EffectFire2020` | ⚠️ our addition; one-time init issue |
| Ocean | not a separate effect | use Perlin + Ocean palette |

---

## 4. Known Bugs in Current Effects

### Bug 1 — EffectScanner: speed parameter ignored

**File:** `effects-core/src/effect_scanner.cpp:25`

```cpp
static constexpr uint32_t kCycleMs = 4800;  // ← hardcoded, never changes
```

The `tick()` signature accepts `const EffectParams&` but ignores it entirely. The scan cycle is always 4.8 seconds regardless of the Speed slider in Home Assistant.

**Fix:** Map `params.speed` to cycle duration.  
Example: speed 0 → 8000 ms cycle (slow), speed 128 → 4000 ms, speed 255 → 1200 ms (fast).

```cpp
const uint32_t kCycleMs = 8000 - (uint32_t)params.speed * 26;
```

---

### Bug 2 — EffectFire: flame too hot, shows almost white

**File:** `effects-core/src/effect_fire.cpp:28-30`

```cpp
uint8_t cooling = 2 + (255 - params.scale) / 16;   // max 17 at scale=0
uint8_t sparking = 80 + params.speed / 3;            // 80–165
```

At default `speed=128`, `scale=128`:
- cooling = **9** — very weak cooling
- sparking = **122** — 48% chance per column per step

The `kHeatColors` palette ends in white at index 240+. With weak cooling and high sparking frequency, heat values in the upper rows stay above 200, pushing into yellow→white range.

**Comment in code is wrong:** it says "scale 0 = slow cooling (tall flames)" but the formula gives the opposite — scale=0 → cooling=17 (heavy, short flames); scale=255 → cooling=2 (light, tall white flames).

**Fix options:**
1. Increase base cooling: `uint8_t cooling = 25 + (255 - params.scale) / 8;`
2. Reduce sparking: `uint8_t sparking = 50 + params.speed / 5;`
3. Fix the scale→cooling comment to match the actual formula direction.

---

### Bug 3 — EffectFire2020: scale change ignored after first frame

**File:** `effects-core/src/effect_fire2020.cpp`

```cpp
if (!initialized_) {
    initialized_ = true;
    delta_value_ = map_int(params.scale, 0, 255, 8, 168);
    // ...
}
```

`delta_value_`, `delta_hue_`, `step_` are computed **once** from `params.scale` at first tick. If the user changes Scale in HA while the effect is running, it has no effect until the lamp reboots or the effect is switched and switched back.

**Fix:** Either remove `initialized_` guard and recompute each frame (cheap), or re-initialize when `params.scale` changes (compare with stored `last_scale_`).

---

## 5. "Ocean" Effect — How to Achieve It

Ocean is not a separate effect in GyverLamp2. It is:

```
EffectPerlin + PaletteId::Ocean
```

The `kOceanColors` palette is already defined in `effects-core/src/palette.cpp`:

```cpp
// Dark blue → blue → teal → cyan → white
const Palette16 kOceanColors = {{
    {  0,  0, 16}, {  0,  0, 48}, ...
}};
```

**To add "Ocean" to ESPHome:** add a second `addressable_lambda` for the Perlin effect with the palette forced to Ocean and appropriate default speed/scale.

```yaml
- addressable_lambda:
    name: "Ocean"
    update_interval: 16ms
    lambda: |-
      if (initial_run) {
        engine.set_effect(ambient_matrix::EffectId::Perlin);
        engine.set_params({100, 80, 0, ambient_matrix::PaletteId::Ocean, true});
      }
      // ...
```

Or better: implement a named preset system (see Section 6).

---

## 6. Preset System — Design for Our Project

### Current state (as of July 2026)

`select.lamp_effect` in HA shows raw algorithm names: "Color", "Gradient", "Perlin", "Fire", etc.  
`number.lamp_speed`, `number.lamp_scale` and palette index are separate HA entities.  
The user selects an effect and manually adjusts sliders — no named presets exist yet.

### Design decision (fixed)

**Presets are defined once in C++ code. There is no HA UI for creating or editing presets.**

Rationale:
- Keeps HA dashboard simple — one dropdown, done
- No risk of users leaving the lamp in a broken or ugly state
- Custom looks are added by editing code and reflashing, not through a UI builder
- Matches the way the project is developed: parameters are tuned once in code and reused

### What HA shows

A single `select.lamp_preset` dropdown with preset names. Examples:

```
Perlin Warm
Perlin Ocean
Perlin Forest
Perlin Cold
Fire Classic
Campfire
Confetti
Particles
Tornado
Rainbow
Color Shift
Scanner
...
```

The existing speed/scale/palette sliders **may be kept** as developer/debug entities (hidden in the main HA dashboard, visible in device settings) for tuning new presets. But the primary day-to-day UX is just the preset dropdown.

### Preset struct in C++

```cpp
struct Preset {
    const char* name;
    EffectId    effect;
    uint8_t     speed;
    uint8_t     scale;
    PaletteId   palette;
    // MicMode  mic_mode;  // ← add later when sound-reactive presets are implemented
};

static const Preset kPresets[] = {
    // Perlin variants (algorithm: Perlin, palette changes the mood)
    {"Perlin Warm",   EffectId::Perlin,    100, 80,  PaletteId::Heat},
    {"Perlin Ocean",  EffectId::Perlin,    80,  100, PaletteId::Ocean},
    {"Perlin Forest", EffectId::Perlin,    70,  120, PaletteId::Forest},
    {"Perlin Cold",   EffectId::Perlin,    90,  90,  PaletteId::Cloud},
    {"Perlin Party",  EffectId::Perlin,    140, 80,  PaletteId::Party},

    // Fire variants
    {"Fire Classic",  EffectId::Fire,      128, 180, PaletteId::Heat},
    {"Campfire",      EffectId::Campfire,  100, 150, PaletteId::Heat},
    {"Fire2020",      EffectId::Fire2020,  100, 128, PaletteId::Heat},

    // Motion / particle effects
    {"Confetti",      EffectId::Confetti,  128, 128, PaletteId::Rainbow},
    {"Particles",     EffectId::Particles, 128, 100, PaletteId::Party},
    {"Tornado",       EffectId::Tornado,   100, 128, PaletteId::Rainbow},
    {"Matrix Rain",   EffectId::MatrixRain,100, 128, PaletteId::Forest},

    // Calm / ambient
    {"Rainbow",       EffectId::Rainbow,   100, 128, PaletteId::Rainbow},
    {"Color Shift",   EffectId::ColorShift, 80, 128, PaletteId::Rainbow},
    {"Gradient",      EffectId::Gradient,   90, 128, PaletteId::Rainbow},

    // Scanner (from GyverLamp v1, our custom addition)
    {"Scanner",       EffectId::Scanner,   128, 128, PaletteId::Rainbow},
    {"Blue Scanner",  EffectId::BlueScanner,128,128, PaletteId::Ocean},
};
```

### ESPHome wiring

```yaml
select:
  - platform: template
    name: "Lamp Preset"
    id: lamp_preset
    options:
      - "Perlin Warm"
      - "Perlin Ocean"
      - "Perlin Forest"
      # ... (must match C++ kPresets names exactly)
    set_action:
      - lambda: |-
          apply_preset(x.c_str());
```

`apply_preset()` looks up the preset by name in `kPresets[]`, then calls `engine.set_effect()` and `engine.set_params()` with the preset's values.

---

## 7. Sound / Mic Integration

### What GyverLamp2 does with sound

Each preset has an **"Дополнительно"** (Additional) parameter that links the microphone to a rendering parameter:

| ADC mode | Effect on rendering |
|---|---|
| None | No mic influence |
| Volume | Scale brightness by current volume level |
| Bass (Low) | Scale some effect parameter by FFT low-band energy |
| Treble (High) | Scale some effect parameter by FFT high-band energy |
| Clock | Clock mode overlay |

GyverLamp2 FFT implementation (`FFT_C.h`): 64-point FFT, bit-reversal + butterfly + magnitude, output normalized by right-shift ×18.

GyverLamp2 Volume analyzer (`VolAnalyzer.h`):
- Samples every 500 µs, 20-sample window
- Exponential smoothing every 150 ms
- Outputs: raw, peak, filtered vol (0–100), pulse flag

### Our hardware

INMP441 I²S MEMS microphone connected to GPIO32 (SCK), GPIO33 (WS), GPIO34 (SD).

Files already in project:
- `effects-core/include/ambient_matrix/mic/vol_analyzer.h` — volume analysis
- `effects-core/include/ambient_matrix/mic/clap_detector.h` — clap detection
- `esphome/custom/mic_reader.h` — I²S reader for ESP32

### Planned sound-reactive preset parameters

When implementing the preset system, add a `mic_mode` field per preset:

```cpp
enum class MicMode : uint8_t {
    None,
    Volume,   // modulate brightness by volume
    Bass,     // modulate scale by FFT low band
    Treble,   // modulate speed by FFT high band
};

struct Preset {
    const char* name;
    EffectId effect;
    uint8_t speed;
    uint8_t scale;
    PaletteId palette;
    MicMode mic_mode;  // ← new
};
```

At render time, before calling `engine.tick()`:

```cpp
if (preset.mic_mode == MicMode::Volume) {
    uint8_t vol = mic_reader.volume();  // 0–255
    canvas.set_brightness_scale(vol);
} else if (preset.mic_mode == MicMode::Bass) {
    params.scale = mic_reader.bass();
}
```

### Clap detection

Already implemented (`clap_detector.h`). Default mapping:

| Claps | Action |
|---|---|
| 1 | Toggle lamp on/off |
| 2 | Next preset |
| 3 | Previous preset |

---

## 8. Sunrise Alarm

GyverLamp2 has per-weekday alarm with fade-in/out duration. Our plan (from `idea.md`):

```
HA stores schedule → ESPHome exposes Sunrise effect + start/stop controls
→ HA triggers at alarm time → ESPHome renders sunrise animation locally
→ At final time, HA plays soft wake sound via media_player
```

GyverLamp2 sunrise config (24 bytes): 7 × (enable flag + hour + minute), target brightness, fade-in duration, post-alarm duration.

For our project: store schedule in HA, not in ESP32. ESPHome only needs:
- A sunrise `Effect` that animates from dark-red → orange → warm white
- `button.lamp_start_sunrise` and `button.lamp_stop_sunrise` entities

---

## 9. Action Plan (Prioritized)

### Immediate fixes

1. **Fix Scanner speed** — map `params.speed` to `kCycleMs` in `effect_scanner.cpp`
2. **Fix Fire white flames** — increase cooling base value in `effect_fire.cpp`
3. **Fix Fire2020 re-init** — remove one-time init guard, recompute params each frame

### Short term

4. **Add Ocean preset** — ESPHome lambda: EffectPerlin + PaletteId::Ocean with tuned speed/scale
5. **Preset system v1** — named preset select in ESPHome, static table in C++

### Medium term

6. **Mic modulation per preset** — add `MicMode` field to preset struct; wire to vol_analyzer output
7. **Fix scale→cooling comment** in `effect_fire.cpp` to match actual formula direction

### Long term

8. **Clock effect** — `drawClock()` using 5×7 font on 16×16 matrix
9. **Weather effects** — per HA weather entity state
10. **FFT-reactive presets** — bass/treble modulation on Perlin, Gradient, Particles

---

## 10. Palette Reference

Palettes already in `effects-core/src/palette.cpp`:

| PaletteId | Description |
|---|---|
| `Heat` | Black → dark red → orange → yellow → white |
| `Lava` | Black → deep red → red → orange (no white) |
| `Campfire` | Ember black → deep red → orange → warm yellow |
| `Rainbow` | Full hue cycle |
| `RainbowStripe` | Each hue separated by black |
| `Ocean` | Dark blue → blue → teal → cyan → white |
| `Forest` | Deep green → lime → yellow-green |
| `Cloud` | Blue/grey/white sky gradient |
| `Party` | Purple/magenta/blue/cyan/yellow/red |

GyverLamp2 has 32 palettes total — our current 9 cover the most useful ones. Additional palettes can be added to `palette.h` / `palette.cpp` and `PaletteId` enum as needed.
