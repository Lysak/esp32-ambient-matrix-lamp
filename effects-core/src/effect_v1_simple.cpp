// Based on colorWheel, rainbowVertical, rainbowHorizontal from GyverLamp v1 effects.ino
#include "ambient_matrix/effect_v1_simple.h"
#include "ambient_matrix/types.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

void EffectColorChange::tick(MatrixCanvas& canvas, const Matrix& matrix,
                              const EffectParams& params, uint32_t) {
    Rgb c = Rgb::from_hsv(hue_, 255, 255);
    uint8_t w = matrix.width(), h = matrix.height();
    for (uint8_t x = 0; x < w; x++)
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), c);
    hue_ = (uint8_t)(hue_ + (params.speed >> 5) + 1);
}

void EffectRainbowVert::tick(MatrixCanvas& canvas, const Matrix& matrix,
                              const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    uint8_t spread = params.scale ? params.scale : 16;
    for (uint8_t x = 0; x < w; x++) {
        Rgb c = Rgb::from_hsv((uint8_t)(offset_ + (uint16_t)x * spread / w), 255, 255);
        for (uint8_t y = 0; y < h; y++)
            canvas.set_pixel(matrix.xy(x, y), c);
    }
    offset_ = (uint8_t)(offset_ + (params.speed >> 5) + 1);
}

void EffectRainbowHoriz::tick(MatrixCanvas& canvas, const Matrix& matrix,
                               const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    uint8_t spread = params.scale ? params.scale : 16;
    for (uint8_t y = 0; y < h; y++) {
        Rgb c = Rgb::from_hsv((uint8_t)(offset_ + (uint16_t)y * spread / h), 255, 255);
        for (uint8_t x = 0; x < w; x++)
            canvas.set_pixel(matrix.xy(x, y), c);
    }
    offset_ = (uint8_t)(offset_ + (params.speed >> 5) + 1);
}

} // namespace ambient_matrix
