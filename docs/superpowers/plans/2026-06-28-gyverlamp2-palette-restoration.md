# GyverLamp2 Palette Restoration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore the main effect list to GyverLamp2-style v2 effects, remove recently-added GyverLamp v1 legacy effects from the main UI, and introduce palette-aware per-effect settings for the v2 effects that use palettes.

**Architecture:** Keep the existing `EffectEngine` and ESPHome effect lambdas, but change the parameter model from `speed + scale only` to `speed + scale + palette` on a per-effect basis. Preserve all custom effects with suffix `2`, and treat legacy v1 effects as removed from the main UI rather than remapped.

**Tech Stack:** ESPHome YAML, C++ `effects-core`, ESP32 NVS helpers in `esphome/custom/`, Makefile verification.

---

### Task 1: Remove v1 legacy effects from the main UI

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

- [x] Remove the v1 entries from the `lamp_effect.options` list:
  - `Color Change`
  - `Rainbow Vert`
  - `Rainbow Horiz`
  - `Madness 3D`
  - `Clouds 3D`
  - `Lava 3D`
  - `Plasma 3D`
  - `Rainbow 3D`
  - `Peacock 3D`
  - `Zebra 3D`
  - `Forest 3D`
  - `Ocean 3D`

- [x] Remove the corresponding `addressable_lambda` entries for those effects from the light `effects:` block.

- [x] Keep the order of the GyverLamp2 v2 effects first:
  - `Color`
  - `Color Shift`
  - `Gradient`
  - `Perlin`
  - `Particles`
  - `Fire`
  - `Fire 2020`
  - `Confetti`
  - `Tornado`

- [x] Keep all custom effects with suffix `2` unchanged and in their current order.

### Task 2: Extend persisted per-effect settings to include palette

**Files:**
- Modify: `esphome/custom/effect_prefs.h`
- Modify: `esphome/common/led_matrix.yaml`

- [x] Add a new persisted global palette value alongside `g_effect_speed` and `g_effect_scale`.

- [x] Extend NVS load/save helpers so each effect stores:
  - `speed`
  - `scale`
  - `palette`

- [x] Preserve backward compatibility:
  - existing effects without saved palette should default to a sensible GyverLamp2 palette
  - existing `speed` and `scale` values must continue loading normally

- [x] Update all effect-selection flows in YAML so selecting a new effect loads and publishes:
  - `g_effect_speed`
  - `g_effect_scale`
  - `g_effect_palette`

### Task 3: Add palette control for GyverLamp2-style effects

**Files:**
- Modify: `esphome/common/led_matrix.yaml`

- [x] Add a new palette control entity for the v2 model.

- [x] The palette options should mirror the currently-supported palette set in the repo, starting with the core GyverLamp2-relevant ones:
  - `Heat`
  - `Lava`
  - `Party`
  - `Rainbow`
  - `Rainbow Stripe`
  - `Cloud`
  - `Ocean`
  - `Forest`

- [x] Save palette changes per current effect through `effect_prefs_save(...)`.

- [x] Keep `Effect Speed` and `Effect Scale` at `0..255`, matching GyverLamp2 v2.

### Task 4: Map v2 effects to the parameters they actually use

**Files:**
- Modify: `effects-core/include/ambient_matrix/engine.h`
- Modify: `effects-core/src/engine.cpp`
- Modify: relevant effect source/header files in `effects-core/`
- Modify: `esphome/common/led_matrix.yaml`

- [x] Implement this parameter map for the first block of GyverLamp2-style effects:
  - `Perlin` -> `speed`, `scale`, `palette`
  - `Color` -> `scale`, `color`
  - `Color Shift` -> `speed`, `scale`, `palette`
  - `Gradient` -> `speed`, `scale`, `palette`
  - `Particles` -> `speed`, `scale`, `color` for now
  - `Fire` -> `speed`, `scale`, `color`
  - `Fire 2020` -> `scale`, `palette`
  - `Confetti` -> `speed`, `scale`, `color` for now
  - `Tornado` -> `speed`, `scale`, `palette`

- [x] Do not change the behavior of suffix-`2` effects in this pass.

- [x] Ensure palette-aware v2 effects consume the selected palette instead of a hardcoded one where appropriate.

### Task 5: Verification

**Files:**
- Verify: `esphome/common/led_matrix.yaml`
- Verify: `esphome/custom/effect_prefs.h`
- Verify: `effects-core/src/engine.cpp`

- [x] Run: `make esphome-compile`
- [x] Expected: `Successfully compiled program.`

- [x] If native tests still cover touched logic, run: `make test`
- [x] Expected: `all tests passed`

- [x] Summarize any effects that still need a second pass to match GyverLamp2 more precisely, especially `Particles` and `Confetti` palette-vs-color behavior.
