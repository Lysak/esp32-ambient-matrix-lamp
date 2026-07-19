// Solid color fill.
// Based on case 2 (COLOR) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_color.h"

namespace ambient_matrix {

void EffectColor::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                       uint32_t) {
    // scale controls lit portion: scale=255 → all pixels bright, scale=0 → all dim
    uint16_t n = (uint16_t)matrix.width() * matrix.height();
    uint16_t led_count = (uint16_t)params.scale * n / 255;
    Rgb bright = Rgb::from_hsv(params.color, 255, 255);
    Rgb dim = Rgb::from_hsv(params.color, 255, 30);
    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, i < led_count ? bright : dim);
}

} // namespace ambient_matrix
