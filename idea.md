# ESP32 Ambient Matrix Lamp — idea.md

## Project name

**Repository name:** `esp32-ambient-matrix-lamp`

**Product / device name:** ESP32 Ambient Matrix Lamp

**Short description:**

DIY smart ambient matrix lamp inspired by GyverLamp 2. The lamp is based on ESP32, a 16×16 addressable LED matrix, Home Assistant integration, custom ambient-matrix LED effects, audio notifications, microphone input, sunrise alarm mode, and status indication for terminal agents such as Codex and Claude Code.

## Core direction

This is a second-version rewrite of the Goover Lamp concept, redesigned for ESP32 and ESPHome.

- Build an ambient matrix lamp controlled by ESP32.
- Use ESPHome as the main firmware and configuration layer.
- Add flexible control over lighting effects.
- Keep the design practical for real hardware and easy future iteration.

---

## Main goal

Create a custom smart lamp that works as:

1. Decorative ambient LED matrix lamp.
2. Home Assistant controlled smart light.
3. Status indicator for Codex / Claude Code terminal sessions.
4. Audio notification device for short pleasant sounds.
5. Music-reactive / sound-reactive lamp using a microphone.
6. Wake-up light / sunrise alarm.

This is not intended to be a full 1:1 clone of GyverLamp 2. The goal is to port the useful visual effects and interaction ideas, while replacing the original app/protocol/ecosystem with ESPHome + Home Assistant.

---

## Hardware already bought / selected

| Category | Component | Status | Notes |
|---|---:|---:|---|
| Body | Glass shade / diffuser | Bought | 10×20 cm cylindrical shade |
| LED | Addressable LED matrix | Bought | 16×16, 256 LEDs, likely WS2812B / NeoPixel-like |
| Controller | ESP32 DevKit with CP2102 | Bought / ordered | Not ESP32-S3. CP2102 is USB-UART bridge, not the ESP32 model itself |
| Power supply | 5V 60W PSU | Bought / selected | 5V 12A. Brightness/current must be limited in firmware |
| Input | Touch button | Bought / selected | Used for on/off, next effect, long press, etc. |
| Microphone | INMP441 | Bought / selected | I2S digital microphone for sound level / music reactive / clap detection |
| Audio amplifier | MAX98357A | Bought | I2S DAC + mono Class-D amplifier |
| Speaker | 3525 4R 3W | Bought | 35×25 mm, 4Ω, 3W speaker |
| LED protection | Capacitor | Available | 1000 µF 35V, from FPV stack kit. Use near LED power input |
| Thin cable | Audio cable 2×0.35 mm² | Available | Good for speaker and small signals, not for LED power |
| Power cable | 18 AWG | Planned / considered | Good for LED power. Approx. 0.82 mm² |

---

## Hardware still needed / recommended

| Category | Component | Priority | Notes |
|---|---:|---:|---|
| LED data protection | 470Ω resistor | Required | Put in series with LED DATA line, preferably near LED DIN |
| Logic level shifting | 74AHCT125 / 74HCT245 | Recommended | ESP32 DATA is 3.3V, LED matrix is 5V. Use this for stable WS2812B data |
| Power protection | Fuse + holder | Recommended | 10A or 15A inline fuse on +5V input |
| Power wiring | 18 AWG or 0.75–1.5 mm² wire | Required | For +5V/GND to LED matrix |
| Power connector | XT30 or screw terminal | Recommended | Avoid weak small DC jacks if carrying high current |
| Temperature | DS18B20 or NTC 10K | Recommended | Reduce brightness if inside temperature is too high |
| Ambient light | BH1750 (I2C) | Recommended | Auto-brightness based on room light level |
| Mechanical core | PVC/plumbing pipe | Missing | Original-style inner tube, likely 40 mm |
| Mechanical cap | Plumbing end cap | Missing | 40 mm cap/plug |
| Diffusion | White tracing paper / diffuser film | Missing | Not carbon copy paper. Use white tracing paper or LED diffuser film |
| Mounting | Heat-shrink, flux, cable ties, standoffs | Needed | For safe internal wiring |
| Sound openings | Holes/grille in bottom base | Needed | Speaker must not be fully sealed inside the base |
| Ventilation | Bottom/top airflow holes | Needed | 256 LEDs can generate heat |

