# Preset System & HA UI — Design Spec

**Date:** 2026-07-09  
**Approach:** B — preset-first, NVS keyed by preset name  
**Scope:** effects-core bug fixes + ESPHome YAML + new C++ preset table

---

## 1. Naming Convention

### Effects
Effects that are faithful 1:1 ports of GyverLamp2 algorithms carry the suffix `(Original)`.  
Custom additions (not in GyverLamp2) have no suffix.

Original effects must behave identically to GyverLamp2 algorithm logic, adapted to run at 60 fps.

### Presets
Presets based on GyverLamp2's single factory default carry the suffix `(Original)`.  
Our own preset combinations have no suffix.

---

## 2. Effect List

### Original effects — suffix `(Original)`

| Display name | EffectId | GyverLamp2 equivalent | Notes |
|---|---|---|---|
| Perlin (Original) | `Perlin` | Перлин | |
| Color (Original) | `Color` | Цвет | |
| Color Shift (Original) | `ColorShift` | Смена цвета | |
| Gradient (Original) | `Gradient` | Градиент | |
| Particles (Original) | `Particles` | Частицы | |
| Fire (Original) | `Fire` | Огонь | cooling formula fixed (see §5) |
| Fire 2020 (Original) | `Fire2020` | Огонь 2020 | re-init bug fixed (see §5) |
| Confetti (Original) | `Confetti` | Конфетти | |
| Tornado (Original) | `Tornado` | Смерч | |

### Custom effects — no suffix

| Display name | EffectId | Notes |
|---|---|---|
| Campfire | `Campfire` | our addition |
| Scanner | `Scanner` | from GyverLamp v1; speed bug fixed (see §5) |
| Blue Scanner | `BlueScanner` | our addition |
| Matrix Rain | `MatrixRain` | our addition |
| Space | `Space` | our addition |
| Starfield | `Starfield` | our addition |
| Noise | `Noise` | our addition |

### Hidden (entity_category: config)
The raw `select.lamp_effect` entity remains in ESPHome for debug use but is moved to `entity_category: config` — hidden from the main HA dashboard, visible in device settings.

---

## 3. Preset List

Stored as a static C++ array in `esphome/custom/preset_table.h`.  
**NVS key = preset name** (not effect name) — each preset has independent speed/scale storage.

### Original preset — suffix `(Original)`

| Preset name | Effect | Palette | Speed | Scale |
|---|---|---|---|---|
| Perlin Warm (Original) | Perlin (Original) | Heat | 200 | 100 |

Matches the GyverLamp2 factory default exactly (effect=1, palette=HeatColors_p, speed=200, scale=100).

### Custom presets — no suffix

| Preset name | Effect | Palette | Speed | Scale |
|---|---|---|---|---|
| Perlin Ocean | Perlin (Original) | Ocean | 150 | 120 |
| Perlin Forest | Perlin (Original) | Forest | 120 | 140 |
| Perlin Rainbow | Perlin (Original) | Rainbow | 180 | 100 |
| Perlin Party | Perlin (Original) | Party | 200 | 80 |
| Fire | Fire (Original) | Heat | 128 | 180 |
| Campfire | Campfire | Heat | 100 | 150 |
| Fire 2020 | Fire 2020 (Original) | Heat | 100 | 128 |
| Confetti | Confetti (Original) | Rainbow | 128 | 128 |
| Particles | Particles (Original) | Party | 128 | 100 |
| Tornado | Tornado (Original) | Rainbow | 100 | 128 |
| Color Shift | Color Shift (Original) | Rainbow | 80 | 128 |
| Scanner | Scanner | — | 128 | 128 |

---

## 4. Home Assistant Entities

### Primary UI

| Entity | Type | Description |
|---|---|---|
| `select.lamp_preset` | select | Named preset dropdown — primary control |
| `number.effect_speed` | number (0–255, step 1) | Speed slider, auto-saves to NVS per preset |
| `number.effect_scale` | number (0–255, step 1) | Scale slider, auto-saves to NVS per preset |
| `button.lamp_speed_up` | button | Speed +10 |
| `button.lamp_speed_down` | button | Speed −10, min 0 |
| `button.lamp_scale_up` | button | Scale +10 |
| `button.lamp_scale_down` | button | Scale −10, min 0 |
| `button.lamp_reset_defaults` | button | Reset speed+scale to current preset defaults |

### Debug / config (hidden from main dashboard)

| Entity | Type | Description |
|---|---|---|
| `select.lamp_effect` | select | Raw effect select, `entity_category: config` |
| `select.effect_palette` | select | Palette select, `entity_category: config` |
| `number.effect_color` | number | HSV color, `entity_category: config` |

---

## 5. C++ Changes

### 5a. New file: `esphome/custom/preset_table.h`

