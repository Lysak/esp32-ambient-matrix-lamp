# GyverLamp v1 Effects Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port 12 missing GyverLamp v1 effects — 9 Perlin-noise variants (Madness 3D, Clouds 3D, Lava 3D, Plasma 3D, Rainbow 3D, Peacock 3D, Zebra 3D, Forest 3D, Ocean 3D) plus 3 simple effects (Color Change, Rainbow Vert, Rainbow Horiz) — into `effects-core/` and expose them in `led_matrix.yaml`.

**Architecture:** A single `EffectNoise` class with a `NoiseStyle` enum covers all 9 Perlin variants using the `fillNoiseLED` algorithm from GyverLamp v1 (`effects.ino`). Three lightweight effect classes handle the simple effects. All 12 new `EffectId` values are appended to the enum; engine.cpp gets 12 new switch cases. The YAML select list gains 12 new entries after the existing GyverLamp2 originals.

**Tech Stack:** C++17, `inoise8(x,y,z)` from `effects-core/include/ambient_matrix/noise.h`, `Palette16` + `color_from_palette()` from `palette.h`, existing `math_utils.h` helpers.

## Global Constraints

- All code and comments in English.
- No FastLED, no Arduino-specific types. Use `ambient_matrix::Rgb`, `MatrixCanvas`, `Matrix`, `inoise8`, etc.
- Port algorithm from GyverLamp v1 `effects.ino` — note source file in each `.cpp`.
- New effects go in select list as GyverLamp originals (no "2" suffix), inserted after the existing 9 GyverLamp2 originals and before the custom "2" effects.
- `matrix.xy(x, y)` — x=column (0=left), y=row (0=bottom), serpentine layout.
- Build: `make esphome-compile`. Flash: `make esphome-ota`. Sequential, never parallel.

---

## File Map

| Action  | Path |
|---------|------|
| Modify  | `effects-core/include/ambient_matrix/math_utils.h` — add `dim8_raw` |
| Modify  | `effects-core/include/ambient_matrix/palette.h` — declare 4 new palettes |
| Modify  | `effects-core/src/palette.cpp` — define kCloudColors, kPartyColors, kRainbowStripeColors, kZebraColors |
| Create  | `effects-core/include/ambient_matrix/effect_noise.h` |
| Create  | `effects-core/src/effect_noise.cpp` |
| Create  | `effects-core/include/ambient_matrix/effect_v1_simple.h` |
| Create  | `effects-core/src/effect_v1_simple.cpp` |
| Modify  | `effects-core/include/ambient_matrix/engine.h` — add 12 EffectId values |
| Modify  | `effects-core/src/engine.cpp` — add 12 switch cases + includes |
| Modify  | `esphome/common/led_matrix.yaml` — add 12 select options + 12 lambda effects |

---

### Task 1: Add `dim8_raw` to math_utils.h

**Files:**
- Modify: `effects-core/include/ambient_matrix/math_utils.h`

**Interfaces:**
- Produces: `inline uint8_t dim8_raw(uint8_t x)` — quadratic dimming, equivalent to FastLED's `dim8_raw`. Used by `EffectNoise::render_palette()`.

- [ ] **Step 1: Add the function**

In `effects-core/include/ambient_matrix/math_utils.h`, after the `add_rgb` function, add:

```cpp
// Quadratic dimming: x squared / 256. Used by noise effects for brightness shaping.
inline uint8_t dim8_raw(uint8_t x) { return scale8(x, x); }
```

- [ ] **Step 2: Verify the unit-test build**