---

## Important wiring notes

### LED power

Do not power the LED matrix through the ESP32.

```text
5V PSU +  → LED +5V
5V PSU -  → LED GND
5V PSU -  → ESP32 GND
5V PSU -  → MAX98357A GND
5V PSU -  → INMP441 GND
```

All grounds must be common.

### Capacitor

Use the existing **1000 µF 35V** electrolytic capacitor near the LED matrix power input:

```text
capacitor + → LED +5V
capacitor - → LED GND
```

The stripe on the capacitor usually marks the negative side.

### LED DATA resistor

Put the 470Ω resistor only into the LED DATA line:

```text
ESP32 GPIO / level shifter output → 470Ω → LED DIN
```

Do not put this resistor into +5V or GND.

### Level shifter

Preferred final wiring:

```text
ESP32 GPIO → 74AHCT125 input
74AHCT125 output → 470Ω → LED DIN
74AHCT125 VCC → 5V
74AHCT125 GND → common GND
```

### Speaker / audio

Use MAX98357A, not PAM8403, for this design.

Reason:

```text
Home Assistant → Wi‑Fi → ESP32 → I2S digital audio → MAX98357A → speaker
```

PAM8403 is an analog amplifier. It expects analog audio input. It is not a direct replacement for MAX98357A unless an extra I2S DAC is added.

Speaker wiring:

```text
MAX98357A OUT+ → speaker +
MAX98357A OUT- → speaker -
```

The 2×0.35 mm² audio cable is fine for the speaker.

---

## Firmware direction

Use **ESPHome as the main firmware base**.

Reasons:

1. Native Home Assistant integration.
2. OTA updates after first USB flash.
3. Good fit for entities: light, select, number, button, switch, media_player, sensors.
4. Supports custom addressable LED effects through lambda / C++.
5. Supports I2S audio on ESP32 for MAX98357A.
6. Better for the intended device than keeping the original GyverLamp app/protocol.

Do not start with a full 1:1 GyverLamp2 port. Instead:

```text
GyverLamp2 / similar projects → source of effect algorithms and UX ideas
ESPHome → device platform, Home Assistant API, OTA, audio, buttons, configuration
Custom C++ library → portable effects engine
```

---

## Target features

### Light / effects

MVP effects:

1. Rainbow
2. Fire
3. Matrix
4. Sparkles
5. Noise / Plasma
6. Night Light
7. Sunrise
8. Codex Working
9. Claude Working
10. Done / Error status effects

Later:

1. More ambient-matrix effects.
2. Palettes.
3. Auto mode / random mode.
4. Music-reactive effects.
5. Clock display (HH:MM or hours only).
6. Temperature / percent digit display.
7. Weather ambient effects (rain, sun, storm, snow).
8. Morse code mode (blink any text from HA).

### Home Assistant entities

Expected entities:

```text
light.esp32_ambient_matrix_lamp
select.lamp_effect
number.lamp_speed
number.lamp_scale
number.lamp_brightness_limit
button.lamp_next_effect
button.lamp_previous_effect
button.lamp_start_sunrise
button.lamp_stop_sunrise
switch.lamp_auto_mode
sensor.lamp_temperature
media_player.lamp_speaker
```

### Button behavior

Initial idea:

```text
Short press        → next effect
Double press       → on/off
Long press         → night mode or brightness control
Very long press    → reset / safe mode action, optional
```

### Codex / Claude Code status behavior

The lamp should be controllable from terminal scripts through Home Assistant HTTP API or local API.

Example statuses:

```text
Codex started      → blue / cyan slow pulse + short start sound
Claude started     → purple slow pulse + short start sound
Working            → continue running effect
Done               → green confirmation animation + pleasant done sound
Error              → red blink + error sound
Waiting input      → yellow/orange attention effect
Idle               → restore previous decorative effect
```

Home Assistant should orchestrate both light effect and sound playback.

### Sound

No SD card. Short sounds should be played through Home Assistant / URL / media service.

Target path:

```text
Home Assistant stores or generates MP3/WAV
Home Assistant calls media_player.play_media
ESP32 receives stream/URL
MAX98357A plays through 4Ω 3W speaker
```

Short sounds:

