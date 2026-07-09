#include "ambient_matrix/effect_campfire.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectCampfire::reset() {
    clock_.reset();
    stepper_.reset();
    sim_ms_ = 0;
    memset(heat_, 0, sizeof(heat_));
    memset(next_heat_, 0, sizeof(next_heat_));
    for (auto& spark : sparks_) spark = {};
}

void EffectCampfire::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams&, uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    const uint8_t steps = stepper_.consume(frame, 6);
    uint8_t w = matrix.width();
    uint8_t h = matrix.height();
    if (w > kMaxSize) w = kMaxSize;
    if (h > kMaxSize) h = kMaxSize;
    if (w == 0 || h == 0) return;

    for (uint8_t step = 0; step < steps; step++) {
        sim_ms_ += 30;
        const uint16_t noise_time = (uint16_t)(sim_ms_ / 4);

        for (uint8_t x = 0; x < w; x++) {
            const uint8_t noise = cylindrical_noise8(x, 0, w, 215, noise_time);
            const uint8_t target = qadd8(168, scale8(noise, 100));
            heat_[x][0] = lerp8u(heat_[x][0], target, 76);
            next_heat_[x][0] = heat_[x][0];
        }

        for (uint8_t y = 1; y < h; y++) {
            for (uint8_t x = 0; x < w; x++) {
                const uint8_t left_x = matrix.wrap_x((int16_t)x - 1);
                const uint8_t right_x = matrix.wrap_x((int16_t)x + 1);
                const uint16_t rising = (uint16_t)heat_[x][y - 1] * 3
                                      + heat_[left_x][y - 1]
                                      + heat_[right_x][y - 1];
                const uint16_t lateral = (uint16_t)heat_[x][y] * 2
                                       + heat_[left_x][y]
                                       + heat_[right_x][y];
                const uint8_t mixed = (uint8_t)((rising * 3 + lateral) / 19);
                const uint8_t cooling = (uint8_t)(4 + y * 2
                    + scale8(cylindrical_noise8(x, y, w, 121, noise_time), 5));
                const uint8_t target = qsub8(mixed, cooling);
                next_heat_[x][y] = lerp8u(heat_[x][y], target, 92);
            }
        }

        memcpy(heat_, next_heat_, sizeof(heat_));

        if (random8() < 14) {
            for (auto& spark : sparks_) {
                if (spark.active) continue;
                spark.x = (float)random8(w);
                spark.y = 1.0f;
                spark.vx = ((int16_t)random8(21) - 10) / 220.0f;
                spark.vy = 0.08f + (float)random8(10) / 100.0f;
                spark.life = random8(180, 255);
                spark.active = true;
                break;
            }
        }

        for (auto& spark : sparks_) {
            if (!spark.active) continue;
            spark.x += spark.vx;
            spark.y += spark.vy;
            spark.life = qsub8(spark.life, 4);

            if (spark.x < 0.0f) spark.x += w;
            if (spark.x >= w) spark.x -= w;
            if (spark.y >= h || spark.life == 0) {
                spark.active = false;
            }
        }
    }

    Rgb frame_buf[kMaxSize * kMaxSize]{};
    for (uint8_t y = 0; y < h; y++) {
        for (uint8_t x = 0; x < w; x++) {
            frame_buf[matrix.xy(x, y)] = color_from_palette(kCampfireColors,
                                                            heat_[x][y], 255);
        }
    }

    for (auto& spark : sparks_) {
        if (!spark.active) continue;
        const uint8_t sx = (uint8_t)(spark.x + 0.5f) % w;
        const uint8_t sy = (uint8_t)spark.y;
        const uint16_t index = matrix.xy(sx, sy);
        frame_buf[index] = add_rgb(frame_buf[index],
                                   color_from_palette(kCampfireColors, 238,
                                                      spark.life));
    }

    const uint16_t count = (uint16_t)w * h;
    for (uint16_t i = 0; i < count; i++) canvas.set_pixel(i, frame_buf[i]);
}

} // namespace ambient_matrix
