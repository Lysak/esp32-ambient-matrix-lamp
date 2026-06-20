#include "ambient_matrix/effect_scanner.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/noise.h"
#include <cmath>

namespace ambient_matrix {
namespace {

float clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

} // namespace

void EffectScanner::tick(MatrixCanvas& canvas, const Matrix& matrix,
                         const EffectParams&, uint32_t now_ms) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0) return;

    // Cubic easing reaches zero velocity at both ends, avoiding a hard bounce.
    static constexpr uint32_t kCycleMs = 4800;
    static constexpr uint32_t kHalfCycleMs = kCycleMs / 2;
    const uint32_t phase = now_ms % kCycleMs;
    const float t = (float)(phase % kHalfCycleMs) / (float)kHalfCycleMs;
    const float eased = t * t * (3.0f - 2.0f * t);
    const float normalized_y = phase < kHalfCycleMs ? 1.0f - eased : eased;
    const float center_y = normalized_y * (float)(h - 1);

    for (uint8_t x = 0; x < w; x++) {
        static constexpr float kTau = 6.28318531f;
        const float angle = kTau * x / w;
        const uint8_t texture = (uint8_t)(225 + 30
            * (0.5f + 0.5f * std::sin(angle * 3.0f + now_ms * 0.0015f)));
        const uint8_t hue = blue_only_
            ? (uint8_t)(160 + std::sin(angle + now_ms * 0.001f) * 12.0f)
            : (uint8_t)(now_ms / 18 + (uint16_t)x * 256 / w);

        for (uint8_t y = 0; y < h; y++) {
            const float distance = y > center_y ? y - center_y : center_y - y;
            float halo = clamp01(1.0f - distance / 4.5f);
            float core = clamp01(1.0f - distance / 1.25f);
            halo *= halo;
            core *= core;
            const uint8_t beam = (uint8_t)(halo * 112.0f + core * 143.0f);
            const uint8_t brightness = scale8(beam, texture);
            const uint8_t saturation = blue_only_ ? 235 : 250;
            canvas.set_pixel(matrix.xy(x, y),
                             Rgb::from_hsv(hue, saturation, brightness));
        }
    }
}

} // namespace ambient_matrix
