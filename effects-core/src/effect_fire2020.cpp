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
    clock_.reset();
    stepper_.reset();
    ff_y_ = ff_z_ = 0;
    initialized_  = false;
}

void EffectFire2020::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    const uint8_t steps = stepper_.consume(frame, 6);
    uint8_t w = matrix.width();
    uint8_t h = matrix.height();
    const Palette16& palette = palette_by_id(params.palette);
    const int8_t rise = 1;

    if (!initialized_) {
        initialized_  = true;
        delta_value_  = map_int(params.scale, 0, 255, 8, 168);
        delta_hue_    = map_int(delta_value_, 8, 168, 8, 84);
        step_         = map_int(255 - delta_value_, 87, 247, 4, 32);
        uint8_t num   = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
        for (uint8_t i = 0; i < num && i < kMaxSparks; i++) {
            spark_y_[i] = (float)random8(4);
            spark_x_[i] = random8(w);
        }
    }

    uint8_t num_sparks = (uint8_t)(w / 8 < 1 ? 1 : w / 8);
    if (num_sparks > kMaxSparks) num_sparks = kMaxSparks;

    for (uint8_t step = 0; step < steps; step++) {
        for (uint8_t i = 0; i < num_sparks; i++) {
            spark_y_[i] += (float)rise;
            if (!matrix.in_bounds_y((int16_t)spark_y_[i])) {
                spark_y_[i] = (float)random8(4);
                spark_x_[i] = (float)random8(w);
            }
            if (!random8(step_))
                spark_x_[i] = (float)((w + (uint8_t)spark_x_[i] + 1 - random8(3)) % w);
        }

        ff_y_++;
        if (ff_y_ & 1) ff_z_++;
    }

    for (uint8_t x = 0; x < w; x++) {
        for (uint8_t y = 0; y < h; y++) {
            const uint8_t j = h - 1 - y;
            const uint8_t shift = (uint8_t)((uint16_t)y * 255 / (h > 1 ? h - 1 : 1));
            uint8_t n = cylindrical_noise8(x, (uint8_t)j, w,
                                           delta_value_,
                                           (uint16_t)(ff_z_ + ff_y_ * delta_hue_));
            uint8_t idx = qsub8(n, shift);
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(palette, idx, 255));
        }
    }

    for (uint8_t i = 0; i < num_sparks; i++) {
        uint8_t sx = (uint8_t)spark_x_[i];
        uint8_t sy = (uint8_t)spark_y_[i];
        const uint8_t spark_height = sy;
        if (spark_height > 3 && spark_height < h) {
            const uint8_t sample_height = 3;
            const uint8_t j3 = h - 1 - sample_height;
            uint8_t n3 = cylindrical_noise8(sx, (uint8_t)j3, w,
                                            delta_value_,
                                            (uint16_t)(ff_z_ + ff_y_ * delta_hue_));
            uint8_t sh3 = (uint8_t)((uint16_t)sample_height * 255 / (h > 1 ? h - 1 : 1));
            uint8_t idx3 = qsub8(n3, sh3);
            uint8_t fade = (spark_height > 127) ? 255u : (uint8_t)(spark_height * 2u);
            canvas.set_pixel(matrix.xy(sx, sy),
                             color_from_palette(palette, idx3, 255 - fade));
        }
    }
}

} // namespace ambient_matrix