```bash
cd /Users/Files/www/pet/esp32-ambient-matrix-lamp
cmake -S effects-core -B build -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -5
cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 3: Commit**

```bash
git add effects-core/include/ambient_matrix/math_utils.h
git commit -m "feat: add dim8_raw() quadratic dimming helper to math_utils.h"
```

---

### Task 2: Add 4 new palettes

**Files:**
- Modify: `effects-core/include/ambient_matrix/palette.h`
- Modify: `effects-core/src/palette.cpp`

**Interfaces:**
- Produces: `extern const Palette16 kCloudColors` — blue/grey/white sky gradient
- Produces: `extern const Palette16 kPartyColors` — bright multicolor (magenta/blue/cyan/yellow/red)
- Produces: `extern const Palette16 kRainbowStripeColors` — rainbow hues with black gaps between each color
- Produces: `extern const Palette16 kZebraColors` — black with white every 4th entry

- [ ] **Step 1: Declare in palette.h**

In `effects-core/include/ambient_matrix/palette.h`, after the existing `kCampfireColors` declaration, add:

```cpp
extern const Palette16 kCloudColors;
extern const Palette16 kPartyColors;
extern const Palette16 kRainbowStripeColors;
extern const Palette16 kZebraColors;
```

- [ ] **Step 2: Define in palette.cpp**

In `effects-core/src/palette.cpp`, append the four definitions:

```cpp
// Blue/grey/white sky gradient (mirrors FastLED CloudColors)
const Palette16 kCloudColors = {{
    {  0,   0, 255}, {  0,   0, 220}, { 30,  30, 255}, { 60,  60, 255},
    {128, 128, 255}, {170, 170, 255}, {200, 200, 255}, {220, 220, 255},
    {255, 255, 255}, {220, 220, 255}, {180, 180, 220}, {128, 128, 200},
    { 60,  60, 180}, { 30,  30, 160}, {  0,   0, 200}, {  0,   0, 255},
}};

// Bright party colors: purple/magenta/blue/cyan/yellow/red cycle
const Palette16 kPartyColors = {{
    { 85,   0, 128}, {129,   0, 147}, {170,   0, 154}, {186,   0, 122},
    {215,   0, 217}, {151,   0, 255}, { 12,   0, 255}, {  0,  55, 255},
    {  0, 255, 213}, {  0, 255,  85}, {  0, 255,  42}, { 55, 255,   0},
    {255, 255,   0}, {255, 128,   0}, {255,  85,   0}, {255,  42,   0},
}};

// Rainbow stripes: each hue block separated by a black entry
const Palette16 kRainbowStripeColors = {{
    {255,   0,   0}, {  0,   0,   0}, {171, 171,   0}, {  0,   0,   0},
    {  0, 255,   0}, {  0,   0,   0}, {  0, 171, 171}, {  0,   0,   0},
    {  0,   0, 255}, {  0,   0,   0}, {171,   0, 171}, {  0,   0,   0},
    {255,   0,  85}, {  0,   0,   0}, {255,   0,   0}, {  0,   0,   0},
}};

// Zebra: white stripes on black, white every 4th entry
const Palette16 kZebraColors = {{
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
}};
```

- [ ] **Step 3: Build and verify**

```bash
cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 4: Commit**

```bash
git add effects-core/include/ambient_matrix/palette.h effects-core/src/palette.cpp
git commit -m "feat: add kCloudColors, kPartyColors, kRainbowStripeColors, kZebraColors palettes"
```

---

### Task 3: Implement `EffectNoise` (9 Perlin-noise variants)

**Files:**
- Create: `effects-core/include/ambient_matrix/effect_noise.h`
- Create: `effects-core/src/effect_noise.cpp`

**Interfaces:**
- Consumes: `inoise8(x,y,z)` from `noise.h`; `qsub8`, `qadd8`, `scale8`, `dim8_raw` from `math_utils.h`; `color_from_palette`, all 6+4 palettes from `palette.h`
- Produces: `class EffectNoise : public Effect` with `enum class NoiseStyle`

**Algorithm source:** GyverLamp v1 `effects.ino`, function `fillNoiseLED()` and `mapNoiseToLEDsUsingPalette()`.

- [ ] **Step 1: Write `effect_noise.h`**

