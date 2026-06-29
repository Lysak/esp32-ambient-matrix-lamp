// Slowly cycling solid color using palette interpolation.
// Based on case 3 (COLOR SHIFT) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_color_shift.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectColorShift::tick(MatrixCanvas& canvas, const Matrix& matrix,
                            const EffectParams& params, uint32_t now_ms) {
    uint8_t idx = (uint8_t)((now_ms >> 5) * params.speed >> 8);
    Rgb c = color_from_palette(palette_by_id(params.palette), idx, 255);
    uint16_t n = (uint16_t)matrix.width() * matrix.height();
    for (uint16_t i = 0; i < n; i++) canvas.set_pixel(i, c);
}

} // namespace ambient_matrix
