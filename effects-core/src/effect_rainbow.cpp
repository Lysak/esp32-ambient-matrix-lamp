#include "ambient_matrix/effect_rainbow.h"

namespace ambient_matrix {

void EffectRainbow::reset() {
    clock_.reset();
    phase_.reset();
}

void EffectRainbow::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams& params, uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    phase_.advance_linear8(frame, params.speed, 1280);
    const uint8_t hue_offset = phase_.byte();

    // scale controls how spread the hue gradient is across the matrix
    uint8_t spread = params.scale ? params.scale : 1;

    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            uint8_t hue = (uint8_t)((uint16_t)x * 256 / matrix.width()
                          + (uint16_t)y * spread / matrix.height()) + hue_offset;
            canvas.set_pixel(matrix.xy(x, y), Rgb::from_hsv(hue, 255, 255));
        }
    }
}

} // namespace ambient_matrix