```cpp
// effects-core/include/ambient_matrix/effect_noise.h
// Based on fillNoiseLED() from GyverLamp v1 effects.ino
#pragma once
#include "canvas.h"
#include "engine.h"
#include <cstdint>
#include <cstring>

namespace ambient_matrix {

enum class NoiseStyle : uint8_t {
    Madness,    // raw HSV from noise (no palette)
    Clouds,     // kCloudColors
    Lava,       // kLavaColors
    Plasma,     // kPartyColors   + colorLoop
    Rainbow3D,  // kRainbowColors + colorLoop
    Peacock,    // kRainbowStripeColors + colorLoop
    Zebra,      // kZebraColors   + colorLoop
    Forest,     // kForestColors
    Ocean,      // kOceanColors
};

class EffectNoise : public Effect {
public:
    explicit EffectNoise(NoiseStyle style) : style_(style) {}
    void reset() override;
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;

private:
    NoiseStyle style_;
    uint16_t   x_ = 0, y_ = 0;
    uint32_t   z_ = 0;
    uint8_t    ihue_ = 0;
    uint8_t    noise_[16][16] = {};

    void fill_noise(uint8_t w, uint8_t h, uint8_t speed, uint8_t scale);
    void render_palette(MatrixCanvas& canvas, const Matrix& m,
                        const Palette16& pal, bool color_loop);
    void render_madness(MatrixCanvas& canvas, const Matrix& m);
};

} // namespace ambient_matrix
```

- [ ] **Step 2: Write `effect_noise.cpp`**

```cpp
// effects-core/src/effect_noise.cpp
// Based on fillNoiseLED() + mapNoiseToLEDsUsingPalette() from GyverLamp v1 effects.ino

#include "ambient_matrix/effect_noise.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include "ambient_matrix/types.h"
#include <cstring>

namespace ambient_matrix {

void EffectNoise::reset() {
    x_ = y_ = ihue_ = 0;
    z_ = 0;
    memset(noise_, 0, sizeof(noise_));
}

void EffectNoise::fill_noise(uint8_t w, uint8_t h, uint8_t speed, uint8_t scale) {
    // dataSmoothing: blend previous noise with new when speed is slow
    uint8_t smoothing = (speed < 50) ? (uint8_t)(200 - speed * 4) : 0;

    for (uint8_t i = 0; i < w; i++) {
        uint16_t ioff = (uint16_t)scale * i;
        for (uint8_t j = 0; j < h; j++) {
            uint16_t joff = (uint16_t)scale * j;
            uint8_t data = inoise8((uint16_t)(x_ + ioff),
                                   (uint16_t)(y_ + joff),
                                   (uint16_t)(z_ & 0xFFFF));
            // Expand the lower range (mirrors v1 post-processing)
            data = qsub8(data, 16);
            data = qadd8(data, scale8(data, 39));
            if (smoothing) {
                uint8_t old = noise_[i][j];
                data = qadd8(scale8(old, smoothing),
                             scale8(data, (uint8_t)(255 - smoothing)));
            }
            noise_[i][j] = data;
        }
    }
    z_ += speed;
    x_ += speed / 8;
    y_ -= speed / 16;
    ihue_++;
}

void EffectNoise::render_madness(MatrixCanvas& canvas, const Matrix& m) {
    uint8_t w = m.width(), h = m.height();
    for (uint8_t i = 0; i < w; i++) {
        for (uint8_t j = 0; j < h; j++) {
            // v1: CHSV(noise[j][i], 255, noise[i][j])  ← transposed axes
            canvas.set_pixel(m.xy(i, j),
                Rgb::from_hsv(noise_[j][i], 255, noise_[i][j]));
        }
    }
}

void EffectNoise::render_palette(MatrixCanvas& canvas, const Matrix& m,
                                  const Palette16& pal, bool color_loop) {
    uint8_t w = m.width(), h = m.height();
    for (uint8_t i = 0; i < w; i++) {
        for (uint8_t j = 0; j < h; j++) {
            // v1: index from noise[j][i], brightness from noise[i][j] (transposed)
            uint8_t idx = noise_[j][i];
            uint8_t bri = noise_[i][j];
            if (color_loop) idx = (uint8_t)(idx + ihue_);
            // Reshape brightness: above midpoint → full; below → quadratic roll-off
            if (bri > 127) {
                bri = 255;
            } else {
                bri = dim8_raw((uint8_t)(bri * 2));
            }
            canvas.set_pixel(m.xy(i, j), color_from_palette(pal, idx, bri));
        }
    }
}

void EffectNoise::tick(MatrixCanvas& canvas, const Matrix& matrix,
                       const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    fill_noise(w, h, params.speed ? params.speed : 1, params.scale ? params.scale : 1);

    switch (style_) {
        case NoiseStyle::Madness:
            render_madness(canvas, matrix);
            break;
        case NoiseStyle::Clouds:
            render_palette(canvas, matrix, kCloudColors, false);
            break;
        case NoiseStyle::Lava:
            render_palette(canvas, matrix, kLavaColors, false);
            break;
        case NoiseStyle::Plasma:
            render_palette(canvas, matrix, kPartyColors, true);
            break;
        case NoiseStyle::Rainbow3D:
            render_palette(canvas, matrix, kRainbowColors, true);
            break;
        case NoiseStyle::Peacock:
            render_palette(canvas, matrix, kRainbowStripeColors, true);
            break;
        case NoiseStyle::Zebra:
            render_palette(canvas, matrix, kZebraColors, true);
            break;
        case NoiseStyle::Forest:
            render_palette(canvas, matrix, kForestColors, false);
            break;
        case NoiseStyle::Ocean:
            render_palette(canvas, matrix, kOceanColors, false);
            break;
    }
}

} // namespace ambient_matrix
```

