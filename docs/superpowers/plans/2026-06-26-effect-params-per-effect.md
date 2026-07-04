# Per-Effect Speed & Scale Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Expose two HA number sliders (Effect Speed 0–255, Effect Scale 0–255) that tune the active effect, persist per-effect values in NVS, and restore them when an effect is selected.

**Architecture:** A new header `esphome/custom/effect_prefs.h` holds two inline globals (`g_effect_speed`, `g_effect_scale`) plus `effect_prefs_load(name)` / `effect_prefs_save(name, speed, scale)` that read/write Arduino `Preferences` (NVS). Every effect lambda calls `engine.set_params({g_effect_speed, g_effect_scale})` on each tick. When the user changes effects or moves a slider, prefs are immediately persisted.

**Tech Stack:** Arduino `Preferences` (ESP32 NVS), ESPHome `template number`, inline globals in ESPHome custom C++.

## Global Constraints

- All identifiers and comments in English.
- Build with `make esphome-compile` before flashing with `make esphome-ota`. Never run both in parallel.
- Default speed = 128, default scale = 128 (matches GyverLamp2 midpoint).
- NVS namespace: `"lamp"`. Key format: first 13 chars of effect name lowercased, spaces→`_`, then `_s` (speed) or `_c` (scale). Max key length 15 chars (Preferences limit).

---

## File Map

| Action  | Path |
|---------|------|
| Create  | `esphome/custom/effect_prefs.h` |
| Modify  | `esphome/common/led_matrix.yaml` — add 2 globals, 2 number entities, update set_action, patch all 37 effect lambdas |
| Modify  | `esphome/ambient_matrix_esp32.yaml` — add include + prefs load on boot |

---

### Task 1: Create `effect_prefs.h`

**Files:**
- Create: `esphome/custom/effect_prefs.h`

**Interfaces:**
- Produces: `uint8_t g_effect_speed`, `uint8_t g_effect_scale` (inline globals)
- Produces: `void effect_prefs_load(const char* name)`, `void effect_prefs_save(const char* name, uint8_t speed, uint8_t scale)`

- [ ] **Step 1: Create the file**

```cpp
// esphome/custom/effect_prefs.h
// Per-effect speed and scale persistence via Arduino Preferences (NVS).
#pragma once
#include <Preferences.h>
#include <cstdint>
#include <cstring>
#include <cctype>

inline uint8_t g_effect_speed = 128;
inline uint8_t g_effect_scale = 128;

// Derive a stable NVS key from an effect name.
// Rule: first 13 chars, lowercase, spaces→'_', then '_s' or '_c'.
// Max 15 chars total (Preferences limit).
inline void effect_prefs_key(const char* name, char suffix, char* out) {
    int i = 0;
    for (; name[i] && i < 13; i++) {
        char c = name[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
        else if (c == ' ')        c = '_';
        out[i] = c;
    }
    out[i++] = '_';
    out[i++] = suffix;
    out[i]   = '\0';
}

inline void effect_prefs_load(const char* name) {
    char ks[16], kc[16];
    effect_prefs_key(name, 's', ks);
    effect_prefs_key(name, 'c', kc);
    Preferences p;
    p.begin("lamp", /*readOnly=*/true);
    g_effect_speed = (uint8_t)p.getUInt(ks, 128);
    g_effect_scale = (uint8_t)p.getUInt(kc, 128);
    p.end();
}

inline void effect_prefs_save(const char* name, uint8_t speed, uint8_t scale) {
    char ks[16], kc[16];
    effect_prefs_key(name, 's', ks);
    effect_prefs_key(name, 'c', kc);
    Preferences p;
    p.begin("lamp", /*readOnly=*/false);
    p.putUInt(ks, speed);
    p.putUInt(kc, scale);
    p.end();
}
```

- [ ] **Step 2: Verify it compiles standalone**

