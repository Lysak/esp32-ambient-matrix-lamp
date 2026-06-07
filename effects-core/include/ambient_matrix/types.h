#pragma once
#include <cstdint>

namespace ambient_matrix {

struct Rgb {
    uint8_t r, g, b;

    constexpr Rgb() : r(0), g(0), b(0) {}
    constexpr Rgb(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    static Rgb from_hsv(uint8_t h, uint8_t s, uint8_t v) {
        if (s == 0) return {v, v, v};
        uint8_t region    = h / 43;
        uint8_t remainder = (h - region * 43) * 6;
        uint8_t p = (uint16_t)v * (255 - s) >> 8;
        uint8_t q = (uint16_t)v * (255 - ((uint16_t)s * remainder >> 8)) >> 8;
        uint8_t t = (uint16_t)v * (255 - ((uint16_t)s * (255 - remainder) >> 8)) >> 8;
        switch (region) {
            case 0:  return {v, t, p};
            case 1:  return {q, v, p};
            case 2:  return {p, v, t};
            case 3:  return {p, q, v};
            case 4:  return {t, p, v};
            default: return {v, p, q};
        }
    }
};

} // namespace ambient_matrix