> **Note on `Rgb::from_hsv`:** Check that `types.h` provides `Rgb::from_hsv(hue, sat, val)`. If it does not exist yet, add it before this task. The HSV→RGB formula: hue 0–255 maps full circle; sat 0–255; val 0–255. Standard 6-sector integer HSV conversion.

- [ ] **Step 3: Check `Rgb::from_hsv` exists**

```bash
grep -n "from_hsv\|hsv" /Users/Files/www/pet/esp32-ambient-matrix-lamp/effects-core/include/ambient_matrix/types.h
```

If not found, add to `types.h` before the closing `}` of the `Rgb` struct:

```cpp
    static Rgb from_hsv(uint8_t h, uint8_t s, uint8_t v) {
        if (s == 0) return {v, v, v};
        uint8_t region = h / 43;
        uint8_t rem    = (h - (region * 43)) * 6;
        uint8_t p = (uint16_t)v * (255 - s) >> 8;
        uint8_t q = (uint16_t)v * (255 - ((uint16_t)s * rem >> 8)) >> 8;
        uint8_t t = (uint16_t)v * (255 - ((uint16_t)s * (255 - rem) >> 8)) >> 8;
        switch (region) {
            case 0: return {v, t, p};
            case 1: return {q, v, p};
            case 2: return {p, v, t};
            case 3: return {p, q, v};
            case 4: return {t, p, v};
            default: return {v, p, q};
        }
    }
```

- [ ] **Step 4: Build**

```bash
cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 5: Commit**

```bash
git add effects-core/include/ambient_matrix/effect_noise.h \
        effects-core/src/effect_noise.cpp \
        effects-core/include/ambient_matrix/types.h
git commit -m "feat: add EffectNoise with 9 NoiseStyle variants (GyverLamp v1 fillNoiseLED)"
```

---

### Task 4: Implement 3 simple v1 effects

**Files:**
- Create: `effects-core/include/ambient_matrix/effect_v1_simple.h`
- Create: `effects-core/src/effect_v1_simple.cpp`

**Interfaces:**
- Produces: `class EffectColorChange : public Effect` — cycles a single solid hue through the full spectrum; speed controls rotation rate
- Produces: `class EffectRainbowVert : public Effect` — vertical rainbow bands scrolling; speed controls scroll rate, scale controls band width
- Produces: `class EffectRainbowHoriz : public Effect` — horizontal rainbow bands scrolling

**Algorithm source:** GyverLamp v1 `effects.ino`, cases for colorWheel (Color Change), rainbowVertical, rainbowHorizontal.

- [ ] **Step 1: Write `effect_v1_simple.h`**

```cpp
// effects-core/include/ambient_matrix/effect_v1_simple.h
// Based on colorWheel, rainbowVertical, rainbowHorizontal from GyverLamp v1 effects.ino
#pragma once
#include "canvas.h"
#include "engine.h"

