// Palette-colored particles on Perlin-noise trajectories with fade trail.
// Based on case 5 (PARTICLES) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_particles.h"
#include "ambient_matrix/effect_color_source.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectParticles::reset() {
    clock_.reset();
    drift_phase_.reset();
    jitter_phase_.reset();
    memset(buf_, 0, sizeof(buf_));
}

void EffectParticles::tick(MatrixCanvas& canvas, const Matrix& matrix,
                           const EffectParams& params, uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    uint8_t  w      = matrix.width(), h = matrix.height();
    uint16_t n      = (uint16_t)w * h;
    uint8_t  amount = (params.scale >> 3) + 1;
    drift_phase_.advance_linear16(frame, params.speed, 32);
    jitter_phase_.advance_linear16(frame, params.speed, 510);
    const uint32_t t = drift_phase_.word();
    const uint16_t jitter = jitter_phase_.word();
    const Palette16& palette = palette_by_id(params.palette);

    // Fade all pixels
    uint8_t fade = static_cast<uint8_t>((70u * (frame.delta_ms ? frame.delta_ms : 30u)) / 30u);
    if (fade == 0) fade = 1;
    for (uint16_t i = 0; i < n; i++) fade_to_black(buf_[i], fade);

    // Place each particle at its noise-driven position
    for (uint8_t i = 0; i < amount; i++) {
        // Home position: slow Perlin drift (scale to [0, h) and [0, w))
        int16_t hx = (int16_t)((uint32_t)(inoise16((uint32_t)i * 100000UL + t) >> 8) * h >> 8);
        int16_t hy = (int16_t)((uint32_t)(inoise16((uint32_t)i * 100000UL + 2000000UL + t) >> 8) * w >> 8);

        // Local offset: faster Perlin jitter
        int16_t ox = (int16_t)inoise8((uint16_t)(i * 2500 + jitter)) - 128;
        int16_t oy = (int16_t)inoise8((uint16_t)(i * 2500 + 30000 + jitter)) - 128;
        ox = (int16_t)h / 2 * ox / 128;
        oy = (int16_t)w / 2 * oy / 128;

        int16_t px = hx + ox;
        int16_t py = hy + oy;

        if (px >= 0 && px < (int16_t)h) {
            uint16_t idx = matrix.xy_wrap(py, (uint8_t)px);
            buf_[idx] = indexed_effect_color(params, palette, i * 255 / amount);
        }
    }

    for (uint16_t i = 0; i < n; i++) canvas.set_pixel(i, buf_[i]);
}

} // namespace ambient_matrix
