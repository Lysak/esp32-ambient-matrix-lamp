#pragma once
#include "types.h"
#include <cstdint>
#include <cstdlib>

namespace ambient_matrix {

inline uint8_t lerp8u(uint8_t a, uint8_t b, uint8_t t) {
    if (b >= a) return a + (uint8_t)((uint16_t)(b - a) * t >> 8);
    return        a - (uint8_t)((uint16_t)(a - b) * t >> 8);
}

inline uint8_t qsub8(uint8_t a, uint8_t b) { return a > b ? a - b : 0; }
inline uint8_t qadd8(uint8_t a, uint8_t b) { uint16_t s = (uint16_t)a + b; return s > 255 ? 255 : (uint8_t)s; }
inline uint8_t scale8(uint8_t val, uint8_t s) { return (uint16_t)val * s >> 8; }

inline uint8_t random8()                         { return (uint8_t)(rand() & 0xFF); }
inline uint8_t random8(uint8_t lim)              { return lim ? (uint8_t)(rand() % lim) : 0; }
inline uint8_t random8(uint8_t lo, uint8_t hi)   { return hi > lo ? lo + random8(hi - lo) : lo; }

inline int16_t map_int(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    if (in_max == in_min) return out_min;
    return (int32_t)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline void fade_to_black(Rgb& px, uint8_t amount) {
    px.r = scale8(px.r, 255 - amount);
    px.g = scale8(px.g, 255 - amount);
    px.b = scale8(px.b, 255 - amount);
}

inline Rgb add_rgb(const Rgb& a, const Rgb& b) {
    return { qadd8(a.r, b.r), qadd8(a.g, b.g), qadd8(a.b, b.b) };
}

// Quadratic dimming: x²/256. Used by noise effects for brightness shaping.
inline uint8_t dim8_raw(uint8_t x) { return scale8(x, x); }

} // namespace ambient_matrix