namespace ambient_matrix {

class EffectColorChange : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t hue_ = 0;
};

class EffectRainbowVert : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t offset_ = 0;
};

class EffectRainbowHoriz : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t offset_ = 0;
};

} // namespace ambient_matrix
```

- [ ] **Step 2: Write `effect_v1_simple.cpp`**

```cpp
// effects-core/src/effect_v1_simple.cpp
// Based on colorWheel, rainbowVertical, rainbowHorizontal from GyverLamp v1 effects.ino
#include "ambient_matrix/effect_v1_simple.h"
#include "ambient_matrix/types.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

void EffectColorChange::tick(MatrixCanvas& canvas, const Matrix& matrix,
                              const EffectParams& params, uint32_t) {
    Rgb c = Rgb::from_hsv(hue_, 255, 255);
    uint8_t w = matrix.width(), h = matrix.height();
    for (uint8_t x = 0; x < w; x++)
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), c);
    // speed: 1 full cycle in ~256000ms at speed=1; ~1000ms at speed=255
    hue_ = (uint8_t)(hue_ + (params.speed >> 5) + 1);
}

void EffectRainbowVert::tick(MatrixCanvas& canvas, const Matrix& matrix,
                              const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    // scale: controls hue spread across columns (0=narrow bands, 255=wide)
    uint8_t spread = params.scale ? params.scale : 16;
    for (uint8_t x = 0; x < w; x++) {
        Rgb c = Rgb::from_hsv((uint8_t)(offset_ + x * spread / w), 255, 255);
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), c);
    }
    offset_ = (uint8_t)(offset_ + (params.speed >> 5) + 1);
}

void EffectRainbowHoriz::tick(MatrixCanvas& canvas, const Matrix& matrix,
                               const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    uint8_t spread = params.scale ? params.scale : 16;
    for (uint8_t y = 0; y < h; y++) {
        Rgb c = Rgb::from_hsv((uint8_t)(offset_ + y * spread / h), 255, 255);
        for (uint8_t x = 0; x < w; x++)
            canvas.set_pixel(matrix.xy(x, y), c);
    }
    offset_ = (uint8_t)(offset_ + (params.speed >> 5) + 1);
}

} // namespace ambient_matrix
```

- [ ] **Step 3: Build**

```bash
cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 4: Commit**

```bash
git add effects-core/include/ambient_matrix/effect_v1_simple.h \
        effects-core/src/effect_v1_simple.cpp
git commit -m "feat: add EffectColorChange, EffectRainbowVert, EffectRainbowHoriz (GyverLamp v1)"
```

---

### Task 5: Register 12 new EffectIds in engine

**Files:**
- Modify: `effects-core/include/ambient_matrix/engine.h`
- Modify: `effects-core/src/engine.cpp`

**Interfaces:**
- Consumes: `EffectNoise`, `EffectColorChange`, `EffectRainbowVert`, `EffectRainbowHoriz` from tasks 3–4
- Produces: 12 new `EffectId` enum values that `led_matrix.yaml` lambdas will use

- [ ] **Step 1: Add EffectId values to `engine.h`**

In `effects-core/include/ambient_matrix/engine.h`, the current enum ends with `Confetti, Tornado,`. Append:

```cpp
    // GyverLamp v1 originals
    ColorChange,
    RainbowVert,
    RainbowHoriz,
    Madness3D,
    Clouds3D,
    Lava3D,
    Plasma3D,
    Rainbow3D,
    Peacock3D,
    Zebra3D,
    Forest3D,
    Ocean3D,
```