```text
start.mp3
done.mp3
error.mp3
water_drop.mp3
soft_notification.mp3
```

### Ambient light sensor (BH1750)

**Hardware:** BH1750, I2C digital sensor, 1–65535 lux range, 3.3V native (no level shifter needed).

**Placement:** Glued to the back of the frosted shade, above the power cable entry point — hidden from view, wire routed alongside the existing cable.

**Reading strategy — fade-to-measure:**

Because the sensor is near the LED matrix, reading while the lamp is on would mix ambient light with the lamp's own output. Instead:

```text
Every N minutes (e.g. 5 min):
  1. Fade lamp to 0 over ~1 second
  2. Wait 150 ms
  3. BH1750 reads ambient lux
  4. Fade lamp back to target brightness over ~1 second
```

When the lamp is off, readings can be taken directly without any fade.

**Calibration:** On first setup, measure lux values in two reference conditions:
- Bright day / lights on → record as `lux_max`
- Night / lights off → record as `lux_min`

Map the 0–100% brightness range to this lux range. ESPHome `globals` store the calibrated min/max; they can be updated from Home Assistant without reflashing.

**Auto-brightness behavior:**

```text
lux < lux_min threshold  → lamp at minimum brightness (e.g. 10%)
lux > lux_max threshold  → lamp at maximum configured brightness
between                  → linear interpolation
```

Hysteresis recommended to avoid flicker at threshold boundaries.

---

### Letter / digit display

Large characters rendered on the 16×16 matrix using a bold font (~10–14 px tall). At this size strokes are 2–3 px wide — readable through diffusion.

**Use cases:**

- Status notifications: large **"C"** (purple) = Claude active, **"G"** (blue) = Codex, **"✓"** (green) = done, **"!"** (red) = error
- Clock: hours only as 2 large digits, or HH:MM in 5×7 font (fits exactly in 16 px wide)
- Temperature: 2–3 digit value + degree symbol
- Any short label sent from Home Assistant

**Implementation:** a reusable `drawChar()` / `drawDigit()` function in `effects-core`, similar to GyverLamp2's `drawDigit()` + `drawClock()`. Characters are bright on black background for maximum contrast through diffusion.

---

### Morse code mode

A dedicated effect mode where the lamp blinks a message in Morse code.

**How it works:**

```text
Home Assistant sends a text string → ESPHome encodes it to Morse → lamp blinks dot/dash pattern
```

Morse timing (standard):
```
dot          = 1 unit on
dash         = 3 units on
gap between symbols  = 1 unit off
gap between letters  = 3 units off
gap between words    = 7 units off
```

Unit duration is configurable (speed param), e.g. 150–300 ms per unit.

**Visual:** full matrix lights up in a chosen color for on-states, goes dark for off-states. Simple and clear even through diffusion.

**Input:** a `text` entity or HA script parameter. Example: send `"SOS"` → lamp blinks `··· — — — ···`.

This is a standalone effect mode, not part of the ambient effect rotation.

---

### Weather display

Weather data is sourced from Home Assistant (OpenWeatherMap or similar integration). HA passes the current condition to the lamp as a select or light effect trigger.

**Approach:** dedicated ambient weather effects, not text/icons (16×16 is too small for readable text).

Planned effects:

| Condition | Effect idea |
|---|---|
| Clear / sunny | warm yellow/white glow, slow pulse |
| Cloudy | cool white/grey, slow dim drift |
| Rain | blue drizzle animation, drops falling downward |
| Storm / thunder | dark blue base + random white flash |
| Snow | white sparkles drifting down |
| Fog | low-brightness white noise wash |

**Integration path:**

```text
Home Assistant → OpenWeatherMap integration
→ automation triggers on weather state change
→ calls light effect select on the lamp
→ ESPHome renders the matching effect
```

Weather effects are lower priority than core ambient effects. Implemented in Phase 4 or later.

---

### Microphone

INMP441 is for:

1. Music reactive modes.
2. Sound level visualization.
3. Possible clap detection.

Voice assistant / wake word is not the primary goal for the current ESP32 CP2102 board. That would be better for a future ESP32-S3 with PSRAM.

### Sunrise alarm

Implement as:

