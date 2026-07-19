// Scrolling palette gradient along matrix height.
// Based on case 4 (GRADIENT) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_gradient.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectGradient::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                          uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    phase_.advance_centered8(frame, params.speed, 1024);
    const uint8_t offset = phase_.byte();
    uint8_t h = matrix.height(), w = matrix.width();
    const Palette16& palette = palette_by_id(params.palette);

    // spread: scale=0 → ~10% of palette cycle, scale=255 → ~2 full cycles
    uint8_t spread = (uint8_t)((uint16_t)params.scale * 19 / 10 + 25);
    uint16_t n = (uint16_t)w * h;
    for (uint16_t i = 0; i < n; i++) {
        uint8_t idx = (uint8_t)((uint32_t)i * spread / n + offset);
        canvas.set_pixel(i, color_from_palette(palette, idx, 255));
    }
}

} // namespace ambient_matrix
