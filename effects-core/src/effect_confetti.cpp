// Random colored sparks that fade over time.
// Based on case 8 (CONFETTI) from GyverLamp2/effects.ino.
// Source: https://github.com/AlexGyver/GyverLamp2

#include "ambient_matrix/effect_confetti.h"
#include "ambient_matrix/effect_color_source.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include <cstring>

namespace ambient_matrix {

void EffectConfetti::reset() {
    clock_.reset();
    stepper_.reset();
    memset(buf_, 0, sizeof(buf_));
}

void EffectConfetti::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                          uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    const uint8_t steps = stepper_.consume(frame, 6);
    uint16_t n = (uint16_t)matrix.width() * matrix.height();
    uint8_t amount = (params.scale >> 3) + 1;
    uint8_t fade = params.speed / 2 + 1;
    const Palette16& palette = palette_by_id(params.palette);

    for (uint8_t step = 0; step < steps; step++) {
        // Fade all pixels first (matches FastLED fadeToBlackBy)
        for (uint16_t i = 0; i < n; i++)
            fade_to_black(buf_[i], fade);

        // Place new sparks only on exactly-black pixels (GyverLamp2: leds[x] == CRGB(0,0,0))
        for (uint8_t i = 0; i < amount; i++) {
            uint16_t pos = (uint32_t)random8() * n >> 8;
            Rgb& px = buf_[pos];
            if (px.r == 0 && px.g == 0 && px.b == 0)
                px = indexed_effect_color(params, palette, i * 255 / amount);
        }
    }

    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, buf_[i]);
}

} // namespace ambient_matrix