```text
Home Assistant stores schedule, days, and alarm time.
ESPHome exposes a Sunrise effect and start/stop controls.
Home Assistant triggers sunrise mode at the right time.
ESPHome renders the sunrise animation locally.
At final time, Home Assistant optionally plays a soft sound through media_player.
```

Sunrise animation idea:

```text
Start: very low brightness, warm red/orange
Middle: orange/yellow, increasing brightness
End: warm white / soft daylight, final configured brightness
Optional: matrix animation where light rises from bottom to top
```

---

## Software architecture

The project should not be one huge YAML file. Use a layered architecture.

Recommended repo structure:

```text
esp32-ambient-matrix-lamp/
├── README.md
├── idea.md
├── effects-core/
│   ├── include/
│   │   └── ambient_matrix/
│   │       ├── color.h
│   │       ├── matrix.h
│   │       ├── engine.h
│   │       ├── effects.h
│   │       ├── effect_rainbow.h
│   │       ├── effect_fire.h
│   │       ├── effect_matrix.h
│   │       ├── effect_sparkles.h
│   │       └── effect_sunrise.h
│   ├── src/
│   │   ├── matrix.cpp
│   │   ├── engine.cpp
│   │   ├── effect_rainbow.cpp
│   │   ├── effect_fire.cpp
│   │   ├── effect_matrix.cpp
│   │   ├── effect_sparkles.cpp
│   │   └── effect_sunrise.cpp
│   └── library.json
│
├── esphome/
│   ├── ambient_matrix_esp32.yaml
│   ├── ambient_matrix_esp32s3.yaml
│   ├── common/
│   │   ├── base.yaml
│   │   ├── led_matrix.yaml
│   │   ├── controls.yaml
│   │   ├── audio.yaml
│   │   └── effects.yaml
│   ├── boards/
│   │   ├── esp32-cp2102.yaml
│   │   └── esp32-s3.yaml
│   └── custom/
│       └── esphome_effect_adapter.h
│
├── simulator/
│   ├── wokwi/
│   │   ├── sketch.ino
│   │   ├── diagram.json
│   │   └── libraries.txt
│   └── desktop-preview/
│       └── optional
│
├── home-assistant/
│   ├── scripts.yaml
│   ├── automations.yaml
│   └── dashboard-notes.md
│
└── tests/
    ├── test_xy_mapping.cpp
    ├── test_effect_engine.cpp
    └── test_effect_bounds.cpp
```

---

## Effects library design

Create a private portable C++ library: `effects-core`.

The library should not depend directly on ESPHome. It should only know about abstract pixels and matrix geometry.

Core concepts:

```cpp
namespace ambient_matrix {

struct Rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum class EffectId : uint8_t {
  Rainbow,
  Fire,
  Matrix,
  Sparkles,
  Sunrise,
  CodexWorking,
  ClaudeWorking,
  Done,
  Error,
};

struct EffectConfig {
  uint8_t speed = 128;
  uint8_t scale = 128;
  uint8_t brightness = 128;
};

class PixelSink {
 public:
  virtual void set_pixel(uint16_t index, Rgb color) = 0;
  virtual void clear() = 0;
  virtual uint16_t size() const = 0;
  virtual ~PixelSink() = default;
};

class Matrix {
 public:
  Matrix(uint8_t width, uint8_t height);
  uint16_t xy(uint8_t x, uint8_t y) const;
};

class EffectEngine {
 public:
  explicit EffectEngine(Matrix matrix);
  void set_effect(EffectId effect);
  void set_config(EffectConfig config);
  void reset();
  void tick(PixelSink& pixels, uint32_t now_ms);
};

}
```

ESPHome adapter should convert ESPHome `AddressableLight` to `PixelSink`.

---

## ESPHome integration strategy

Start simple:

1. Use `includes` for a small ESPHome adapter header.
2. Use `libraries` or local file path for `effects-core`.
3. Later consider `external_components` when the API stabilizes.

Initial ESPHome approach:

```yaml
esphome:
  name: esp32-ambient-matrix-lamp
  friendly_name: ESP32 Ambient Matrix Lamp
  includes:
    - custom/esphome_effect_adapter.h
  libraries:
    - "file:///config/esphome/libs/effects-core"
```

Light effect approach:

