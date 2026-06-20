// Value noise using Perlin permutation table.
// Adapted from public-domain Perlin noise by Ken Perlin.
// Reference: https://mrl.cs.nyu.edu/~perlin/noise/

#include "ambient_matrix/noise.h"
#include <cmath>

namespace ambient_matrix {

static const uint8_t kP[256] = {
    151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
    140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
    247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
     57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
     74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
     60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
     65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
    200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
     52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
    207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
    119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
    129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
    218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
     81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
    184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
    222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
};

// Cubic smoothstep: 3t^2 - 2t^3, t in [0,255] -> [0,255]
static uint8_t ease8(uint8_t t) {
    uint8_t  t2 = (uint16_t)t * t >> 8;
    uint8_t  t3 = (uint16_t)t2 * t >> 8;
    uint16_t r  = 3u * t2 - 2u * t3;
    return r > 255u ? 255u : (uint8_t)r;
}

static uint8_t lerp8(uint8_t a, uint8_t b, uint8_t t) {
    if (b >= a) return a + (uint8_t)((uint16_t)(b - a) * t >> 8);
    return        a - (uint8_t)((uint16_t)(a - b) * t >> 8);
}

uint8_t inoise8(uint16_t x) {
    uint8_t xi = x >> 8, xf = x & 0xFF;
    return lerp8(kP[xi], kP[(uint8_t)(xi + 1)], ease8(xf));
}

uint8_t inoise8(uint16_t x, uint16_t y) {
    uint8_t xi = x >> 8, yi = y >> 8;
    uint8_t xf = x & 0xFF, yf = y & 0xFF;
    uint8_t u  = ease8(xf), v = ease8(yf);
    uint8_t a  = kP[xi] + yi, b = kP[(uint8_t)(xi + 1)] + yi;
    return lerp8(lerp8(kP[a],             kP[b],             u),
                 lerp8(kP[(uint8_t)(a+1)], kP[(uint8_t)(b+1)], u), v);
}

uint8_t inoise8(uint16_t x, uint16_t y, uint16_t z) {
    uint8_t xi = x >> 8, yi = y >> 8, zi = z >> 8;
    uint8_t xf = x & 0xFF, yf = y & 0xFF, zf = z & 0xFF;
    uint8_t u = ease8(xf), v = ease8(yf), w = ease8(zf);
    uint8_t a  = kP[xi] + yi,             b  = kP[(uint8_t)(xi+1)] + yi;
    uint8_t aa = kP[a]  + zi,             ab = kP[(uint8_t)(a+1)]  + zi;
    uint8_t ba = kP[b]  + zi,             bb = kP[(uint8_t)(b+1)]  + zi;
    uint8_t x0 = lerp8(lerp8(kP[aa], kP[ba], u), lerp8(kP[ab], kP[bb], u), v);
    uint8_t x1 = lerp8(lerp8(kP[(uint8_t)(aa+1)], kP[(uint8_t)(ba+1)], u),
                       lerp8(kP[(uint8_t)(ab+1)], kP[(uint8_t)(bb+1)], u), v);
    return lerp8(x0, x1, w);
}

uint16_t inoise16(uint32_t x, uint32_t y, uint32_t z) {
    return (uint16_t)inoise8((uint16_t)(x >> 8), (uint16_t)(y >> 8), (uint16_t)(z >> 8)) << 8;
}

uint8_t cylindrical_noise8(uint8_t x, uint8_t y, uint8_t width,
                           uint16_t spatial_scale, uint16_t time) {
    if (width == 0) return 0;
    int32_t circle_x;
    int32_t circle_y;
    const int32_t radius = (int32_t)spatial_scale * width * 41 / 256;

    if (width == 16) {
        static constexpr int16_t kCos16[16] = {
             32767,  30273,  23170,  12539,      0, -12539, -23170, -30273,
            -32767, -30273, -23170, -12539,      0,  12539,  23170,  30273,
        };
        static constexpr int16_t kSin16[16] = {
                 0,  12539,  23170,  30273,  32767,  30273,  23170,  12539,
                 0, -12539, -23170, -30273, -32767, -30273, -23170, -12539,
        };
        circle_x = (int32_t)kCos16[x & 0x0F] * radius >> 15;
        circle_y = (int32_t)kSin16[x & 0x0F] * radius >> 15;
    } else {
        static constexpr float kTau = 6.28318531f;
        const float angle = kTau * x / width;
        circle_x = (int32_t)(std::cos(angle) * radius);
        circle_y = (int32_t)(std::sin(angle) * radius);
    }

    const int32_t nx = 32768 + circle_x + (int32_t)y * spatial_scale / 2;
    const int32_t ny = 32768 + circle_y + (int32_t)y * spatial_scale;
    return inoise8((uint16_t)nx, (uint16_t)ny, time);
}

} // namespace ambient_matrix
