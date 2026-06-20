#include "ambient_matrix/effect_starfield.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

void EffectStarfield::spawn(Star& star, bool initial) {
    star.x = ((int16_t)random8() - 128) / 255.0f;
    star.y = ((int16_t)random8() - 128) / 255.0f;
    star.z = initial ? 0.2f + random8(205) / 255.0f : 1.0f;
    star.speed = 0.16f + random8(20) / 100.0f;
    star.hue = (uint8_t)(145 + random8(55));
}

void EffectStarfield::reset() {
    last_ms_ = 0;
    initialized_ = false;
}

void EffectStarfield::tick(MatrixCanvas& canvas, const Matrix& matrix,
                           const EffectParams&, uint32_t now_ms) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0 || canvas.size() > 256) return;

    if (!initialized_) {
        for (auto& star : stars_) spawn(star, true);
        initialized_ = true;
        last_ms_ = now_ms;
    }

    float dt = (now_ms - last_ms_) * 0.001f;
    if (dt > 0.08f) dt = 0.08f;
    last_ms_ = now_ms;

    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;
    const float scale = (w < h ? w : h) * 0.48f;
    Rgb frame[256]{};

    for (auto& star : stars_) {
        star.z -= star.speed * dt;
        if (star.z < 0.08f) spawn(star, false);

        const float px = cx + star.x / star.z * scale;
        const float py = cy + star.y / star.z * scale;
        if (py < 0.0f || py >= h) {
            spawn(star, false);
            continue;
        }

        const uint8_t value = (uint8_t)(70.0f + (1.0f - star.z) * 185.0f);
        const uint16_t index = matrix.xy_wrap((int16_t)px, (uint8_t)py);
        frame[index] = add_rgb(frame[index],
                               Rgb::from_hsv(star.hue, 105, value));

        const float trail_z = star.z + 0.055f;
        const float tx = cx + star.x / trail_z * scale;
        const float ty = cy + star.y / trail_z * scale;
        if (ty >= 0.0f && ty < h) {
            const uint16_t trail_index = matrix.xy_wrap((int16_t)tx, (uint8_t)ty);
            frame[trail_index] = add_rgb(frame[trail_index],
                                         Rgb::from_hsv(star.hue, 180,
                                                       value / 3));
        }
    }

    for (uint16_t i = 0; i < canvas.size(); i++) canvas.set_pixel(i, frame[i]);
}

} // namespace ambient_matrix
