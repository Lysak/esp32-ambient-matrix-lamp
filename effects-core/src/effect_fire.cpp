// Classic 2D heat-diffusion fire.
// Based on case 6 (FIRE) from GyverLamp2/effects.ino and FastLED Fire2012.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_fire.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectFire::reset() {
    clock_.reset();
    stepper_.reset();
    memset(heat_, 0, sizeof(heat_));
}

void EffectFire::tick(MatrixCanvas& canvas, const Matrix& matrix,
                      const EffectParams& params, uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    const uint8_t steps = stepper_.consume(frame, 6);
    uint8_t w = matrix.width();
    uint8_t h = matrix.height();
    if (w > 16) w = 16;
    if (h > 16) h = 16;
    const int8_t rise = 1;
    const int16_t top = h - 1;

    // cooling: scale 0 = slow cooling (tall flames), scale 255 = fast (short)
    uint8_t cooling = 2 + (255 - params.scale) / 16;
    // sparking: speed controls spark frequency
    uint8_t sparking = 80 + params.speed / 3;

    for (uint8_t step = 0; step < steps; step++) {
        for (uint8_t x = 0; x < w; x++) {
            for (uint8_t y = 0; y < h; y++)
                heat_[x][y] = qsub8(heat_[x][y], random8(0, cooling));

            for (int16_t y = top; matrix.in_bounds_y(y); y -= rise) {
                const int16_t below_1 = y - rise;
                const int16_t below_2 = y - 2 * rise;
                if (!matrix.in_bounds_y(below_2)) break;
                heat_[x][y] = ((uint16_t)heat_[x][below_1] + heat_[x][below_2] + heat_[x][below_2]) / 3;
            }

            if (random8() < sparking) {
                const uint8_t y = random8(3);
                heat_[x][y] = qadd8(heat_[x][y], random8(160, 255));
            }
        }
    }

    for (uint8_t x = 0; x < w; x++) {
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(kHeatColors, heat_[x][y], 255));
    }
}

} // namespace ambient_matrix
