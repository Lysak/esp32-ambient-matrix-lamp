// Random colored sparks that fade over time.
// Based on case 8 (CONFETTI) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_confetti.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectConfetti::reset() {
    memset(buf_, 0, sizeof(buf_));
}

void EffectConfetti::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t) {
    uint16_t n      = (uint16_t)matrix.width() * matrix.height();
    uint8_t  amount = (params.scale >> 3) + 1;
    uint8_t  fade   = params.speed / 2 + 1;

    // Spawn new sparks on dark pixels
    for (uint8_t i = 0; i < amount; i++) {
        uint16_t pos = (uint32_t)random8() * n >> 8;
        Rgb& px = buf_[pos];
        if (px.r < 10 && px.g < 10 && px.b < 10)
            px = color_from_palette(kRainbowColors, i * 255 / amount, 255);
    }

    // Fade and write to canvas
    for (uint16_t i = 0; i < n; i++) {
        Rgb& px = buf_[i];
        if (px.r >= 10 || px.g >= 10 || px.b >= 10)
            fade_to_black(px, fade);
        else
            px = {};
        canvas.set_pixel(i, px);
    }
}

} // namespace ambient_matrix