```yaml
light:
  - platform: esp32_rmt_led_strip
    id: led_matrix
    name: "Ambient Matrix Lamp"
    pin: ${led_pin}
    num_leds: 256
    chipset: WS2812
    rgb_order: GRB
    effects:
      - addressable_lambda:
          name: "Ambient Matrix Engine"
          update_interval: 30ms
          lambda: |-
            static ambient_matrix::EffectEngine engine(ambient_matrix::Matrix(16, 16));
            ESPHomePixelSink sink(it);
            if (initial_run) {
              engine.reset();
            }
            engine.tick(sink, millis());
```

Board-specific config should be separated:

```text
common logic = shared
ESP32 CP2102 board config = current lamp
ESP32-S3 board config = future lamp
```

This keeps the firmware mostly reusable for a future ESP32-S3 lamp.

---

## Testing strategy before ESP32 arrives

Yes, development can start before the physical ESP32 arrives.

### 1. Effects-core unit tests

Test:

```text
XY mapping
bounds safety
effect tick does not write outside 0..255
speed/scale values do not break effects
reset behavior
```

### 2. Visual simulator

Create a desktop or browser preview for 16×16 matrix.

The preview should render every LED as a square. It should call the same `EffectEngine::tick()` code as ESPHome.

### 3. Wokwi simulator

Use Wokwi for ESP32 + FastLED / NeoPixel style visual simulation if convenient.

### 4. ESPHome compile without device

Even without ESP32, run:

```bash
esphome compile esphome/ambient_matrix_esp32.yaml
```

This validates YAML and C++ integration.

### 5. Real hardware dev loop after ESP32 arrives

First flash via USB. Later updates by OTA.

Development loop:

```text
edit effect → run simulator → compile ESPHome → OTA flash → check physical LED matrix → check logs
```

Do not assemble everything into the final shade immediately. First build a desk test bench:

```text
ESP32
LED 16×16
5V PSU
1000 µF capacitor
470Ω data resistor
common GND
```

Then add:

```text
MAX98357A
speaker
INMP441
touch button
temperature sensor
final body
```

---

## ESP32 vs ESP32-S3 support

Current board is normal ESP32 DevKit / ESP32-WROOM-32 with CP2102.

Future board may be ESP32-S3.

The main effect library and high-level logic should stay the same. Only hardware-specific configuration should differ:

```text
board type
GPIO pins
I2S pins
PSRAM availability
framework options
possibly audio component details
```

Use separate YAML targets:

```text
esphome/ambient_matrix_esp32.yaml
esphome/ambient_matrix_esp32s3.yaml
```

Do not hardcode pins inside the effect library.

---

## Implementation phases

### Phase 1 — repo and effects core

1. Create repository `esp32-ambient-matrix-lamp`.
2. Add `idea.md`.
3. Add `effects-core` skeleton.
4. Implement `Rgb`, `Matrix`, `PixelSink`, `EffectEngine`.
5. Implement simple `Rainbow` effect.
6. Add unit tests for XY mapping.
7. Add simple visual preview.

### Phase 2 — first ESPHome firmware

1. Create `ambient_matrix_esp32.yaml`.
2. Configure ESP32 board.
3. Configure Wi-Fi/API/OTA/logger.
4. Configure LED matrix using `esp32_rmt_led_strip`.
5. Add `Ambient Matrix Engine` addressable lambda.
6. Compile without device.
7. Flash when ESP32 arrives.

### Phase 3 — basic interaction

1. Add `select` for effect selection.
2. Add `number` for speed.
3. Add `number` for scale.
4. Add touch button behavior.
5. Add Home Assistant dashboard controls.

### Phase 4 — port useful ambient-matrix effects

Port effects one by one:

1. Fire
2. Matrix
3. Sparkles
4. Noise / Plasma
5. Snow / Ocean / other desired effects

For each effect, document:

```text
source file / inspiration
required state
required parameters
dependencies
porting difficulty
status
```

### Phase 5 — audio output

1. Add MAX98357A I2S config.
2. Add `media_player` entity.
3. Test short sound playback from Home Assistant.
4. Add HA scripts: play start/done/error sounds.

### Phase 6 — microphone

1. Add INMP441 I2S microphone.
2. Expose sound level if feasible.
3. Implement simple clap detection if stable.
4. Later implement music-reactive effects.

### Phase 7 — status automation

