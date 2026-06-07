# AGENTS

This repository is for the ESP32 / ESPHome ambient matrix lamp project.

## Rules

- Keep changes focused on the lamp concept and implementation.
- Prefer short, practical files and minimal diffs.
- Record important project ideas in `idea.md`.
- Keep decisions grounded in the actual hardware and ESPHome constraints.
- When reading context, use targeted tools first and avoid unnecessary long reads.
- Be careful with tokens: do not waste them on blind scans or large irrelevant dumps.
- If a detail is not yet defined, note it explicitly instead of inventing it.
- All code, identifiers, comments, and commit messages must be in English.

## Project scope

- ESP32-based controller.
- ESPHome-based firmware/configuration.
- Ambient matrix lighting effects.
- Future support for effect management and control.

## Effect sources and porting rules

LED matrix effects are ported from **GyverLamp2** (https://github.com/AlexGyver/GyverLamp2).

Rules:
- Use GyverLamp2 as the **reference source** for effect algorithms (fire, noise, particles, confetti, tornado, etc.) and UX ideas (preset rotation, clap detection, sunrise alarm, FFT sound reactivity).
- Do **not** port the full GyverLamp2 codebase. Port only the effect algorithms themselves.
- Rewrite each effect inside `effects-core/` as a clean C++ class that extends `Effect` and renders through the `MatrixCanvas` interface — no FastLED, no ESP8266, no GyverLamp2 protocol dependencies.
- ESPHome handles Wi-Fi, API, OTA, entities, I2S audio, and sensors. Home Assistant handles scheduling and automation. Do not replicate GyverLamp2's UDP protocol, app, or EEPROM logic.
- When porting an effect, note the source file in the commit message or a brief comment in the `.cpp` (e.g., `// based on fire2D.ino from GyverLamp2`).

