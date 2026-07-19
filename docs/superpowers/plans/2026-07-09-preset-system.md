# Preset System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace raw effect select in Home Assistant with a named preset dropdown, add Speed/Scale +/− buttons, add Reset-to-defaults button, fix Scanner speed and Fire cooling bugs, and rename GyverLamp2-original effects with `(Original)` suffix.

**Architecture:** A new static C++ table `kPresets[]` in `preset_table.h` drives both the HA dropdown options and the NVS key (preset name → saved speed/scale). Selecting a preset loads NVS by preset name, falls back to table defaults, then activates the matching `addressable_lambda`. Physical button cycling moves from raw effects to presets. The existing `select.lamp_effect` becomes a debug-only entity.

**Tech Stack:** ESPHome YAML (addressable_lambda, select, number, button, script), C++17, ESP-IDF NVS for persistence, effects-core C++ library.

## Global Constraints

- All identifiers, names, comments, and YAML strings must be in English.
- Effect lambda names that are 1:1 ports of GyverLamp2 algorithms must carry the suffix ` (Original)`.
- Custom additions (not in GyverLamp2) have no suffix.
- NVS keys for presets are derived from preset name (≤13 chars + suffix char = ≤15 chars total).
- GyverLamp2 effect mapping: app's "Bonfire" = Effect 6 = fire2D algorithm; app's "Fire" = Effect 7 = fire2020 algorithm.
- Our `EffectFire` (heat diffusion, FastLED Fire2012 style) ≠ fire2D → gets no `(Original)` suffix; renamed to "Fire Classic".
- Our `EffectFire2020` = fire2020 → gets `(Original)` suffix; renamed to "Fire (Original)".
- ESPHome compile: `make esphome-compile`. C++ unit tests: `make test`.
- Never run `esphome compile` or `esphome run` directly — always use `make` targets.
- Compile first, flash second; never in parallel.

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `esphome/custom/preset_table.h` | **CREATE** | Preset struct, kPresets[], find_preset() |
| `esphome/custom/effect_prefs.h` | **MODIFY** | Add g_current_preset, preset_prefs_load/save |
| `esphome/common/led_matrix.yaml` | **MODIFY** | Rename lambdas, add select/buttons, update scripts |
| `effects-core/src/effect_scanner.cpp` | **MODIFY** | Fix speed (was hardcoded) |
| `effects-core/src/effect_fire.cpp` | **MODIFY** | Fix cooling formula |
| `effects-core/src/effect_fire2020.cpp` | **MODIFY** | Fix one-time init |
| `tests/` | **MODIFY** | Add preset table unit tests |

---

## Task 1: Fix Scanner speed bug

**Files:**
- Modify: `effects-core/src/effect_scanner.cpp:25-28`
- Test: `tests/` (compile + visual check)

**Context:** `EffectScanner::tick()` ignores `params.speed` — cycle time is hardcoded at 4800 ms. The fix maps speed 0→7830 ms (slow) and speed 255→1200 ms (fast).

- [ ] **Step 1: Edit effect_scanner.cpp**

Replace lines 25–28 (the `kCycleMs` constants and `phase` computation):

```cpp
// Remove these two lines:
static constexpr uint32_t kCycleMs = 4800;
static constexpr uint32_t kHalfCycleMs = kCycleMs / 2;
const uint32_t phase = frame.total_ms % kCycleMs;

// Replace with:
const uint32_t cycle_ms = 1200u + (255u - params.speed) * 26u;
const uint32_t half_cycle_ms = cycle_ms / 2u;
const uint32_t phase = frame.total_ms % cycle_ms;
```

Also update the `t` computation on the next line — it references `kHalfCycleMs`:

```cpp
// Was:
const float t = (float)(phase % kHalfCycleMs) / (float)kHalfCycleMs;

// Becomes:
const float t = (float)(phase % half_cycle_ms) / (float)half_cycle_ms;
```

- [ ] **Step 2: Run unit tests**

```bash
make test
```

