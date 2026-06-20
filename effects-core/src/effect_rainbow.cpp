#include "ambient_matrix/effect_rainbow.h"

namespace ambient_matrix {

void EffectRainbow::reset() {
    last_ms_    = 0;
    hue_offset_ = 0;
}

void EffectRainbow::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t now_ms) {
    // speed 0 = very slow (~255ms/step), speed 255 = very fast (~5ms/step)
    uint32_t step_ms = 5 + (uint32_t)(255 - params.speed) / 4;
    if (now_ms - last_ms_ >= step_ms) {
        last_ms_ = now_ms;
        hue_offset_++;
    }

    // scale controls how spread the hue gradient is across the matrix
    uint8_t spread = params.scale ? params.scale : 1;

    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            uint8_t hue = (uint8_t)((uint16_t)x * 256 / matrix.width()
                          + (uint16_t)y * spread / matrix.height()) + hue_offset_;
            canvas.set_pixel(matrix.xy(x, y), Rgb::from_hsv(hue, 255, 255));
        }
    }
}

} // namespace ambient_matrix
