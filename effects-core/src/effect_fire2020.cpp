// Perlin-noise fire with rising sparks.
// Based on fire2020.ino (c) SottNick.
// Perlin noise fire procedure by Yaroslaw Turbin.
// Source: https://github.com/AlexGyver/GyverLamp2
// Original idea: https://www.reddit.com/r/FastLED/comments/hgu16i/

#include "ambient_matrix/effect_fire2020.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectFire2020::reset() {
    ff_y_ = ff_z_ = 0;
    initialized_  = false;
}

void EffectFire2020::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width();
    uint8_t h = matrix.height();

    if (!initialized_) {
        initialized_  = true;
        delta_value_  = map_int(params.scale, 0, 255, 8, 168);
        delta_hue_    = map_int(delta_value_, 8, 168, 8, 84);
        step_         = map_int(255 - delta_value_, 87, 247, 4, 32);
        uint8_t num   = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
        for (uint8_t i = 0; i < num && i < kMaxSparks; i++) {
            spark_y_[i] = random8(h);
            spark_x_[i] = random8(w);
        }
    }

    // Render base flame using Perlin noise
    for (uint8_t x = 0; x < w; x++) {
        for (uint8_t y = 0; y < h; y++) {
            uint8_t j     = h - 1 - y;                // noise row: 0=top, h-1=bottom
            uint8_t shift = (uint8_t)((uint16_t)y * 255 / (h > 1 ? h - 1 : 1));
            uint8_t n     = inoise8((uint16_t)x * delta_value_,
                                    ((uint16_t)j + ff_y_ + random8(2)) * delta_hue_,
                                    ff_z_);
            uint8_t idx = qsub8(n, shift);
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(kRainbowColors, idx, 255));
        }
    }

    // Draw rising sparks
    uint8_t num_sparks = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
    if (num_sparks > kMaxSparks) num_sparks = kMaxSparks;

    for (uint8_t i = 0; i < num_sparks; i++) {
        uint8_t sx = (uint8_t)spark_x_[i];
        uint8_t sy = (uint8_t)spark_y_[i];

        if (sy > 3 && sy < h) {
            // Recompute the flame color at row y=3, same column
            uint8_t j3   = h - 1 - 3;
            uint8_t n3   = inoise8((uint16_t)sx * delta_value_,
                                   ((uint16_t)j3 + ff_y_) * delta_hue_, ff_z_);
            uint8_t sh3  = (uint8_t)((uint16_t)3 * 255 / (h > 1 ? h - 1 : 1));
            uint8_t idx3 = qsub8(n3, sh3);
            uint8_t fade = (sy > 127) ? 255u : (uint8_t)(sy * 2u);
            canvas.set_pixel(matrix.xy(sx, sy),
                             color_from_palette(kRainbowColors, idx3, 255 - fade));
        }

        spark_y_[i] += 1.0f;
        if (spark_y_[i] >= (float)h) {
            spark_y_[i] = (float)random8(4);
            spark_x_[i] = (float)random8(w);
        }
        if (!random8(step_))
            spark_x_[i] = (float)((w + (uint8_t)spark_x_[i] + 1 - random8(3)) % w);
    }

    ff_y_++;
    if (ff_y_ & 1) ff_z_++;
}

} // namespace ambient_matrix
