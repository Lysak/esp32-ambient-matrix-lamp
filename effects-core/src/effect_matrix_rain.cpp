#include "ambient_matrix/effect_matrix_rain.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

void EffectMatrixRain::reset() {
    last_ms_ = 0;
    initialized_ = false;
}

void EffectMatrixRain::tick(MatrixCanvas& canvas, const Matrix& matrix,
                            const EffectParams&, uint32_t now_ms) {
    uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w > kMaxColumns) w = kMaxColumns;
    if (w == 0 || h == 0) return;

    if (!initialized_) {
        for (uint8_t x = 0; x < w; x++) {
            heads_[x] = h + (float)random8((uint8_t)(h + 1));
            speeds_[x] = 2.2f + random8(180) / 100.0f;
        }
        initialized_ = true;
        last_ms_ = now_ms;
    }

    float dt = (now_ms - last_ms_) * 0.001f;
    if (dt > 0.08f) dt = 0.08f;
    last_ms_ = now_ms;

    static constexpr float kTrail = 7.0f;
    for (uint8_t x = 0; x < w; x++) {
        heads_[x] -= speeds_[x] * dt;
        if (heads_[x] < -kTrail) {
            heads_[x] = h + (float)random8((uint8_t)(h + 1));
            speeds_[x] = 2.2f + random8(180) / 100.0f;
        }

        for (uint8_t y = 0; y < h; y++) {
            const float distance = y - heads_[x];
            uint8_t value = 0;
            if (distance >= -0.65f && distance < kTrail) {
                float fade = 1.0f - (distance < 0.0f ? 0.0f : distance) / kTrail;
                fade *= fade;
                value = (uint8_t)(fade * 245.0f);
            }
            const uint8_t hue = (uint8_t)(88 + x * 2);
            canvas.set_pixel(matrix.xy(x, y), Rgb::from_hsv(hue, 245, value));
        }
    }
}

} // namespace ambient_matrix