1. Create Home Assistant scripts:
   - `lamp_codex_start`
   - `lamp_codex_done`
   - `lamp_codex_error`
   - `lamp_claude_start`
   - `lamp_claude_done`
2. Create terminal helper scripts that call Home Assistant API.
3. Preserve and restore previous lamp state after status mode.

### Phase 8 — sunrise alarm

1. Add Sunrise effect to effects-core.
2. Add start/stop sunrise controls.
3. Create Home Assistant automation for alarm time and weekdays.
4. Play optional soft sound at the end.

### Phase 9 — safety and final assembly

1. Add brightness/current limit.
2. Add temperature sensor and thermal protection.
3. Add fuse.
4. Add ventilation.
5. Assemble inside shade and base.
6. Keep USB accessible if possible.

---

## Key design decisions

1. Use ESPHome as the firmware platform.
2. Do not depend on GyverLamp Android app.
3. Port effects, not the whole GyverLamp2 ecosystem.
4. Keep effects in a private portable C++ library.
5. Use Home Assistant for orchestration: Telegram, terminal status, sunrise schedule, audio triggering.
6. Use MAX98357A for I2S audio output.
7. Use INMP441 for sound input.
8. Use OTA after first USB flash.
9. Keep board-specific differences isolated from the core logic.

---

## Technical references

Use official docs first:

- ESPHome external components: https://esphome.io/components/external_components/
- ESPHome light effects / addressable lambda: https://esphome.io/components/light/
- ESPHome I2S audio media player: https://esphome.io/components/media_player/i2s_audio/
- ESPHome I2S audio component: https://esphome.io/components/i2s_audio/
- ESPHome OTA: https://esphome.io/components/ota/esphome/
- ESPHome developer component architecture: https://developers.esphome.io/architecture/components/

---

## GyverLamp2 firmware analysis

> Source: https://github.com/AlexGyver/GyverLamp2  
> Platform: ESP8266 + FastLED. Not a 1:1 target — used as a reference for effect ideas and UX patterns.

### Main loop execution order

1. `timeTicker()` — internal clock update
2. WiFi reconnect if disconnected
3. UDP packet parsing — receive and apply commands
4. `checkEEupdate()` — flush deferred EEPROM writes
5. `presetRotation()` — auto-switch presets if enabled
6. `effectsRoutine()` — render current effect to LEDs
7. `button()` — poll physical button
8. `checkAnalog()` — audio / light sensor processing
9. `iAmOnline()` — periodic UDP status broadcast

---

### Feature hierarchy

- **Effect engine** (`effects.ino`, `fire2020.ino`, `fire2D.ino`)
  - 11 built-in effects
    - Perlin Noise — params: speed, scale, palette
    - Solid Color — params: color (HSV), scale, brightness
    - Color Transition — params: speed, palette, brightness
    - Gradient — params: speed, scale, palette; optional sound-reactive
    - Particles — params: speed, scale, palette/color, decay; noise-driven movement
    - Fire Classic — params: speed, scale, color, palette; heat array with dissipation
    - Fire 2020 (`fire2020.ino`) — Perlin noise flames + spark particles
      - deltaValue (8–168): flame width
      - deltaHue (8–84): flame height
      - step (4–32): spark drift probability
      - spark system: width/8 rising particles with fade
    - Fire 2D (`fire2D.ino`) — lookup-table driven (valueMask, hueMask) with frame interpolation
      - rows 0–10: gradient mask
      - row 11: 20% probability spark generation
      - rows 12+: downward brightness propagation
    - Confetti — random pixel activation with fade-to-black decay
    - Tornado — 2D noise swirling vortex
    - Clock Display — `drawClock()` HH:MM with optional scrolling
  - Global effect params: brightness, speed, scale, palette (32 options), sound reactivity mode
  - 32 palettes: built-in (Heat, Lava, Party, Rainbow, Ocean, Forest…) + custom gradients (cyberpunk, aurora, xmas, acid, leo…)
  - Sound reactivity modes: brightness-reactive, scale-reactive, length-reactive

