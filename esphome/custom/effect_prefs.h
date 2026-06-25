// Per-effect speed and scale persistence via ESP-IDF NVS.
#pragma once
#include "nvs.h"
#include "nvs_flash.h"
#include <cstdint>

inline uint8_t g_effect_speed = 128;
inline uint8_t g_effect_scale = 128;

// Derive a stable NVS key from an effect name.
// Rule: first 13 chars, lowercase, spaces→'_', then '_s' or '_c'. Max 15 chars.
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
    nvs_handle_t h;
    if (nvs_open("lampfx", NVS_READONLY, &h) != ESP_OK) return;
    uint8_t v = 128;
    g_effect_speed = (nvs_get_u8(h, ks, &v) == ESP_OK) ? v : 128;
    v = 128;
    g_effect_scale = (nvs_get_u8(h, kc, &v) == ESP_OK) ? v : 128;
    nvs_close(h);
}

inline void effect_prefs_save(const char* name, uint8_t speed, uint8_t scale) {
    char ks[16], kc[16];
    effect_prefs_key(name, 's', ks);
    effect_prefs_key(name, 'c', kc);
    nvs_handle_t h;
    if (nvs_open("lampfx", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, ks, speed);
    nvs_set_u8(h, kc, scale);
    nvs_commit(h);
    nvs_close(h);
}