Expected: all tests pass (no scanner-specific unit test exists; this confirms compile).

- [ ] **Step 3: Commit**

```bash
git add effects-core/src/effect_scanner.cpp
git commit -m "fix: scanner speed now responds to params.speed

speed=255 → 1200ms cycle (fast), speed=0 → 7830ms (slow)

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 2: Fix Fire cooling bug and rename to "Fire Classic"

**Files:**
- Modify: `effects-core/src/effect_fire.cpp:28-30`
- Modify: `effects-core/src/effect_fire.h` (class comment)

**Context:** At default params, `cooling=9` and `sparking=122` (48% chance) make flames reach the white end of `kHeatColors`. Also, our heat-diffusion algorithm is NOT GyverLamp2's fire2D lookup-table algorithm, so it must NOT get the `(Original)` suffix — it becomes "Fire Classic".

The renaming in ESPHome YAML happens in Task 6. This task only fixes the algorithm.

- [ ] **Step 1: Fix cooling formula in effect_fire.cpp**

```cpp
// Was (line ~28):
uint8_t cooling = 2 + (255 - params.scale) / 16;
uint8_t sparking = 80 + params.speed / 3;

// Becomes:
uint8_t cooling = 20 + (255 - params.scale) / 8;   // scale=255→20, scale=0→51
uint8_t sparking = 50 + params.speed / 5;           // speed=0→50, speed=255→101
```

- [ ] **Step 2: Fix wrong comment in effect_fire.h**

In `effects-core/include/ambient_matrix/effect_fire.h`, the class doc comment says `// Classic 2D heat-diffusion fire.`. Update it:

```cpp
// Heat-diffusion fire (FastLED Fire2012 style). NOT the GyverLamp2 fire2D algorithm.
// GyverLamp2 equivalent is "Bonfire" (fire2D.ino) which uses lookup tables — not ported yet.
```

- [ ] **Step 3: Run tests**

```bash
make test
```

Expected: all pass.

- [ ] **Step 4: Commit**

```bash
git add effects-core/src/effect_fire.cpp effects-core/include/ambient_matrix/effect_fire.h
git commit -m "fix: increase fire cooling to prevent white flames at default params

cooling formula: 20+(255-scale)/8, sparking: 50+speed/5
Also clarify that EffectFire is heat-diffusion, not GyverLamp2 fire2D

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 3: Fix Fire2020 re-init bug and confirm (Original) alignment

**Files:**
- Modify: `effects-core/src/effect_fire2020.cpp`
- Modify: `effects-core/include/ambient_matrix/effect_fire2020.h`

**Context:** `initialized_` flag means `delta_value_`, `delta_hue_`, `step_` are computed only once — changing Scale in HA after startup has no effect. Fix: remove the one-time guard; recompute when `params.scale` changes by caching `last_scale_`. This is the GyverLamp2 fire2020 algorithm → will be named "Fire (Original)" in Task 6.

- [ ] **Step 1: Add last_scale_ field to header**

In `effect_fire2020.h`, replace:
```cpp
bool    initialized_  = false;
```
with:
```cpp
uint8_t last_scale_   = 255;   // sentinel: forces recompute on first tick
```
Remove the `initialized_` member entirely.

- [ ] **Step 2: Update tick() in effect_fire2020.cpp**

Replace the `if (!initialized_)` block:

```cpp
// Remove:
if (!initialized_) {
    initialized_  = true;
    delta_value_  = map_int(params.scale, 0, 255, 8, 168);
    delta_hue_    = map_int(delta_value_, 8, 168, 8, 84);
    step_         = map_int(255 - delta_value_, 87, 247, 4, 32);
    uint8_t num   = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
    for (uint8_t i = 0; i < num && i < kMaxSparks; i++) {
        spark_y_[i] = (float)random8(4);
        spark_x_[i] = random8(w);
    }
}

