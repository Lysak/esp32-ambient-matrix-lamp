// Classic 2D heat-diffusion fire.
// Based on case 6 (FIRE) from GyverLamp2/effects.ino and FastLED Fire2012.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_fire.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectFire::reset() {
    memset(heat_, 0, sizeof(heat_));
}

void EffectFire::tick(MatrixCanvas& canvas, const Matrix& matrix,
                      const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width();
    uint8_t h = matrix.height();
    if (w > 16) w = 16;
    if (h > 16) h = 16;

    // cooling: scale 0 = slow cooling (tall flames), scale 255 = fast (short)
    uint8_t cooling = 2 + (255 - params.scale) / 16;
    // sparking: speed controls spark frequency
    uint8_t sparking = 80 + params.speed / 3;

    for (uint8_t x = 0; x < w; x++) {
        // Step 1: Cool every cell
        for (uint8_t y = 0; y < h; y++)
            heat_[x][y] = qsub8(heat_[x][y], random8(0, cooling));

        // Step 2: Diffuse heat upward
        for (uint8_t y = h - 1; y >= 2; y--)
            heat_[x][y] = ((uint16_t)heat_[x][y-1] + heat_[x][y-2] + heat_[x][y-2]) / 3;

        // Step 3: Spark at bottom
        if (random8() < sparking) {
            uint8_t y = random8(3);
            heat_[x][y] = qadd8(heat_[x][y], random8(160, 255));
        }

        // Step 4: Map heat to color and write to canvas
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(kHeatColors, heat_[x][y], 255));
    }
}

} // namespace ambient_matrix
