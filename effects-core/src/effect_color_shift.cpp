// Slowly cycling solid color using palette interpolation.
// Based on case 3 (COLOR SHIFT) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_color_shift.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectColorShift::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                            uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    phase_.advance_linear8(frame, params.speed, 8192);
    const uint8_t idx = phase_.byte();
    uint16_t n = (uint16_t)matrix.width() * matrix.height();
    uint16_t led_count = (uint16_t)params.scale * n / 255;
    Rgb bright = color_from_palette(palette_by_id(params.palette), idx, 255);
    Rgb dim = color_from_palette(palette_by_id(params.palette), idx, 30);
    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, i < led_count ? bright : dim);
}

} // namespace ambient_matrix