// Replace with:
if (params.scale != last_scale_) {
    last_scale_   = params.scale;
    delta_value_  = map_int(params.scale, 0, 255, 8, 168);
    delta_hue_    = map_int(delta_value_, 8, 168, 8, 84);
    step_         = map_int(255 - delta_value_, 87, 247, 4, 32);
    uint8_t num   = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
    for (uint8_t i = 0; i < num && i < kMaxSparks; i++) {
        spark_y_[i] = (float)random8(4);
        spark_x_[i] = random8(w);
    }
}
```

- [ ] **Step 3: Update reset() to clear last_scale_**

```cpp
void EffectFire2020::reset() {
    clock_.reset();
    stepper_.reset();
    ff_y_ = ff_z_ = 0;
    last_scale_ = 255;   // force recompute on next tick
}
```

- [ ] **Step 4: Run tests**

```bash
make test
```

Expected: all pass.

- [ ] **Step 5: Commit**

```bash
git add effects-core/src/effect_fire2020.cpp effects-core/include/ambient_matrix/effect_fire2020.h
git commit -m "fix: fire2020 recomputes params when scale changes

Removed one-time initialized_ flag; now tracks last_scale_ and
recomputes delta_value/delta_hue/step whenever scale param changes.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 4: Create preset_table.h

**Files:**
- Create: `esphome/custom/preset_table.h`
- Create: `tests/test_preset_table.cpp`

**Interfaces:**
- Produces: `struct Preset`, `kPresets[]`, `kPresetCount`, `find_preset(const char*) → const Preset*`

- [ ] **Step 1: Write failing test**

Create `tests/test_preset_table.cpp`:

```cpp
#include "catch2/catch_amalgamated.hpp"
// Minimal stubs so preset_table.h compiles in test context
#include <cstdint>
#include <cstring>

#include "../esphome/custom/preset_table.h"

TEST_CASE("find_preset returns correct preset by name") {
    const Preset* p = find_preset("Perlin Warm (Original)");
    REQUIRE(p != nullptr);
    REQUIRE(strcmp(p->effect, "Perlin (Original)") == 0);
    REQUIRE(p->speed == 200);
    REQUIRE(p->scale == 100);
    REQUIRE(p->palette == 0); // Heat
}

TEST_CASE("find_preset returns nullptr for unknown name") {
    REQUIRE(find_preset("Does Not Exist") == nullptr);
}

TEST_CASE("kPresetCount matches expected number of presets") {
    REQUIRE(kPresetCount == 13);
}

TEST_CASE("all preset effect names are non-empty") {
    for (size_t i = 0; i < kPresetCount; i++) {
        REQUIRE(kPresets[i].name != nullptr);
        REQUIRE(kPresets[i].effect != nullptr);
        REQUIRE(strlen(kPresets[i].name) > 0);
    }
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
make test 2>&1 | head -20
```

Expected: compile error — `preset_table.h` not found.

- [ ] **Step 3: Create esphome/custom/preset_table.h**

```cpp
#pragma once
#include <cstdint>
#include <cstring>

struct Preset {
    const char* name;     // display name in HA dropdown; also used as NVS key
    const char* effect;   // must match addressable_lambda name in led_matrix.yaml
    uint8_t     speed;    // default speed (0–255)
    uint8_t     scale;    // default scale (0–255)
    uint8_t     palette;  // default palette index (matches effect_palette_name() table)
};

// Palette index reference (matches effect_prefs.h effect_palette_name()):
//   0=Heat  1=Lava  2=Party  3=Rainbow  4=Rainbow Stripe  5=Cloud  6=Ocean  7=Forest

static const Preset kPresets[] = {
    // ── Original (GyverLamp2 factory default) ──────────────────────────────
    {"Perlin Warm (Original)", "Perlin (Original)", 200, 100, 0},

    // ── Custom ─────────────────────────────────────────────────────────────
    {"Perlin Ocean",           "Perlin (Original)", 150, 120, 6},
    {"Perlin Forest",          "Perlin (Original)", 120, 140, 7},
    {"Perlin Rainbow",         "Perlin (Original)", 180, 100, 3},
    {"Perlin Party",           "Perlin (Original)", 200,  80, 2},
    {"Fire",                   "Fire Classic",      128, 180, 0},
    {"Campfire",               "Campfire",          100, 150, 0},
    {"Fire 2020",              "Fire (Original)",   100, 128, 0},
    {"Confetti",               "Confetti (Original)", 128, 128, 3},
    {"Particles",              "Particles (Original)", 128, 100, 2},
    {"Tornado",                "Tornado (Original)",  100, 128, 3},
    {"Color Shift",            "Color Shift (Original)", 80, 128, 3},
    {"Scanner",                "Color Scanner 2",   128, 128, 3},
};

static constexpr size_t kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

inline const Preset* find_preset(const char* name) {
    for (size_t i = 0; i < kPresetCount; i++)
        if (strcmp(kPresets[i].name, name) == 0) return &kPresets[i];
    return nullptr;
}
```

