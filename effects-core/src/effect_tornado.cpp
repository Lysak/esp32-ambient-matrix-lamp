// Perlin-noise driven spiral tornado effect.
// Based on case 9 (TORNADO) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_tornado.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"

namespace ambient_matrix {

void EffectTornado::tick(MatrixCanvas& canvas, const Matrix& matrix,
                         const EffectParams& params, uint32_t now_ms) {
    uint8_t w = matrix.width(), h = matrix.height();
    uint16_t t1 = (uint16_t)((now_ms >> 3) * params.speed >> 8);
    uint16_t t2 = (uint16_t)((now_ms >> 1) * params.speed >> 8);
    uint8_t  layers = (params.scale >> 5) + 1;  // 1-8

    // Local additive buffer so layers blend together (zero-initialized)
    Rgb buf[256]{};

    for (uint8_t k = 0; k < layers; k++) {
        for (uint8_t y = 0; y < h; y++) {
            uint8_t noise_val = inoise8((uint16_t)y * 10 + t1 + (uint16_t)k * 10000 / 256, t2);
            int16_t col       = map_int(noise_val, 50, 200, 0, (int16_t)w);

            // Draw a 4-pixel wide stripe with brightness falloff from center
            static const uint8_t kWidths = 4;
            for (uint8_t j = 0; j < kWidths; j++) {
                uint8_t bright = (j == 0) ? 255 : (kWidths - j) * 255 / (kWidths - 1);
                uint8_t hue_idx = (uint8_t)((uint16_t)j * 255 / kWidths);
                Rgb c = color_from_palette(kRainbowColors, hue_idx, bright);

                auto place = [&](int16_t cx) {
                    Rgb& px = buf[matrix.xy_wrap(cx, y)];
                    px = add_rgb(px, c);
                };
                if (j == 0) {
                    place(col);
                } else {
                    place(col - j);
                    place(col + j);
                }
            }
        }
    }

    uint16_t n = (uint16_t)w * h;
    for (uint16_t i = 0; i < n; i++) canvas.set_pixel(i, buf[i]);
}

} // namespace ambient_matrix