```cpp
struct Preset {
    const char* name;    // display name + NVS key
    const char* effect;  // must match addressable_lambda name in led_matrix.yaml
    uint8_t     speed;
    uint8_t     scale;
    uint8_t     palette; // index into effect_palette_name() table
};

static const Preset kPresets[] = {
    // --- Original (GyverLamp2 factory default) ---
    {"Perlin Warm (Original)", "Perlin (Original)", 200, 100, 0 /*Heat*/},

    // --- Custom ---
    {"Perlin Ocean",   "Perlin (Original)", 150, 120, 6 /*Ocean*/},
    {"Perlin Forest",  "Perlin (Original)", 120, 140, 7 /*Forest*/},
    {"Perlin Rainbow", "Perlin (Original)", 180, 100, 3 /*Rainbow*/},
    {"Perlin Party",   "Perlin (Original)", 200,  80, 2 /*Party*/},
    {"Fire",           "Fire (Original)",   128, 180, 0 /*Heat*/},
    {"Campfire",       "Campfire",          100, 150, 0 /*Heat*/},
    {"Fire 2020",      "Fire 2020 (Original)", 100, 128, 0 /*Heat*/},
    {"Confetti",       "Confetti (Original)", 128, 128, 3 /*Rainbow*/},
    {"Particles",      "Particles (Original)", 128, 100, 2 /*Party*/},
    {"Tornado",        "Tornado (Original)",  100, 128, 3 /*Rainbow*/},
    {"Color Shift",    "Color Shift (Original)", 80, 128, 3 /*Rainbow*/},
    {"Scanner",        "Scanner",           128, 128, 3 /*Rainbow*/},
};

static constexpr size_t kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

inline const Preset* find_preset(const char* name) {
    for (size_t i = 0; i < kPresetCount; i++)
        if (strcmp(kPresets[i].name, name) == 0) return &kPresets[i];
    return nullptr;
}
```

### 5b. Updated `esphome/custom/effect_prefs.h`

Add `preset_prefs_load(preset_name)` and `preset_prefs_save(preset_name, ...)` that use the preset name as the NVS key. The existing `effect_prefs_load/save` functions stay untouched.

New global: `inline std::string g_current_preset = "";`

### 5c. Bug fix — `effects-core/src/effect_scanner.cpp`

```cpp
// Replace hardcoded kCycleMs with speed-dependent value:
const uint32_t cycle_ms = 1200 + (255u - params.speed) * 26u;
// speed=255 → 1200ms (fast), speed=128 → ~4500ms, speed=0 → ~7830ms (slow)
```

### 5d. Bug fix — `effects-core/src/effect_fire.cpp`

```cpp
// Increase cooling to prevent flames reaching white:
uint8_t cooling = 20 + (255 - params.scale) / 8;   // was: 2 + (255-scale)/16
uint8_t sparking = 50 + params.speed / 5;            // was: 80 + speed/3
```

### 5e. Bug fix — `effects-core/src/effect_fire2020.cpp`

Remove `initialized_` guard so `delta_value_`, `delta_hue_`, `step_` recompute when `params.scale` changes:

```cpp
// Remove: if (!initialized_) { initialized_ = true; ... }
// Replace with: always recompute, OR cache last_scale_ and recompute on change
```

### 5f. Rename effect lambdas in `led_matrix.yaml`

All original-port lambdas get `(Original)` appended to their `name:` field.  
Custom lambdas keep their current names.  
The `select.lamp_effect` options list is updated accordingly.

---

## 6. Data Flow

### On preset select

```
user picks preset name
  → find_preset(name) → Preset*
  → preset_prefs_load(name)        // try NVS
      hit  → use stored speed/scale/palette
      miss → use Preset.speed / .scale / .palette (defaults)
  → set lamp effect to Preset.effect  (via existing mechanism)
  → publish speed/scale/palette to HA sliders
  → g_current_preset = name
```

### On speed/scale slider change

```
user moves slider
  → debounce 500ms (existing)
  → preset_prefs_save(g_current_preset, speed, scale, ...)
  → publish new value to HA
```

### On +/− button press

```
button.lamp_speed_up pressed
  → g_effect_speed = min(g_effect_speed + 10, 255)
  → preset_prefs_save(g_current_preset, ...)
  → effect_speed.publish_state(g_effect_speed)
```

### On reset button press

```
button.lamp_reset_defaults pressed
  → p = find_preset(g_current_preset)
  → g_effect_speed  = p->speed
  → g_effect_scale  = p->scale
  → g_effect_palette = p->palette
  → preset_prefs_save(g_current_preset, ...)  // persist defaults
  → publish speed/scale/palette to HA
```

---

## 7. Out of Scope (this iteration)

- Clock (Original) and Weather (Original) effects — not yet implemented
- Custom palette selection from HA
- Mic/ADC modulation per preset
- Auto-rotation between presets
- User-created presets via HA UI