- **Audio & sound processing** (`FFT_C.h`, `VolAnalyzer.h`, `Clap.h`, `analog.ino`)
  - FFT engine — 64-point, bit-reversal → butterfly → magnitude, right-shift ×18 for normalization
  - Volume analyzer
    - sampling every 500 µs, 20-sample window, exponential smoothing every 150 ms
    - outputs: raw, peak, filtered vol (0–100), dynamic min/max, pulse flag
  - Clap detection
    - derivative-based edge detection, sampled every 10 ms
    - threshold: 150, timeout: 700 ms, debounce: 200 ms
    - detects: single clap, exact count (hasClaps(n)), returns count
  - ADC config: 3 processors (volume, low-freq, high-freq); modes: brightness-only / mic-only / both

- **Preset management** (`presetManager.ino`, `data.h`)
  - Max 40 presets, 13 bytes each
  - Each preset stores: effect type, fade, brightness override, advanced mode (None/Volume/Low/High/Clock), sound reactivity on/off, reaction type, signal thresholds, speed, scale, palette, center-animation, color, palette mode
  - `presetRotation(force)` — auto-switch with random or sequential mode
  - `changePreset(dir)` — ±1 with wraparound
  - `setPreset(n)` — direct with bounds check
  - 2-second hold timer pauses auto-rotation during manual input

- **Sunrise / dawn alarm** (`time.ino`, `data.h`)
  - 24 bytes per dawn config: weekly enable flags (7 days), hour/minute per day, target brightness, fade-in duration, post-alarm duration
  - `checkDawn()` — triggers fade sequence on schedule match
  - `checkWorkTime()` — auto on/off during work hours with midnight wraparound

- **Time sync & RTC** (`NTPClient-Gyver.h`, `Time.h`)
  - NTP: `pool.ntp.org`, local port 1337, update interval configurable, timezone ±13
  - `forceUpdate()` for immediate sync
  - 30 ms local tick fallback when offline
  - UTC conversion with timezone offset

- **Button input** (`button.ino`, `Button.h`)
  - 1 click: toggle power
  - 2 clicks: next preset + slave sync
  - 3 clicks: previous preset + slave sync
  - 4 clicks: reset to preset 0 + slave sync
  - 5 clicks: set role Master (indicator: dark slate blue)
  - 6 clicks: set role Slave (indicator: maroon)
  - Hold (on): brightness ±5 per 80 ms, direction toggles on release, range 0–255, syncs to slaves + persists to EEPROM
  - Click timeout: 800 ms

- **WiFi & network** (`startup.ino`, `parsing.ino`)
  - Modes: AP (yellow LED) / Station (green = ok, red = fail)
  - Retry: 3 attempts in 10-second windows
  - Protocol: UDP, message format `GL,<type>,<data…>`
  - Command types:
    - Type 0: power, brightness min/max, preset nav, WiFi mode, role, group, SSID/pass, restart, OTA, fade-timer
    - Type 1: full cfg transfer
    - Type 2: preset data transfer
    - Type 3: dawn/alarm settings
    - Type 4: master→slave commands (fade, set preset, set brightness)
    - Type 5: palette data
    - Type 6: time sync (AP mode)
    - Type 7: ADC data (length, scale, brightness)
  - `iAmOnline()` — periodic broadcast with preset info for device discovery

- **Device roles & groups** (`data.h`)
  - Roles: Master / Slave
  - Groups: 1–10; same group = sync
  - Master broadcasts Type 4 commands; Slaves apply in real-time

- **OTA firmware update** (`startup.ino`)
  - Version tracking via `GL_VERSION`
  - `checkUpdate()` on startup
  - Indicators: cyan = update available, blue = processing
  - Library: `ESP8266httpUpdate`

- **Storage / EEPROM** (`eeprom.ino`, `data.h`)
  - Total: 1000 bytes
  - Layout: key (1 B) → cfg (12 B) → dawn (24 B) → pal → presets
  - cfg (12 B): brightness, device type, matrix dimensions, ADC mode, work hours, rotation mode, WiFi creds, MQTT, group, role, timezone, NTP server, power state, preset index
  - Write strategy: deferred (flag + timer) to reduce EEPROM wear; immediate for critical changes

- **Sensors** (`analog.ino`)
  - Photosensor: A0 ADC, 1000 ms interval, min/max calibration (0–1023), power pin control
  - Microphone: power pin control, `vol.tick()` continuous, clap + FFT analysis
  - ADC mode: brightness-only / mic-only / both (2000 ms timeout fallback)