```bash
cd /Users/Files/www/pet/esp32-ambient-matrix-lamp
make esphome-compile 2>&1 | grep -E "error:|warning:|Compiling|ld "
```
Expected: compiles without errors (the file isn't included yet, so this is a baseline check).

- [ ] **Step 3: Add include to `ambient_matrix_esp32.yaml`**

In `esphome/ambient_matrix_esp32.yaml`, under the `esphome.includes` list, add:
```yaml
      - custom/effect_prefs.h
```
So the includes block becomes:
```yaml
esphome:
  includes:
    - custom/adapter.h
    - custom/power_transition.h
    - custom/sunrise_renderer.h
    - custom/effect_prefs.h
```

- [ ] **Step 4: Commit**

```bash
git add esphome/custom/effect_prefs.h esphome/ambient_matrix_esp32.yaml
git commit -m "feat: add effect_prefs.h for per-effect NVS speed/scale storage"
```

---

### Task 2: Add globals and number entities to `led_matrix.yaml`

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

**Interfaces:**
- Consumes: `uint8_t g_effect_speed`, `uint8_t g_effect_scale` from `effect_prefs.h`
- Produces: ESPHome entities `effect_speed` (number 0–255), `effect_scale` (number 0–255); HA entity names `"Effect Speed"`, `"Effect Scale"`

- [ ] **Step 1: Add two globals at the top of the `globals:` block**

In `esphome/common/led_matrix.yaml`, append these two entries to the existing `globals:` list (after `sunrise_max_level`):

```yaml
  - id: g_effect_speed_sync
    type: uint8_t
    restore_value: false
    initial_value: '128'
  - id: g_effect_scale_sync
    type: uint8_t
    restore_value: false
    initial_value: '128'
```

> Note: `g_effect_speed` and `g_effect_scale` are C++ inline globals from `effect_prefs.h` — they live outside the ESPHome globals system. These `_sync` globals are not used in logic; they are only present to ensure the value is visible in the ESPHome YAML context if needed. **Actually, do NOT add these** — the ESPHome lambdas can read `g_effect_speed` and `g_effect_scale` directly since `effect_prefs.h` is included. Skip this step.

- [ ] **Step 2: Add `Effect Speed` number entity**

In `esphome/common/led_matrix.yaml`, under `number:`, add after the existing `lamp_sunrise_max` entity:

```yaml
  - platform: template
    id: effect_speed
    name: "Effect Speed"
    min_value: 0
    max_value: 255
    step: 1
    lambda: |-
      return (float)g_effect_speed;
    set_action:
      - lambda: |-
          g_effect_speed = (uint8_t)x;
          effect_prefs_save(id(current_effect).c_str(), g_effect_speed, g_effect_scale);
```

- [ ] **Step 3: Add `Effect Scale` number entity**

In `esphome/common/led_matrix.yaml`, under `number:`, add after the `Effect Speed` entity:

```yaml
  - platform: template
    id: effect_scale
    name: "Effect Scale"
    min_value: 0
    max_value: 255
    step: 1
    lambda: |-
      return (float)g_effect_scale;
    set_action:
      - lambda: |-
          g_effect_scale = (uint8_t)x;
          effect_prefs_save(id(current_effect).c_str(), g_effect_speed, g_effect_scale);
```

- [ ] **Step 4: Compile to verify no syntax errors**

```bash
make esphome-compile 2>&1 | tail -20
```
Expected: compiles successfully.

- [ ] **Step 5: Commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: add Effect Speed and Effect Scale HA number sliders"
```

---

### Task 3: Load prefs on effect change and on boot

**Files:**
- Modify: `esphome/common/led_matrix.yaml` — update `lamp_effect` set_action
- Modify: `esphome/ambient_matrix_esp32.yaml` — load prefs on boot

**Interfaces:**
- Consumes: `effect_prefs_load(const char*)`, `g_effect_speed`, `g_effect_scale`
- Consumes: ESPHome entities `effect_speed`, `effect_scale` (publish_state)

- [ ] **Step 1: Update `lamp_effect` set_action**

In `esphome/common/led_matrix.yaml`, find the `lamp_effect` select entity. Its current `set_action` is:
```yaml
    set_action:
      - lambda: |-
          id(sunrise_active) = false;
          id(power_transition_active) = false;
          id(current_effect) = x;
          id(lamp_powered) = true;
      - light.turn_on:
          id: led_matrix
          effect: !lambda return x;
```

Replace with:
```yaml
    set_action:
      - lambda: |-
          id(sunrise_active) = false;
          id(power_transition_active) = false;
          id(current_effect) = x;
          id(lamp_powered) = true;
          effect_prefs_load(x.c_str());
          id(effect_speed).publish_state((float)g_effect_speed);
          id(effect_scale).publish_state((float)g_effect_scale);
      - light.turn_on:
          id: led_matrix
          effect: !lambda return x;
```

- [ ] **Step 2: Load prefs on boot in `ambient_matrix_esp32.yaml`**

Find the current `on_boot` in `esphome/ambient_matrix_esp32.yaml`:
```yaml
  on_boot:
    priority: -100
    then:
      - script.execute: run_power_on_transition
```

Replace with:
```yaml
  on_boot:
    priority: -100
    then:
      - lambda: |-
          effect_prefs_load(id(current_effect).c_str());
          id(effect_speed).publish_state((float)g_effect_speed);
          id(effect_scale).publish_state((float)g_effect_scale);
      - script.execute: run_power_on_transition
```

- [ ] **Step 3: Compile**

```bash
make esphome-compile 2>&1 | tail -20
```
Expected: success.

- [ ] **Step 4: Commit**

```bash
git add esphome/common/led_matrix.yaml esphome/ambient_matrix_esp32.yaml
git commit -m "feat: load per-effect prefs on effect change and on boot"
```

---

### Task 4: Wire `set_params` into every effect lambda

**Files:**
- Modify: `esphome/common/led_matrix.yaml` — patch all 37 addressable_lambda effects

Each effect lambda currently looks like:
```yaml
      - addressable_lambda:
          name: "Color"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Color);
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());
```

Add `engine.set_params({g_effect_speed, g_effect_scale, 0});` after `initial_run` check:

```yaml
      - addressable_lambda:
          name: "Color"
          update_interval: 30ms
          lambda: |-
            if (!id(lamp_powered)) { ESPHomeMatrixCanvas(it).clear(); return; }
            static ambient_matrix::EffectEngine engine{ambient_matrix::Matrix(16, 16, ${matrix_flip_x}, ${matrix_flip_y})};
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::Color);
            engine.set_params({g_effect_speed, g_effect_scale, 0});
            ESPHomeMatrixCanvas canvas(it);
            engine.tick(canvas, millis());
