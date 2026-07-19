// Per-effect speed and scale persistence via ESP-IDF NVS.
#pragma once
#include "nvs.h"
#include "nvs_flash.h"
#include "preset_table.h"
#include <cstdint>
#include <string>

inline uint8_t g_effect_speed = 200;
inline uint8_t g_effect_scale = 100;
inline uint8_t g_effect_color = 0;
inline uint8_t g_effect_palette = 3;  // Rainbow
inline uint8_t g_effect_from_palette = 0;
inline std::string g_current_preset = "Perlin Warm (Original)";

inline const char* effect_name(uint8_t id) {
    switch (id) {
        case 0: return "Color (Original)";
        case 1: return "Color Shift (Original)";
        case 2: return "Gradient (Original)";
        case 3: return "Perlin (Original)";
        case 4: return "Particles (Original)";
        case 5: return "Fire Classic";
        case 6: return "Fire (Original)";
        case 7: return "Confetti (Original)";
        case 8: return "Tornado (Original)";
        default: return "Color (Original)";
    }
}

inline uint8_t effect_id(const std::string& name) {
    if (name == "Color (Original)") return 0;
    if (name == "Color Shift (Original)") return 1;
    if (name == "Gradient (Original)") return 2;
    if (name == "Perlin (Original)") return 3;
    if (name == "Particles (Original)") return 4;
    if (name == "Fire Classic") return 5;
    if (name == "Fire (Original)") return 6;
    if (name == "Confetti (Original)") return 7;
    if (name == "Tornado (Original)") return 8;
    return 0;
}

// Derive a stable NVS key from an effect name.
// Rule: first 13 chars, lowercase, spaces→'_', then '_x'. Max 15 chars.
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

inline const char* effect_palette_name(uint8_t id) {
    switch (id) {
        case 0: return "Heat";
        case 1: return "Lava";
        case 2: return "Party";
        case 3: return "Rainbow";
        case 4: return "Rainbow Stripe";
        case 5: return "Cloud";
        case 6: return "Ocean";
        case 7: return "Forest";
        default: return "Rainbow";
    }
}

inline uint8_t effect_palette_id(const std::string& name) {
    if (name == "Heat") return 0;
    if (name == "Lava") return 1;
    if (name == "Party") return 2;
    if (name == "Rainbow") return 3;
    if (name == "Rainbow Stripe") return 4;
    if (name == "Cloud") return 5;
    if (name == "Ocean") return 6;
    if (name == "Forest") return 7;
    return 3;
}

inline void effect_prefs_defaults(const char* name,
                                  uint8_t& speed,
                                  uint8_t& scale,
                                  uint8_t& color,
                                  uint8_t& palette,
                                  uint8_t& from_palette) {
    speed = 200;
    scale = 100;
    color = 0;
    palette = 3;  // Rainbow
    from_palette = 0;

    std::string effect(name);
    if (effect == "Fire (Original)") {
        palette = 0;  // Heat
    } else if (effect == "Fire Classic") {
        color = 8;
    } else if (effect == "Color (Original)") {
        color = 0;
        scale = 255;
    }
}

inline void effect_prefs_load(const char* name) {
    char ks[16], kc[16], kh[16], kp[16], km[16];
    effect_prefs_key(name, 's', ks);
    effect_prefs_key(name, 'c', kc);
    effect_prefs_key(name, 'h', kh);
    effect_prefs_key(name, 'p', kp);
    effect_prefs_key(name, 'm', km);
    nvs_handle_t h;
    if (nvs_open("lampfx", NVS_READONLY, &h) != ESP_OK) return;
    uint8_t speed, scale, color, palette, from_palette;
    effect_prefs_defaults(name, speed, scale, color, palette, from_palette);

    uint8_t v = speed;
    g_effect_speed = (nvs_get_u8(h, ks, &v) == ESP_OK) ? v : speed;
    v = scale;
    g_effect_scale = (nvs_get_u8(h, kc, &v) == ESP_OK) ? v : scale;
    v = color;
    g_effect_color = (nvs_get_u8(h, kh, &v) == ESP_OK) ? v : color;
    v = palette;
    g_effect_palette = (nvs_get_u8(h, kp, &v) == ESP_OK) ? v : palette;
    v = from_palette;
    g_effect_from_palette = (nvs_get_u8(h, km, &v) == ESP_OK) ? v : from_palette;
    nvs_close(h);
}

inline void effect_prefs_save(const char* name,
                              uint8_t speed,
                              uint8_t scale,
                              uint8_t color,
                              uint8_t palette,
                              uint8_t from_palette) {
    char ks[16], kc[16], kh[16], kp[16], km[16];
    effect_prefs_key(name, 's', ks);
    effect_prefs_key(name, 'c', kc);
    effect_prefs_key(name, 'h', kh);
    effect_prefs_key(name, 'p', kp);
    effect_prefs_key(name, 'm', km);
    nvs_handle_t h;
    if (nvs_open("lampfx", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, ks, speed);
    nvs_set_u8(h, kc, scale);
    nvs_set_u8(h, kh, color);
    nvs_set_u8(h, kp, palette);
    nvs_set_u8(h, km, from_palette);
    nvs_commit(h);
    nvs_close(h);
}

// Preset NVS — namespace "lamppre", keyed by preset name.
// Loads speed/scale/palette from NVS; falls back to kPresets defaults.
inline void preset_prefs_load(const char* preset_name) {
    const Preset* def = find_preset(preset_name);
    uint8_t speed    = def ? def->speed   : 128;
    uint8_t scale    = def ? def->scale   : 128;
    uint8_t palette  = def ? def->palette : 3;

    char ks[16], kc[16], kp[16];
    effect_prefs_key(preset_name, 's', ks);
    effect_prefs_key(preset_name, 'c', kc);
    effect_prefs_key(preset_name, 'p', kp);
    nvs_handle_t h;
    if (nvs_open("lamppre", NVS_READONLY, &h) == ESP_OK) {
        uint8_t v = speed;
        speed   = (nvs_get_u8(h, ks, &v) == ESP_OK) ? v : speed;
        v = scale;
        scale   = (nvs_get_u8(h, kc, &v) == ESP_OK) ? v : scale;
        v = palette;
        palette = (nvs_get_u8(h, kp, &v) == ESP_OK) ? v : palette;
        nvs_close(h);
    }
    g_effect_speed   = speed;
    g_effect_scale   = scale;
    g_effect_palette = palette;
}

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