- **Display utilities** (`0_func.ino`)
  - `drawDigit(digit, x, y, color)` — 5×7 font, digits 0–9
  - `drawDots(x, y, color)` — time separator colon
  - `drawClock(y, speed, color)` — full HH:MM with optional scroll
  - `blink16(color)` — 16-LED status blink (3 cycles × 300 ms)

- **LED hardware** (`startup.ino`)
  - Library: FastLED 3.4.0
  - Max LEDs: 300–900
  - Matrix types: 1D Strip / Zigzag / Parallel
  - Dimensions: up to 16×16
  - `startStrip()` initialization with current limiting

- **Utility libraries**
  - `mString.h` — string operations
  - `fastRandom.h` — fast embedded RNG
  - `timeRandom.h` — time-seeded randomization
  - `timerMillis.h` — millisecond timer class
  - `FastFilter.h` — exponential smoothing for analog signals

---

### What is relevant for this project

| Feature | GyverLamp2 has it | Our plan | Notes |
|---|---|---|---|
| Fire Classic | ✅ | Port | Core effect |
| Fire 2020 / Fire 2D | ✅ | Port (one variant) | Pick the better one |
| Perlin Noise / Plasma | ✅ | Port | Good ambient effect |
| Particles | ✅ | Port | Nice for sparkles |
| Confetti | ✅ | Port | Simple, nice |
| Tornado | ✅ | Later | Interesting |
| Gradient | ✅ | Port | Easy baseline |
| 32 palettes | ✅ | Partial | Pick ~10 useful ones |
| Clock display | ✅ | Skip / Later | Low priority |
| Sunrise alarm | ✅ | Port | Phase 8 |
| Clap detection | ✅ | Later | Phase 6 |
| FFT / music reactive | ✅ | Later | Phase 6 |
| Volume analyzer | ✅ | Port logic | For music-reactive effects |
| Preset system (40 presets) | ✅ | Simplified | HA select replaces this |
| Auto rotation | ✅ | HA automation | Let HA handle scheduling |
| Master / Slave sync | ✅ | Skip | Single device |
| OTA | ✅ | ESPHome OTA | Already handled by ESPHome |
| WiFi AP fallback | ✅ | ESPHome captive portal | Already handled |
| EEPROM persistence | ✅ | ESPHome globals + flash | Already handled |
| NTP / RTC | ✅ | ESPHome `sntp` | Already handled |
| Button multi-click | ✅ | Port UX idea | Adapt to ESPHome button component |
| Work hours auto on/off | ✅ | HA automation | Let HA schedule it |
| Photosensor | ✅ (LDR/ADC) | BH1750 instead | Digital I2C, cleaner than ADC; fade-to-measure approach |
| Weather display | ❌ (TODO in code) | Add | HA → OpenWeatherMap → ambient weather effects |

---

## Prompt for Codex / Claude Code

You are helping implement a firmware project named `esp32-ambient-matrix-lamp`.

The device is a DIY ESP32 smart ambient LED matrix lamp inspired by GyverLamp 2. It uses a 16×16 WS2812B-like LED matrix, ESP32 DevKit with CP2102, Home Assistant + ESPHome, INMP441 I2S microphone, MAX98357A I2S amplifier, 4Ω 3W speaker, touch button, and 5V 60W PSU.

Important architecture:

1. Do not create one huge YAML file.
2. Create a portable C++ `effects-core` library for LED matrix effects.
3. The effects library must not depend directly on ESPHome.
4. Effects must render through an abstract `PixelSink` interface.
5. ESPHome should only adapt `AddressableLight` to `PixelSink`.
6. Use ESPHome for Wi-Fi, API, OTA, Home Assistant entities, button, I2S audio, and sensors.
7. Port ambient-matrix effects gradually: Rainbow first, then Fire, Matrix, Sparkles, Sunrise.
8. Keep board-specific config separate so the same logic can later support ESP32-S3.
9. Add tests for XY mapping and effect bounds.
10. Add a visual simulator/preview for 16×16 before hardware testing.

Start by creating the repository structure, `effects-core` skeleton, a simple Rainbow effect, XY mapping tests, and an initial ESPHome YAML that compiles.
