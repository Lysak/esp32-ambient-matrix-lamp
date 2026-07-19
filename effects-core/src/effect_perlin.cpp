// 2D Perlin/value noise plasma mapped to a palette.
// Based on case 1 (PERLIN) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_perlin.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectPerlin::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                        uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    phase_.advance_linear16(frame, params.speed, 2048);
    // step = scale/5: matches GyverLamp2's original spatial scale formula
    uint8_t step = params.scale / 5;
    const uint8_t t = phase_.byte();

    uint8_t w = matrix.width(), h = matrix.height();
    int8_t cx = (int8_t)(w / 2), cy = (int8_t)(h / 2);
    const Palette16& palette = palette_by_id(params.palette);
    for (uint8_t y = 0; y < h; y++) {
        for (uint8_t x = 0; x < w; x++) {
            // centered coordinates match GyverLamp2's x*step-CENTER_X, y*step-CENTER_Y
            uint8_t n =
                inoise8((uint16_t)((int16_t)x * step - cx), (uint16_t)((int16_t)y * step - cy), t);
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(palette, n, 255));
        }
    }
}

} // namespace ambient_matrix
