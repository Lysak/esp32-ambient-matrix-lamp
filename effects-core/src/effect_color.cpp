// Solid color fill.
// Based on case 2 (COLOR) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_color.h"

namespace ambient_matrix {

void EffectColor::tick(MatrixCanvas& canvas, const Matrix& matrix,
                       const EffectParams& params, uint32_t) {
    // color = hue, scale = saturation
    Rgb c = Rgb::from_hsv(params.color, params.scale, 255);
    uint16_t n = (uint16_t)matrix.width() * matrix.height();
    for (uint16_t i = 0; i < n; i++) canvas.set_pixel(i, c);
}

} // namespace ambient_matrix
