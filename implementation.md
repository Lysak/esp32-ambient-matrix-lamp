# Implementation Plan

## Goal

Start the lamp on ESPHome, connect it to Home Assistant, and then implement custom ambient-matrix effects in a controlled, production-quality way.

The first version should be stable, simple, and easy to extend. Do not try to implement everything at once.

## What to keep from GyverLamp2

GyverLamp2 is useful as a source of ideas for:

- matrix-style ambient effects
- multiple effect modes
- sound-reactive behavior
- adaptive brightness
- sunrise alarm behavior
- ordered and random effect switching
- grouping / master-slave logic as a future idea

The goal is not a 1:1 clone. The goal is to re-implement the useful effect ideas inside the ESPHome + Home Assistant stack.

## ESPHome effect integration strategy

ESPHome supports three ways to add custom effects. This project uses the production approach: a portable C++ library with a thin ESPHome adapter.

### Chosen approach: `libraries` + `includes`

Repository layout:

```
effects-core/                    ← pure C++ library, no ESPHome dependency
  library.json                   ← PlatformIO manifest
  include/ambient_matrix/
    engine.h
    effect_rainbow.h
    effect_fire.h
    ...
  src/
    engine.cpp
    effect_fire.cpp
    ...

esphome/
  custom/
    adapter.h                    ← ESPHomePixelSink + lambda glue
  ambient_matrix_esp32.yaml
  ambient_matrix_esp32s3.yaml
```

ESPHome YAML wires the library in:

```yaml
esphome:
  libraries:
    - "file://../effects-core"   # PlatformIO picks up all .cpp files
  includes:
    - custom/adapter.h           # single glue header
```

### Data flow

```
Home Assistant (select / number / switch)
    ↓  native API
ESPHome globals (g_effect_id, g_speed, g_scale)
    ↓  read by addressable_lambda every 30ms
EffectEngine::tick()             ← C++ in effects-core
    ↓
MatrixCanvas::set_pixel()        ← abstract interface
    ↓
ESPHomeMatrixCanvas              ← adapter: it[index] = color
    ↓
LED matrix (WS2812B)
```

### Why not inline lambda

All effect logic inside YAML lambda is not maintainable and not testable. `effects-core` has no ESPHome dependency, so it can be unit-tested and previewed on desktop without hardware.

### Why not external_components

`external_components` is needed when registering new ESPHome component types (sensors, entities). For rendering effects only, `libraries` + `includes` is sufficient and simpler.

## Phase 1: ESPHome skeleton

1. Create the first ESPHome configuration for the ESP32 board.
2. Add Wi-Fi, API, OTA, logger, and fallback access.
3. Define the LED matrix as the main light entity.
4. Make sure the lamp can turn on, turn off, and show a basic static color.
5. Add a brightness limit so the matrix does not overload the power supply.

## Phase 2: Home Assistant integration

1. Expose the lamp as a normal Home Assistant light.
2. Add controls for:
   - effect selection (`select` entity)
   - brightness
   - speed (`number` entity)
   - scale or intensity (`number` entity)
   - auto mode on/off
3. Add buttons for next effect and previous effect.
4. Keep the entity names simple and stable.

## Phase 3: effects-core library and first effects

1. Create `effects-core` with `Rgb`, `Matrix`, `MatrixCanvas`, `EffectEngine`.
2. Implement `ESPHomeMatrixCanvas` adapter in `esphome/custom/adapter.h`.
3. Add `addressable_lambda` in YAML that calls `engine.tick()`.
4. Implement effects one by one:
   - Rainbow (first, simplest)
   - Fire
   - Matrix
   - Sparkles
   - Noise / Plasma
5. Add ESPHome `globals` for `g_effect_id`, `g_speed`, `g_scale`.
6. Wire `select` and `number` entities to update those globals.
7. Add unit tests for XY mapping and effect bounds.
8. Add terminal preview before hardware testing.

### Terminal preview

The preview renders the 16×16 matrix directly in the terminal using ANSI 24-bit truecolor background colors. Each LED is two spaces with a colored background. No external dependencies — stdlib only.

```cpp
// one pixel
printf("\033[48;2;%d;%d;%dm  \033[0m", r, g, b);
// move cursor up after each frame
printf("\033[16A");
```

Ghostty terminal supports 24-bit color fully. The result is a smooth full-color animation in the terminal — no SDL2 or GUI needed.

`TerminalMatrixCanvas` implements `MatrixCanvas` and writes ANSI escape sequences instead of LED hardware.

## Phase 4: Audio and sensors

1. Add the microphone input for sound level or reactive behavior.
2. Add temperature monitoring if the hardware includes a sensor.
3. Use sensor data to protect the lamp from overheating.
4. Use sound input only after the core lighting path is working.

## Phase 5: Behavior and automation

1. Add effect switching rules.
2. Add ordered and random rotation modes.
3. Add sunrise mode.
4. Add optional status modes for Codex / Claude sessions.
5. Keep automation logic in Home Assistant where possible and in firmware only when needed.

## Recommended build order

1. ESPHome boots and connects.
2. LED matrix works as a light (static color, brightness limit).
3. Home Assistant can control brightness and select effects.
4. `effects-core` library is wired in, Rainbow effect works.
5. More effects are added one by one.
6. Sound and sunrise are added.

## Important rule

If a feature is not needed for the first working version, do not implement it yet.