- [ ] **Step 2: Add includes and switch cases to `engine.cpp`**

Add includes at the top of `effects-core/src/engine.cpp` (after the last existing `#include`):

```cpp
#include "ambient_matrix/effect_noise.h"
#include "ambient_matrix/effect_v1_simple.h"
```

Add cases to `set_effect()` switch, after the `case EffectId::Tornado:` line:

```cpp
        case EffectId::ColorChange:  effect_ = std::make_unique<EffectColorChange>(); break;
        case EffectId::RainbowVert:  effect_ = std::make_unique<EffectRainbowVert>(); break;
        case EffectId::RainbowHoriz: effect_ = std::make_unique<EffectRainbowHoriz>(); break;
        case EffectId::Madness3D:  effect_ = std::make_unique<EffectNoise>(NoiseStyle::Madness);   break;
        case EffectId::Clouds3D:   effect_ = std::make_unique<EffectNoise>(NoiseStyle::Clouds);    break;
        case EffectId::Lava3D:     effect_ = std::make_unique<EffectNoise>(NoiseStyle::Lava);      break;
        case EffectId::Plasma3D:   effect_ = std::make_unique<EffectNoise>(NoiseStyle::Plasma);    break;
        case EffectId::Rainbow3D:  effect_ = std::make_unique<EffectNoise>(NoiseStyle::Rainbow3D); break;
        case EffectId::Peacock3D:  effect_ = std::make_unique<EffectNoise>(NoiseStyle::Peacock);   break;
        case EffectId::Zebra3D:    effect_ = std::make_unique<EffectNoise>(NoiseStyle::Zebra);     break;
        case EffectId::Forest3D:   effect_ = std::make_unique<EffectNoise>(NoiseStyle::Forest);    break;
        case EffectId::Ocean3D:    effect_ = std::make_unique<EffectNoise>(NoiseStyle::Ocean);     break;
```

- [ ] **Step 3: Build**

```bash
cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 4: Commit**

```bash
git add effects-core/include/ambient_matrix/engine.h effects-core/src/engine.cpp
git commit -m "feat: register 12 GyverLamp v1 EffectId values in engine"
```

---

### Task 6: Add 12 effects to CMakeLists and ESPHome build

**Files:**
- Modify: `effects-core/CMakeLists.txt` — add new source files
- Modify: `esphome/common/led_matrix.yaml` — add 12 select options + 12 lambdas

- [ ] **Step 1: Add sources to CMakeLists.txt**

In `effects-core/CMakeLists.txt`, find the `add_library(effects-core ...)` block and add:

```cmake
    src/effect_noise.cpp
    src/effect_v1_simple.cpp
```

- [ ] **Step 2: Rebuild effects-core unit tests**

```bash
cmake -S effects-core -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build 2>&1 | tail -10
```
Expected: builds cleanly.

- [ ] **Step 3: Add 12 options to the `lamp_effect` select in `led_matrix.yaml`**

In `esphome/common/led_matrix.yaml`, find the `options:` list under `lamp_effect`. After `"Tornado"` (last GyverLamp2 original) and before `"Rainbow 2"` (first custom), insert:

```yaml
      # GyverLamp v1 originals
      - "Color Change"
      - "Rainbow Vert"
      - "Rainbow Horiz"
      - "Madness 3D"
      - "Clouds 3D"
      - "Lava 3D"
      - "Plasma 3D"
      - "Rainbow 3D"
      - "Peacock 3D"
      - "Zebra 3D"
      - "Forest 3D"
      - "Ocean 3D"
```

- [ ] **Step 4: Add 12 `addressable_lambda` effect blocks**

In the `light:` / `effects:` section of `led_matrix.yaml`, add 12 new lambda blocks after the `"Tornado"` lambda and before the `"Rainbow 2"` lambda. Use the same pattern as existing lambdas:

```yaml
      - addressable_lambda:
          name: "Color Change"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::ColorChange);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Rainbow Vert"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::RainbowVert);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Rainbow Horiz"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::RainbowHoriz);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Madness 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Madness3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Clouds 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Clouds3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Lava 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Lava3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Plasma 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Plasma3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Rainbow 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Rainbow3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Peacock 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Peacock3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Zebra 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Zebra3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Forest 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Forest3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());

      - addressable_lambda:
          name: "Ocean 3D"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Ocean3D);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());