```

Apply this pattern to **all 37 effect lambdas** (all `addressable_lambda` blocks that use `EffectEngine`). The Sunrise and Power Helix lambdas do NOT use `EffectEngine` — skip them.

Effects to patch (by name in the YAML):
`"Rainbow 2"`, `"Color"`, `"Color Shift"`, `"Gradient"`, `"Perlin"`, `"Particles"`, `"Fire"`, `"Fire 2020"`, `"Confetti"`, `"Tornado"`, `"Color Scanner 2"`, `"Blue Scanner 2"`, `"Plasma 2"`, `"Aurora 2"`, `"Neon Rings 2"`, `"Kaleidoscope 2"`, `"Digital Rain 2"`, `"Comet Shower 2"`, `"Starfield 2"`, `"Cosmic Nebula 2"`, `"Wormhole 2"`, `"Orbital Dance 2"`, `"Solar Storm 2"`, `"Pulsar 2"`, `"Starry Sky 2"`, `"Asteroid Impact 2"`, `"Pac-Man Orbit 2"`, `"Space Police 2"`, `"Purple Meteor Shower 2"`, `"Light Speed 2"`, `"Supernova 2"`, `"Flight to Mars 2"`, `"Singularity 2"`, `"Relic Radiation 2"`, `"Ocean Waves 2"`, `"Lava Lamp 2"`, `"Campfire 2"`.

- [ ] **Step 1: Add `set_params` to every EffectEngine lambda (37 total)**

Use a targeted find-and-replace in the editor. The pattern to find in every matching lambda body:
```
            if (initial_run) engine.set_effect(ambient_matrix::EffectId::
```
Insert on the next line (after the `set_effect` call closes):
```
            engine.set_params({g_effect_speed, g_effect_scale, 0});
```

Verify count:
```bash
grep -c "engine.set_params" /Users/Files/www/pet/esp32-ambient-matrix-lamp/esphome/common/led_matrix.yaml
```
Expected: `37`

- [ ] **Step 2: Compile**

```bash
make esphome-compile 2>&1 | tail -20
```
Expected: success.

- [ ] **Step 3: Flash and verify**

```bash
make esphome-ota
```

After flashing:
- Open Home Assistant → Devices → `ambient-matrix-lamp`
- Confirm `Effect Speed` and `Effect Scale` sliders appear (value = 128)
- Switch to `Fire` effect, move `Effect Speed` slider to 200 → flame should speed up
- Move `Effect Scale` to 50 → flame columns narrower
- Reboot the lamp → sliders should restore to saved values for `Fire`
- Switch to `Confetti` → sliders should jump to 128 (default, first time)
- Move `Effect Scale` to 220 → more sparks spawn
- Reboot → switch to `Confetti` → scale should restore to 220

- [ ] **Step 4: Commit**

```bash
git add esphome/common/led_matrix.yaml
git commit -m "feat: wire effect speed/scale params into all 37 effect lambdas"
```

---

## Self-Review

**Spec coverage:**
- ✅ Two HA sliders (Effect Speed, Effect Scale)
- ✅ Per-effect persistence in NVS (Preferences)
- ✅ Default 128 for new effects
- ✅ Restored when switching effects
- ✅ Restored on reboot
- ✅ Live effect responds to slider movement (via set_params on each tick)

**Placeholder scan:** None found.

**Type consistency:** `g_effect_speed`/`g_effect_scale` are `uint8_t` throughout. `EffectParams::speed` and `EffectParams::scale` are also `uint8_t`. `publish_state` takes `float` — casts are explicit.
