// Scrolling palette gradient along matrix height.
// Based on case 4 (GRADIENT) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_gradient.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectGradient::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t now_ms) {
    // speed > 128 scrolls up, < 128 scrolls down
    int8_t  dir    = (int8_t)params.speed - 128;
    uint8_t offset = (uint8_t)((int32_t)(now_ms >> 3) * dir >> 7);
    uint8_t h = matrix.height(), w = matrix.width();
    const Palette16& palette = palette_by_id(params.palette);

    for (uint8_t y = 0; y < h; y++) {
        // scale controls gradient spread: higher scale = more color cycles
        uint8_t idx = (uint8_t)((uint16_t)y * (params.scale + 1) / h + offset);
        Rgb c = color_from_palette(palette, idx, 255);
        for (uint8_t x = 0; x < w; x++) canvas.set_pixel(matrix.xy(x, y), c);
    }
}

} // namespace ambient_matrix
