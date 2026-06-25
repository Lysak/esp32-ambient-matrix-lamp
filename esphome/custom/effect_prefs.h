// Per-effect speed and scale persistence via Arduino Preferences (NVS).
#pragma once
#include <Preferences.h>
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
