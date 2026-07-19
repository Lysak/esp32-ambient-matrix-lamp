#pragma once
#include <cstring>

// Named presets: static C++ table, not user-editable from HA.
// Each preset maps a display name to an effect lambda name + parameters.
struct Preset {
    const char* name;    // HA dropdown label + NVS key
    const char* effect;  // addressable_lambda name in led_matrix.yaml
    uint8_t speed;
    uint8_t scale;
    uint8_t palette;
};

static const Preset kPresets[] = {
    {"Perlin Warm (Original)", "Perlin (Original)", 200, 100, 0},
    {"Perlin Ocean",           "Perlin (Original)", 150, 120, 6},
    {"Perlin Forest",          "Perlin (Original)", 120, 140, 7},
    {"Perlin Rainbow",         "Perlin (Original)", 180, 100, 3},
    {"Perlin Party",           "Perlin (Original)", 200,  80, 2},
    {"Fire",                   "Fire Classic",      128, 180, 0},
    {"Fire 2020",              "Fire (Original)",   100, 128, 0},
    {"Confetti",               "Confetti (Original)", 128, 128, 3},
    {"Particles",              "Particles (Original)", 128, 100, 2},
    {"Tornado",                "Tornado (Original)",  100, 128, 3},
    {"Color Shift",            "Color Shift (Original)", 80, 128, 3},
    {"Scanner",                "Color Scanner 2",   128, 128, 3},
    {"Gradient",               "Gradient (Original)", 100, 128, 2},
};

static constexpr size_t kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

inline const Preset* find_preset(const char* name) {
    for (size_t i = 0; i < kPresetCount; i++)
        if (strcmp(kPresets[i].name, name) == 0) return &kPresets[i];
    return nullptr;
}
