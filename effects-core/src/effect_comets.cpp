#include "ambient_matrix/effect_comets.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

void EffectComets::reset() {
    clock_.reset();
    initialized_ = false;
}

void EffectComets::tick(MatrixCanvas& canvas, const Matrix& matrix,
                        const EffectParams&, uint32_t now_ms) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0 || canvas.size() > 256) return;

    if (!initialized_) {
        for (auto& comet : comets_) {
            comet.x = random8((uint8_t)(w + 8));
            comet.y = random8((uint8_t)(h + 8));
            comet.vx = -(2.8f + random8(170) / 100.0f);
            comet.vy = -(2.0f + random8(150) / 100.0f);
            comet.hue = random8();
        }
        initialized_ = true;
    }

    const FrameInfo frame_info = clock_.tick(now_ms);
    const float dt = frame_info.delta_s();

    Rgb frame_buf[256]{};
    for (auto& comet : comets_) {
        comet.x += comet.vx * dt;
        comet.y += comet.vy * dt;
        while (comet.x < 0.0f) comet.x += w;
        while (comet.x >= w) comet.x -= w;
        if (comet.y < -5.0f) {
            comet.x = random8(w);
            comet.y = h + random8((uint8_t)(h + 1));
            comet.vx = -(2.8f + random8(170) / 100.0f);
            comet.vy = -(2.0f + random8(150) / 100.0f);
            comet.hue = random8();
        }

        for (uint8_t i = 0; i < 11; i++) {
            const float age = i * 0.12f;
            const int16_t px = (int16_t)(comet.x - comet.vx * age + 0.5f);
            const int16_t py = (int16_t)(comet.y - comet.vy * age + 0.5f);
            if (py < 0 || py >= h) continue;
            const uint8_t fade = (uint8_t)(255 - i * 22);
            const Rgb color = Rgb::from_hsv((uint8_t)(comet.hue + i * 2),
                                            i == 0 ? 80 : 235, fade);
            const uint16_t index = matrix.xy_wrap(px, (uint8_t)py);
            frame_buf[index] = add_rgb(frame_buf[index], color);
        }
    }

    for (uint16_t i = 0; i < canvas.size(); i++) canvas.set_pixel(i, frame_buf[i]);
}

} // namespace ambient_matrix
