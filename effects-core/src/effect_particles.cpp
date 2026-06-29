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
    memset(buf_, 0, sizeof(buf_));
}

void EffectParticles::tick(MatrixCanvas& canvas, const Matrix& matrix,
                           const EffectParams& params, uint32_t now_ms) {
    uint8_t  w      = matrix.width(), h = matrix.height();
    uint16_t n      = (uint16_t)w * h;
    uint8_t  amount = (params.scale >> 3) + 1;
    uint32_t t      = (uint32_t)(now_ms << 3) * params.speed >> 8;
    const Palette16& palette = palette_by_id(params.palette);

    // Fade all pixels
    for (uint16_t i = 0; i < n; i++) fade_to_black(buf_[i], 70);

    // Place each particle at its noise-driven position
    for (uint8_t i = 0; i < amount; i++) {
        // Home position: slow Perlin drift (scale to [0, h) and [0, w))
        int16_t hx = (int16_t)((uint32_t)(inoise16((uint32_t)i * 100000UL + t) >> 8) * h >> 8);
        int16_t hy = (int16_t)((uint32_t)(inoise16((uint32_t)i * 100000UL + 2000000UL + t) >> 8) * w >> 8);

        // Local offset: faster Perlin jitter
        int16_t ox = (int16_t)inoise8((uint16_t)(i * 2500 + (now_ms >> 1) * params.speed / 255)) - 128;
        int16_t oy = (int16_t)inoise8((uint16_t)(i * 2500 + 30000 + (now_ms >> 1) * params.speed / 255)) - 128;
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