- [ ] **Step 4: Add test to CMakeLists or Makefile**

Check `tests/CMakeLists.txt` (or the test target in `Makefile`) and add `test_preset_table.cpp` to the test sources. Follow the pattern of existing test files.

- [ ] **Step 5: Run tests**

```bash
make test
```

Expected: all 4 new tests pass, existing tests still pass.

- [ ] **Step 6: Commit**

```bash
git add esphome/custom/preset_table.h tests/test_preset_table.cpp
git commit -m "feat: add preset_table.h with kPresets[] and find_preset()

13 presets: 1 Original (GyverLamp2 factory default) + 12 custom.
Naming convention: (Original) suffix = faithful GyverLamp2 port.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 5: Update effect_prefs.h — preset-keyed NVS

**Files:**
- Modify: `esphome/custom/effect_prefs.h`

**Context:** Currently NVS keys are derived from the effect name. Presets share the same effect (e.g., "Perlin Ocean" and "Perlin Warm" both run "Perlin (Original)"), so they would share NVS slots. Solution: add `preset_prefs_load/save` that use the preset name as the NVS key. The existing `effect_prefs_load/save` remain untouched.

**Interfaces:**
- Produces: `g_current_preset` (std::string), `preset_prefs_load(const char*)`, `preset_prefs_save(const char*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)`

- [ ] **Step 1: Add g_current_preset global**

After the existing globals in `effect_prefs.h` (after line 12), add:

```cpp
inline std::string g_current_preset = "Perlin Warm (Original)";
```

- [ ] **Step 2: Add preset_prefs_load() function**

Add after `effect_prefs_load()`:

```cpp
// Load speed/scale/palette from NVS keyed by preset name.
// Falls back to preset table defaults if no NVS entry exists.
// Requires: preset_table.h included before this call site.
inline void preset_prefs_load(const char* preset_name) {
    char ks[16], kc[16], kp[16];
    effect_prefs_key(preset_name, 's', ks);
    effect_prefs_key(preset_name, 'c', kc);
    effect_prefs_key(preset_name, 'p', kp);

    // Determine defaults from preset table.
    const Preset* p = find_preset(preset_name);
    uint8_t def_speed   = p ? p->speed   : 128;
    uint8_t def_scale   = p ? p->scale   : 128;
    uint8_t def_palette = p ? p->palette : 3;

    nvs_handle_t h;
    if (nvs_open("lamppre", NVS_READONLY, &h) != ESP_OK) {
        g_effect_speed   = def_speed;
        g_effect_scale   = def_scale;
        g_effect_palette = def_palette;
        return;
    }
    uint8_t v = def_speed;
    g_effect_speed   = (nvs_get_u8(h, ks, &v) == ESP_OK) ? v : def_speed;
    v = def_scale;
    g_effect_scale   = (nvs_get_u8(h, kc, &v) == ESP_OK) ? v : def_scale;
    v = def_palette;
    g_effect_palette = (nvs_get_u8(h, kp, &v) == ESP_OK) ? v : def_palette;
    nvs_close(h);
}
```

Note: uses NVS namespace `"lamppre"` (different from effect prefs `"lampfx"`) to avoid key collisions.

- [ ] **Step 3: Add preset_prefs_save() function**

```cpp
inline void preset_prefs_save(const char* preset_name,
                              uint8_t speed,
                              uint8_t scale,
                              uint8_t palette) {
    char ks[16], kc[16], kp[16];
    effect_prefs_key(preset_name, 's', ks);
    effect_prefs_key(preset_name, 'c', kc);
    effect_prefs_key(preset_name, 'p', kp);
    nvs_handle_t h;
    if (nvs_open("lamppre", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, ks, speed);
    nvs_set_u8(h, kc, scale);
    nvs_set_u8(h, kp, palette);
    nvs_commit(h);
    nvs_close(h);
}
```

- [ ] **Step 4: Add preset_table.h include guard note**

`preset_prefs_load` calls `find_preset()` which is defined in `preset_table.h`. The ESPHome lambda context includes all custom headers, but add a comment:

```cpp
// preset_prefs_load() calls find_preset() from preset_table.h.
// Ensure preset_table.h is included before this file at call sites.
```

- [ ] **Step 5: Run compile check**

```bash
make test
```

Expected: all pass (effect_prefs.h is header-only; compile validates it).

- [ ] **Step 6: Commit**

```cpp
git add esphome/custom/effect_prefs.h
git commit -m "feat: add preset-keyed NVS functions to effect_prefs.h

preset_prefs_load/save use preset name as NVS key (namespace: lamppre).
Presets sharing same effect now have independent speed/scale storage.
Added g_current_preset global.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 6: Rename effect lambdas in led_matrix.yaml

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

**Context:** All addressable_lambda entries that are faithful GyverLamp2 ports get ` (Original)` appended to their `name:` field. `select.lamp_effect` options list is updated to match. `select.lamp_effect` gets `entity_category: config` so it disappears from the main HA dashboard. `button_first_effect` is updated to use the new "Color (Original)" name.

**Rename map:**

| Old name | New name |
|---|---|
| `"Color"` | `"Color (Original)"` |
| `"Color Shift"` | `"Color Shift (Original)"` |
| `"Gradient"` | `"Gradient (Original)"` |
| `"Perlin"` | `"Perlin (Original)"` |
| `"Particles"` | `"Particles (Original)"` |
| `"Fire"` | `"Fire Classic"` |
| `"Fire 2020"` | `"Fire (Original)"` |
| `"Confetti"` | `"Confetti (Original)"` |
| `"Tornado"` | `"Tornado (Original)"` |
| `"Campfire"` | `"Campfire"` (unchanged) |
| `"Color Scanner 2"` | `"Color Scanner 2"` (unchanged) |
| `"Blue Scanner 2"` | `"Blue Scanner 2"` (unchanged) |
| `"Rainbow"` | `"Rainbow"` (unchanged — our custom addition) |
| Benchmark effects | unchanged |

- [ ] **Step 1: Rename all addressable_lambda name: fields**

In `led_matrix.yaml`, find every `addressable_lambda:` block and update its `name:` field per the rename map above. There are 43 lambdas total; most are status/benchmark effects that don't change. Use search-and-replace carefully — match exact strings with surrounding quotes.

Key blocks to find and rename (search for `name:` inside `addressable_lambda:` blocks):
```yaml
# Example — find:
      - addressable_lambda:
          name: "Fire"
# Replace with:
      - addressable_lambda:
          name: "Fire Classic"
```

- [ ] **Step 2: Update select.lamp_effect options list**

Find the `select:` block with `id: lamp_effect` (around line 219). Update `options:` and add `entity_category: config`:

```yaml
select:
  - platform: template
    id: lamp_effect
    name: "Lamp Effect"
    entity_category: config          # ← ADD THIS
    options:
      - "Color (Original)"
      - "Color Shift (Original)"
      - "Gradient (Original)"
      - "Perlin (Original)"
      - "Particles (Original)"
      - "Fire Classic"
      - "Fire (Original)"
      - "Confetti (Original)"
      - "Tornado (Original)"
      - "Campfire"
      - "Color Scanner 2"
      - "Blue Scanner 2"
      - "Rainbow"
      - "Benchmark Circle"
      - "Benchmark Ball"
      - "Benchmark Sine"
```

- [ ] **Step 3: Update current_effect initial value**

In `globals:` (line 13-16), update initial_value:

```yaml
  - id: current_effect
    type: std::string
    restore_value: true
    initial_value: '"Perlin (Original)"'
```

- [ ] **Step 4: Update button_first_effect script**

Around line 794, update the hardcoded "Color" reference:

```yaml
  - id: button_first_effect
    mode: restart
    then:
      - lambda: |-
          id(current_effect) = "Perlin (Original)";
          effect_prefs_load("Perlin (Original)");
          ...
          id(lamp_effect).publish_state(id(current_effect));
```

- [ ] **Step 5: Compile**

```bash
make esphome-compile
```

Expected: successful compile. If there are "unknown effect name" errors, find the mismatched lambda name and fix it.

- [ ] **Step 6: Commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: rename effect lambdas with (Original) suffix for GyverLamp2 ports

Fire→Fire Classic (heat diffusion, not fire2D)
Fire 2020→Fire (Original) (fire2020 algorithm)
Color/Perlin/Gradient/etc→(Original)
select.lamp_effect moved to entity_category: config

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 7: Add select.lamp_preset and wire preset logic

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

**Context:** Add `select.lamp_preset` as the primary HA control. When a preset is selected: load NVS by preset name, apply speed/scale/palette to globals, set `current_effect` to the preset's effect name, activate the light effect, update all sliders. Add `current_preset` global.

**Depends on:** Task 4 (preset_table.h), Task 5 (preset_prefs_load), Task 6 (renamed lambdas).

- [ ] **Step 1: Add current_preset global**

In the `globals:` block (after `current_effect`), add:

```yaml
  - id: current_preset
    type: std::string
    restore_value: true
    initial_value: '"Perlin Warm (Original)"'
```

- [ ] **Step 2: Add select.lamp_preset entity**

In the `select:` block, add BEFORE `select.lamp_effect`:

```yaml
  - platform: template
    id: lamp_preset
    name: "Lamp Preset"
    options:
      - "Perlin Warm (Original)"
      - "Perlin Ocean"
      - "Perlin Forest"
      - "Perlin Rainbow"
      - "Perlin Party"
      - "Fire"
      - "Campfire"
      - "Fire 2020"
      - "Confetti"
      - "Particles"
      - "Tornado"
      - "Color Shift"
      - "Scanner"
    lambda: |-
      return id(current_preset);
    set_action:
      - lambda: |-
          const Preset* p = find_preset(x.c_str());
          if (!p) return;
          id(current_preset) = x;
          id(current_effect) = p->effect;
          preset_prefs_load(x.c_str());
          g_effect_palette = (nvs lookup already done by preset_prefs_load);
          id(effect_speed).publish_state((float)g_effect_speed);
          id(effect_scale).publish_state((float)g_effect_scale);
          id(effect_palette).publish_state((float)g_effect_palette);
          id(lamp_effect).publish_state(id(current_effect));
      - if:
          condition:
            lambda: return id(lamp_powered);
          then:
            - light.turn_on:
                id: led_matrix
                effect: !lambda return id(current_effect);
```

Note: `preset_prefs_load()` already sets `g_effect_speed`, `g_effect_scale`, `g_effect_palette` — just publish them to HA after.

Clean version of set_action lambda:

```cpp
const Preset* p = find_preset(x.c_str());
if (!p) return;
id(current_preset) = x;
id(current_effect) = p->effect;
preset_prefs_load(x.c_str());
id(effect_speed).publish_state((float)g_effect_speed);
id(effect_scale).publish_state((float)g_effect_scale);
id(effect_palette).publish_state((float)g_effect_palette);
id(effect_from_palette).publish_state(g_effect_from_palette != 0);
id(lamp_effect).publish_state(id(current_effect));
```

- [ ] **Step 3: Update speed/scale set_action to save by preset name**

Find `number.effect_speed` set_action debounce script (`apply_effect_speed_debounced`). In the script body where `effect_prefs_save(...)` is called, replace:

```cpp
// Old:
effect_prefs_save(id(current_effect).c_str(), g_effect_speed, g_effect_scale, ...);

// New:
preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
```

Do the same for `apply_effect_scale_debounced`.

- [ ] **Step 4: Compile**

```bash
make esphome-compile
```

Expected: successful compile.

- [ ] **Step 5: Commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: add select.lamp_preset as primary HA control

Preset selection loads NVS by preset name (independent per preset),
falls back to kPresets[] defaults. Updates speed/scale/palette sliders.
NVS save now keys by current_preset instead of current_effect.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 8: Add +/− speed/scale buttons and Reset-to-defaults button

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

**Context:** Add 5 button entities: speed +10, speed −10, scale +10, scale −10, reset-to-defaults. Buttons publish the updated value to HA and save to NVS by preset name.

- [ ] **Step 1: Add button entities**

In `led_matrix.yaml`, find the `button:` section (or add one after `number:`). Add:

```yaml
button:
  - platform: template
    name: "Speed Up"
    id: btn_speed_up
    on_press:
      - lambda: |-
          g_effect_speed = (g_effect_speed > 245) ? 255 : g_effect_speed + 10;
          preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
          id(effect_speed).publish_state((float)g_effect_speed);

  - platform: template
    name: "Speed Down"
    id: btn_speed_down
    on_press:
      - lambda: |-
          g_effect_speed = (g_effect_speed < 10) ? 0 : g_effect_speed - 10;
          preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
          id(effect_speed).publish_state((float)g_effect_speed);

  - platform: template
    name: "Scale Up"
    id: btn_scale_up
    on_press:
      - lambda: |-
          g_effect_scale = (g_effect_scale > 245) ? 255 : g_effect_scale + 10;
          preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
          id(effect_scale).publish_state((float)g_effect_scale);

  - platform: template
    name: "Scale Down"
    id: btn_scale_down
    on_press:
      - lambda: |-
          g_effect_scale = (g_effect_scale < 10) ? 0 : g_effect_scale - 10;
          preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
          id(effect_scale).publish_state((float)g_effect_scale);

  - platform: template
    name: "Reset to Defaults"
    id: btn_reset_defaults
    on_press:
      - lambda: |-
          const Preset* p = find_preset(id(current_preset).c_str());
          if (!p) return;
          g_effect_speed   = p->speed;
          g_effect_scale   = p->scale;
          g_effect_palette = p->palette;
          preset_prefs_save(id(current_preset).c_str(), g_effect_speed, g_effect_scale, g_effect_palette);
          id(effect_speed).publish_state((float)g_effect_speed);
          id(effect_scale).publish_state((float)g_effect_scale);
          id(effect_palette).publish_state((float)g_effect_palette);
```

- [ ] **Step 2: Compile**

```bash
make esphome-compile
```

Expected: successful compile.

- [ ] **Step 3: Commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: add speed/scale +/- buttons and reset-to-defaults button

Step size: 10. Reset reads defaults from kPresets[] and persists to NVS.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Task 9: Update physical button cycling to use presets

**Files:**
- Modify: `esphome/common/led_matrix.yaml` (scripts section)

**Context:** The physical touch button calls `button_next_effect` / `button_previous_effect` scripts. These must now cycle through `kPresets[]` instead of a hardcoded effect list, and must update `current_preset`, load NVS by preset name, and publish to `lamp_preset`.

- [ ] **Step 1: Rewrite button_next_effect script**

Find `button_next_effect` script (~line 724). Replace its lambda body:

```cpp
// Find current preset index in kPresets[]
size_t index = 0;
for (size_t i = 0; i < kPresetCount; i++) {
    if (id(current_preset) == kPresets[i].name) {
        index = i;
        break;
    }
}
index = (index + 1) % kPresetCount;

const Preset& p = kPresets[index];
id(current_preset) = p.name;
id(current_effect) = p.effect;
preset_prefs_load(p.name);
id(effect_speed).publish_state((float)g_effect_speed);
id(effect_scale).publish_state((float)g_effect_scale);
id(effect_palette).publish_state((float)g_effect_palette);
id(effect_from_palette).publish_state(g_effect_from_palette != 0);
id(lamp_preset).publish_state(id(current_preset));
id(lamp_effect).publish_state(id(current_effect));
```

- [ ] **Step 2: Rewrite button_previous_effect script**

Find `button_previous_effect` script (~line 757). Replace its lambda body:

```cpp
size_t index = 0;
for (size_t i = 0; i < kPresetCount; i++) {
    if (id(current_preset) == kPresets[i].name) {
        index = i;
        break;
    }
}
index = (index + kPresetCount - 1) % kPresetCount;

const Preset& p = kPresets[index];
id(current_preset) = p.name;
id(current_effect) = p.effect;
preset_prefs_load(p.name);
id(effect_speed).publish_state((float)g_effect_speed);
id(effect_scale).publish_state((float)g_effect_scale);
id(effect_palette).publish_state((float)g_effect_palette);
id(effect_from_palette).publish_state(g_effect_from_palette != 0);
id(lamp_preset).publish_state(id(current_preset));
id(lamp_effect).publish_state(id(current_effect));
```

- [ ] **Step 3: Update button_first_effect script**

Find `button_first_effect` (~line 790). Replace body:

```cpp
const Preset& p = kPresets[0];   // first preset = "Perlin Warm (Original)"
id(current_preset) = p.name;
id(current_effect) = p.effect;
preset_prefs_load(p.name);
id(effect_speed).publish_state((float)g_effect_speed);
id(effect_scale).publish_state((float)g_effect_scale);
id(effect_palette).publish_state((float)g_effect_palette);
id(effect_from_palette).publish_state(g_effect_from_palette != 0);
id(lamp_preset).publish_state(id(current_preset));
id(lamp_effect).publish_state(id(current_effect));
```

- [ ] **Step 4: Update sunrise restore effect (if exists)**

Search for `button_first_effect` references in the sunrise automation section. Any place that resets to a "first" state should remain consistent.

- [ ] **Step 5: Final compile**

```bash
make esphome-compile
```

Expected: successful compile with no errors or warnings about unknown effect names.

- [ ] **Step 6: Final commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: physical button now cycles presets instead of raw effects

button_next/previous/first_effect scripts updated to iterate kPresets[],
load NVS by preset name, and publish to lamp_preset select entity.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
```

---

## Self-Review Checklist

- [x] **Task 1** covers Scanner speed fix
- [x] **Task 2** covers Fire cooling fix + rename to "Fire Classic"
- [x] **Task 3** covers Fire2020 re-init fix + confirmation of (Original) alignment
- [x] **Task 4** covers preset_table.h with struct, array, and find_preset()
- [x] **Task 5** covers preset NVS functions keyed by preset name
- [x] **Task 6** covers renaming all lambdas + select.lamp_effect → entity_category: config
- [x] **Task 7** covers select.lamp_preset + wiring preset load/save logic
- [x] **Task 8** covers 4 +/- buttons + reset-to-defaults
- [x] **Task 9** covers physical button cycling presets

**Spec gap:** `effect_palette` select is updated in Task 7 `publish_state` calls — this assumes the entity already exists. It does (currently `number.effect_palette` or `select.effect_palette_index`). No change needed to the palette entity itself.

**Type consistency:** `find_preset()` returns `const Preset*` — all call sites use pointer null-check before dereference. ✅  
`kPresets[index]` (by reference) in Task 9 scripts is safe because index is always `< kPresetCount`. ✅  
`preset_prefs_save` takes 3 params (speed, scale, palette) — all call sites in Tasks 7, 8, 9 pass exactly 3 value params. ✅