```

> Note: The `engine.set_params({g_effect_speed, g_effect_scale, 0})` line assumes Plan A (per-effect speed/scale) is already merged. If Plan A is not yet done, remove that line — effects will use `EffectParams` defaults (speed=128, scale=128).

- [ ] **Step 5: ESPHome compile**

```bash
make esphome-compile 2>&1 | tail -30
```
Expected: success. Any C++ compile errors will appear here.

- [ ] **Step 6: Flash and test**

```bash
make esphome-ota
```

After flashing:
- In Home Assistant, open `Lamp Effect` select.
- Switch to `Madness 3D` — matrix should show colorful swirling Perlin noise.
- Switch to `Clouds 3D` — blue/white slow drifting clouds.
- Switch to `Lava 3D` — red/orange lava lamp.
- Switch to `Plasma 3D` — bright party colors, colorLoop mode (shifting hue offset).
- Switch to `Forest 3D` — greens, slow speed.
- Switch to `Peacock 3D` — rainbow stripes, rotating.
- Switch to `Zebra 3D` — black/white striped patterns.
- Switch to `Color Change` — single solid color cycling through spectrum.
- Switch to `Rainbow Vert` — vertical columns of rainbow.
- Switch to `Rainbow Horiz` — horizontal rows of rainbow.
- Adjust `Effect Speed` and `Effect Scale` sliders — each noise effect responds visibly.

- [ ] **Step 7: Commit**

```bash
git add effects-core/CMakeLists.txt esphome/common/led_matrix.yaml
git commit -m "feat: expose 12 GyverLamp v1 effects in ESPHome select + lambdas"
```

---

## Default Speed/Scale Values

These are the GyverLamp v1 defaults (from `effects.ino`). They become the NVS-saved values after the user first moves a slider for each effect:

| Effect | Speed | Scale | Notes |
|--------|-------|-------|-------|
| Madness 3D | 20 | 100 | |
| Clouds 3D  | 20 |  30 | slow drift |
| Lava 3D    | 20 |  50 | |
| Plasma 3D  | 30 |  30 | |
| Rainbow 3D | 30 |  30 | |
| Peacock 3D | 20 |  20 | |
| Zebra 3D   | 20 |  30 | |
| Forest 3D  | 15 | 120 | wide bands |
| Ocean 3D   | 15 |  90 | |
| Color Change | 20 | — | scale unused |
| Rainbow Vert | 20 | 128 | |
| Rainbow Horiz| 20 | 128 | |

Since NVS defaults are 128, the effects will start at a midpoint. To preload the v1 defaults, optionally add a one-time migration in `on_boot` that checks if each key is absent and writes the v1 value. This is optional and can be done as a follow-up.

---

## Self-Review

**Spec coverage:**
- ✅ 9 Perlin-noise effects (Madness 3D … Ocean 3D)
- ✅ 3 simple effects (Color Change, Rainbow Vert, Rainbow Horiz)
- ✅ All 12 in `EffectId` enum and `engine.cpp` switch
- ✅ All 12 in `lamp_effect` select options and as `addressable_lambda` blocks
- ✅ 4 missing palettes added
- ✅ `dim8_raw` added
- ✅ `Rgb::from_hsv` required by Madness 3D is verified/added
- ✅ Select ordering: GyverLamp2 → GyverLamp v1 → custom "2" effects

**Placeholder scan:** None found. All code blocks are complete.

**Type consistency:** `NoiseStyle` enum is defined in `effect_noise.h` and used in `engine.cpp`. `EffectNoise(NoiseStyle::*)` constructor matches the header. `EffectId::Madness3D` etc. match exactly between `engine.h` and `engine.cpp`.
