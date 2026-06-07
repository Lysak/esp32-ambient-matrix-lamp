// 2D Perlin/value noise plasma mapped to a palette.
// Based on case 1 (PERLIN) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_perlin.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectPerlin::tick(MatrixCanvas& canvas, const Matrix& matrix,
                        const EffectParams& params, uint32_t now_ms) {
    // step controls spatial zoom: larger step = coarser noise
    uint8_t  step = (params.scale >> 2) + 4;
    uint16_t t    = (uint16_t)((now_ms >> 3) * params.speed >> 8);

    uint8_t w = matrix.width(), h = matrix.height();
    for (uint8_t y = 0; y < h; y++) {
        for (uint8_t x = 0; x < w; x++) {
            uint8_t n = inoise8((uint16_t)x * step, (uint16_t)y * step, t);
            canvas.set_pixel(matrix.xy(x, y), color_from_palette(kRainbowColors, n, 255));
        }
    }
}

} // namespace ambient_matrix
