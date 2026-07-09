#include "ambient_matrix/effect_space.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/noise.h"
#include <cmath>

namespace ambient_matrix {
namespace {

static constexpr float kPi = 3.14159265f;

float clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

uint8_t to_byte(float value) {
    if (value <= 0.0f) return 0;
    if (value >= 255.0f) return 255;
    return (uint8_t)value;
}

uint8_t hash8(uint16_t value) {
    value ^= value >> 7;
    value *= 40503u;
    value ^= value >> 9;
    return (uint8_t)value;
}

void fill(Rgb* frame, uint16_t count, Rgb color) {
    for (uint16_t i = 0; i < count; i++) frame[i] = color;
}

void put(Rgb* frame, const Matrix& matrix, int16_t x, int16_t y, Rgb color,
         bool additive = true) {
    if (y < 0 || y >= matrix.height()) return;
    Rgb& pixel = frame[matrix.xy_wrap(x, (uint8_t)y)];
    pixel = additive ? add_rgb(pixel, color) : color;
}

void disc(Rgb* frame, const Matrix& matrix, float cx, float cy, float radius,
          Rgb color, bool additive = true) {
    const int16_t min_x = (int16_t)(cx - radius - 1.0f);
    const int16_t max_x = (int16_t)(cx + radius + 1.0f);
    const int16_t min_y = (int16_t)(cy - radius - 1.0f);
    const int16_t max_y = (int16_t)(cy + radius + 1.0f);
    const float limit = radius * radius;
    for (int16_t y = min_y; y <= max_y; y++) {
        for (int16_t x = min_x; x <= max_x; x++) {
            const float dx = x - cx;
            const float dy = y - cy;
            if (dx * dx + dy * dy <= limit)
                put(frame, matrix, x, y, color, additive);
        }
    }
}

void star_background(Rgb* frame, const Matrix& matrix, float t, Rgb background,
                     uint8_t density = 29) {
    fill(frame, matrix.width() * matrix.height(), background);
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const uint8_t seed = hash8((uint16_t)y * 37 + x * 19 + 11);
            if (seed % density != 0) continue;
            const float twinkle = 0.5f + 0.5f * std::sin(t * (1.1f + (seed & 7) * 0.13f)
                                                       + seed * 0.17f);
            const uint8_t value = to_byte(65.0f + twinkle * 175.0f);
            put(frame, matrix, x, y,
                Rgb::from_hsv((uint8_t)(145 + seed % 55), 80, value));
        }
    }
}

void render_nebula(Rgb* frame, const Matrix& matrix, uint32_t now_ms) {
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const uint8_t a = cylindrical_noise8(x, y, matrix.width(), 125,
                                                  (uint16_t)(now_ms / 10));
            const uint8_t b = cylindrical_noise8(matrix.wrap_x(x + 4), y,
                                                  matrix.width(), 83,
                                                  (uint16_t)(now_ms / 16));
            const uint8_t cloud = (uint8_t)(((uint16_t)a * 2 + b) / 3);
            const uint8_t hue = (uint8_t)(168 + cloud / 3 + now_ms / 100);
            const uint8_t value = cloud > 78 ? to_byte((cloud - 78) * 1.35f) : 5;
            frame[matrix.xy(x, y)] = Rgb::from_hsv(hue, 225, value);
        }
    }
}

void render_wormhole(Rgb* frame, const Matrix& matrix, float t) {
    const float cx = (matrix.width() - 1) * 0.5f;
    const float cy = (matrix.height() - 1) * 0.5f;
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float dx = matrix.shortest_x_delta(x, cx);
            const float dy = y - cy;
            const float radius = std::sqrt(dx * dx + dy * dy);
            const float angle = std::atan2(dy, dx);
            float stripe = 0.5f + 0.5f * std::sin(radius * 2.55f - t * 3.1f
                                                 + angle * 4.0f);
            stripe *= stripe;
            const float aperture = clamp01((radius - 1.25f) / 2.2f);
            const uint8_t value = to_byte(stripe * aperture * 255.0f);
            const uint8_t hue = (uint8_t)((angle + kPi) * (256.0f / (2.0f * kPi))
                                          + radius * 14.0f + t * 28.0f);
            frame[matrix.xy(x, y)] = Rgb::from_hsv(hue, 245, value);
        }
    }
}

void render_orbits(Rgb* frame, const Matrix& matrix, float t) {
    star_background(frame, matrix, t, {0, 0, 5}, 37);
    const float cx = 7.5f, cy = 7.5f;
    disc(frame, matrix, cx, cy, 1.45f, {255, 135, 20});
    static const float radii[3] = {3.3f, 5.0f, 6.7f};
    static const float speeds[3] = {1.3f, -0.78f, 0.46f};
    static const uint8_t hues[3] = {145, 12, 205};
    for (uint8_t i = 0; i < 3; i++) {
        const float angle = t * speeds[i] + i * 2.1f;
        const float px = cx + std::cos(angle) * radii[i];
        const float py = cy + std::sin(angle) * radii[i] * 0.58f;
        disc(frame, matrix, px, py, i == 1 ? 1.15f : 0.8f,
             Rgb::from_hsv(hues[i], 220, 245));
    }
}

void render_solar_storm(Rgb* frame, const Matrix& matrix, float t, uint32_t now_ms) {
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float angle = 2.0f * kPi * x / matrix.width();
            const float ribbon = std::sin(y * 0.82f + std::sin(angle * 2.0f + t) * 2.1f
                                          - t * 1.7f);
            const uint8_t noise = cylindrical_noise8(x, y, matrix.width(), 105,
                                                       (uint16_t)(now_ms / 5));
            float energy = clamp01((ribbon + 0.35f) * 0.78f);
            energy = clamp01(energy + (noise - 128) / 350.0f);
            const uint8_t hue = (uint8_t)(2 + energy * 38.0f);
            frame[matrix.xy(x, y)] = Rgb::from_hsv(hue, 245,
                                                   to_byte(12 + energy * 243));
        }
    }
}

void render_pulsar(Rgb* frame, const Matrix& matrix, float t) {
    const float cx = 7.5f, cy = 7.5f;
    const float pulse = 0.5f + 0.5f * std::sin(t * 4.2f);
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float dx = matrix.shortest_x_delta(x, cx), dy = y - cy;
            const float radius = std::sqrt(dx * dx + dy * dy);
            const float ring = clamp01(1.0f - std::fabs(radius - (1.5f + pulse * 5.5f)));
            const float beams = clamp01(1.0f - std::fabs(dx) / 1.1f)
                              + clamp01(1.0f - std::fabs(dy) / 1.1f);
            const float core = clamp01(1.0f - radius / (1.2f + pulse));
            const uint8_t value = to_byte(ring * 150 + beams * pulse * 75 + core * 255);
            frame[matrix.xy(x, y)] = Rgb::from_hsv((uint8_t)(145 + radius * 4),
                                                   165, value);
        }
    }
}

void render_starry_sky(Rgb* frame, const Matrix& matrix, float t) {
    star_background(frame, matrix, t, {1, 0, 9}, 11);
}

void render_asteroid(Rgb* frame, const Matrix& matrix, float t, uint32_t now_ms) {
    star_background(frame, matrix, t, {1, 0, 7}, 41);
    for (uint8_t x = 0; x < matrix.width(); x++) {
        put(frame, matrix, x, 0, {40, 8, 2}, false);
        put(frame, matrix, x, 1, {18, 3, 1}, false);
    }

    const float phase = (now_ms % 9000) / 9000.0f;
    if (phase < 0.55f) {
        const float p = phase / 0.55f;
        const float appear = clamp01(p * 8.0f);
        const float ax = 15.0f - p * 8.0f;
        const float ay = 15.0f - p * 13.0f;
        for (uint8_t i = 1; i < 8; i++) {
            const uint8_t value = to_byte((210 - i * 23) * appear);
            put(frame, matrix, (int16_t)(ax + i * 0.65f),
                (int16_t)(ay + i * 0.9f), Rgb::from_hsv((uint8_t)(18 - i), 250, value));
        }
        disc(frame, matrix, ax, ay, 1.15f,
             {to_byte(185 * appear), to_byte(150 * appear), to_byte(120 * appear)});
    } else {
        const float p = (phase - 0.55f) / 0.45f;
        const float radius = p * 13.0f;
        const float fade = 1.0f - p;
        for (uint8_t y = 0; y < matrix.height(); y++) {
            for (uint8_t x = 0; x < matrix.width(); x++) {
                const float dx = matrix.shortest_x_delta(x, 7.0f);
                const float dy = y - 2.0f;
                const float d = std::sqrt(dx * dx + dy * dy);
                const float ring = clamp01(1.0f - std::fabs(d - radius) / 1.7f);
                const float core = clamp01(1.0f - d / (2.2f + p * 4.0f));
                const uint8_t value = to_byte((ring * 230 + core * 255) * fade);
                put(frame, matrix, x, y,
                    Rgb::from_hsv((uint8_t)(8 + p * 25), 235, value));
            }
        }
    }
}

void render_pacman(Rgb* frame, const Matrix& matrix, float t, uint32_t now_ms) {
    fill(frame, matrix.width() * matrix.height(), {0, 0, 4});
    const float circumference = matrix.width();
    const float progress = std::fmod(now_ms / 180.0f, circumference);
    const float py = (matrix.height() - 1) * 0.5f;

    for (uint8_t slot = 0; slot < matrix.width(); slot++) {
        if (slot <= progress + 1.1f) continue;
        put(frame, matrix, slot, (int16_t)(py + 0.5f), {120, 45, 150});
    }

    const float px = progress;
    static constexpr int8_t dir_x = 1;
    static constexpr int8_t dir_y = 0;
    const float mouth = 0.18f + std::fabs(std::sin(t * 8.0f)) * 0.55f;
    for (int8_t oy = -2; oy <= 2; oy++) {
        for (int8_t ox = -2; ox <= 2; ox++) {
            if (ox * ox + oy * oy > 5) continue;
            const float forward = ox * dir_x + oy * dir_y;
            const float side = std::fabs(ox * dir_y - oy * dir_x);
            if (forward > 0.0f && side < forward * mouth) continue;
            put(frame, matrix, (int16_t)(px + ox + 0.5f),
                (int16_t)(py + oy + 0.5f), {255, 210, 0}, false);
        }
    }
}

void render_space_police(Rgb* frame, const Matrix& matrix, float t) {
    const float red_phase = 0.5f + 0.5f * std::sin(t * 6.2f);
    const float blue_phase = 1.0f - red_phase;
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float angle = 2.0f * kPi * x / matrix.width();
            const float sweep = 0.5f + 0.5f * std::sin(angle * 2.0f + y * 0.7f
                                                       - t * 5.0f);
            const float side = std::sin(angle);
            const float red = clamp01(side) * red_phase * (0.45f + sweep * 0.55f);
            const float blue = clamp01(-side) * blue_phase * (0.45f + sweep * 0.55f);
            frame[matrix.xy(x, y)] = {to_byte(red * 255),
                                      to_byte((red + blue) * 18),
                                      to_byte(blue * 255)};
        }
    }
}

void render_purple_meteors(Rgb* frame, const Matrix& matrix, float t) {
    fill(frame, matrix.width() * matrix.height(), {7, 0, 18});
    for (uint8_t m = 0; m < 8; m++) {
        const float progress = std::fmod(t * (0.18f + m * 0.013f) + m * 0.137f, 1.0f);
        const float head_x = 18.0f - progress * 25.0f + (m % 3) * 3.5f;
        const float head_y = 17.0f - progress * 22.0f + (m / 3) * 2.2f;
        for (uint8_t i = 0; i < 9; i++) {
            const uint8_t value = (uint8_t)(255 - i * 27);
            put(frame, matrix, (int16_t)(head_x + i * 0.72f),
                (int16_t)(head_y + i * 0.72f),
                Rgb::from_hsv((uint8_t)(190 + m * 5), i == 0 ? 80 : 225, value));
        }
    }
}

void render_light_speed(Rgb* frame, const Matrix& matrix, float t) {
    fill(frame, matrix.width() * matrix.height(), {0, 0, 5});
    const float cx = 7.5f, cy = 7.5f;
    for (uint8_t s = 0; s < 38; s++) {
        const uint8_t seed = hash8((uint16_t)s * 47 + 9);
        const float angle = seed * (2.0f * kPi / 255.0f);
        const float progress = std::fmod(t * (0.42f + (seed & 15) * 0.012f)
                                         + s * 0.071f, 1.0f);
        const float radius = progress * progress * 12.0f;
        for (uint8_t i = 0; i < 5; i++) {
            const float r = radius - i * (0.38f + progress * 0.45f);
            if (r < 0.0f) continue;
            put(frame, matrix, (int16_t)(cx + std::cos(angle) * r + 0.5f),
                (int16_t)(cy + std::sin(angle) * r + 0.5f),
                Rgb::from_hsv((uint8_t)(145 + seed % 65), 105,
                              (uint8_t)(255 - i * 48)));
        }
    }
}

void render_supernova(Rgb* frame, const Matrix& matrix, uint32_t now_ms) {
    fill(frame, matrix.width() * matrix.height(), {1, 0, 5});
    const float phase = (now_ms % 8500) / 8500.0f;
    const float cx = 7.5f, cy = 7.5f;
    if (phase < 0.2f) {
        const float p = phase / 0.2f;
        const float radius = 0.6f + p * 1.2f;
        disc(frame, matrix, cx, cy, radius,
             {to_byte(255 * p), to_byte(230 * p), to_byte(170 * p)});
        return;
    }

    const float p = (phase - 0.2f) / 0.8f;
    const float radius = p * 12.0f;
    const float fade = 1.0f - p;
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float dx = matrix.shortest_x_delta(x, cx), dy = y - cy;
            const float d = std::sqrt(dx * dx + dy * dy);
            const float shell = clamp01(1.0f - std::fabs(d - radius) / 2.0f);
            const float core = clamp01(1.0f - d / (1.5f + p * 5.0f));
            const uint8_t value = to_byte((shell * 245 + core * 255) * fade);
            put(frame, matrix, x, y,
                Rgb::from_hsv((uint8_t)(18 + p * 205), 205, value));
        }
    }
}

void render_mars_flight(Rgb* frame, const Matrix& matrix, float t, uint32_t now_ms) {
    render_light_speed(frame, matrix, t * 0.7f);
    const float phase = (now_ms % 12000) / 12000.0f;
    const float approach = phase < 0.8f ? phase / 0.8f : 1.0f;
    const float fade = phase < 0.8f ? 1.0f : (1.0f - phase) / 0.2f;
    const float radius = 1.0f + approach * 5.7f;
    const float cx = 9.5f, cy = 7.0f;
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float dx = matrix.shortest_x_delta(x, cx), dy = y - cy;
            if (dx * dx + dy * dy > radius * radius) continue;
            const uint8_t terrain = cylindrical_noise8(x, y, matrix.width(), 181,
                                                        (uint16_t)(now_ms / 30));
            const uint8_t value = to_byte((120 + terrain / 2) * fade);
            put(frame, matrix, x, y,
                Rgb::from_hsv((uint8_t)(4 + terrain / 32), 235, value), false);
        }
    }
}

void render_singularity(Rgb* frame, const Matrix& matrix, float t) {
    const float cx = 7.5f, cy = 7.5f;
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const float dx = matrix.shortest_x_delta(x, cx);
            const float dy = (y - cy) * 1.85f;
            const float radius = std::sqrt(dx * dx + dy * dy);
            const float angle = std::atan2(dy, dx);
            const float disk = clamp01(1.0f - std::fabs(radius - 5.2f) / 2.7f);
            const float swirl = 0.5f + 0.5f * std::sin(angle * 5.0f - t * 4.0f
                                                      + radius * 1.2f);
            const float lens = clamp01(1.0f - std::fabs(radius - 2.8f) / 0.75f);
            const float hole = clamp01((radius - 1.9f) / 0.7f);
            const uint8_t value = to_byte((disk * swirl * 235 + lens * 180) * hole);
            const uint8_t hue = (uint8_t)(12 + swirl * 195.0f + t * 10.0f);
            frame[matrix.xy(x, y)] = Rgb::from_hsv(hue, 235, value);
        }
    }
}

void render_relic(Rgb* frame, const Matrix& matrix, uint32_t now_ms) {
    for (uint8_t y = 0; y < matrix.height(); y++) {
        for (uint8_t x = 0; x < matrix.width(); x++) {
            const uint8_t broad = cylindrical_noise8(x, y, matrix.width(), 105,
                                                       (uint16_t)(now_ms / 35));
            const uint8_t fine = cylindrical_noise8(matrix.wrap_x(x + 3), y,
                                                      matrix.width(), 241,
                                                      (uint16_t)(now_ms / 28));
            const uint8_t signal = (uint8_t)(((uint16_t)broad * 3 + fine) / 4);
            const uint8_t hue = signal < 128
                ? (uint8_t)(145 + signal / 4)
                : (uint8_t)((signal - 128) / 3);
            const uint8_t value = (uint8_t)(42 + std::fabs((int)signal - 128) * 1.25f);
            frame[matrix.xy(x, y)] = Rgb::from_hsv(hue, 205, value);
        }
    }
}

} // namespace

void EffectSpace::tick(MatrixCanvas& canvas, const Matrix& matrix,
                       const EffectParams&, uint32_t now_ms) {
    const FrameInfo frame_time = clock_.tick(now_ms);
    if (matrix.width() == 0 || matrix.height() == 0 || canvas.size() > 256) return;
    Rgb frame[256]{};
    const float t = frame_time.total_s();

    switch (style_) {
        case SpaceStyle::CosmicNebula: render_nebula(frame, matrix, frame_time.total_ms); break;
        case SpaceStyle::Wormhole: render_wormhole(frame, matrix, t); break;
        case SpaceStyle::OrbitalDance: render_orbits(frame, matrix, t); break;
        case SpaceStyle::SolarStorm: render_solar_storm(frame, matrix, t, frame_time.total_ms); break;
        case SpaceStyle::Pulsar: render_pulsar(frame, matrix, t); break;
        case SpaceStyle::StarrySky: render_starry_sky(frame, matrix, t); break;
        case SpaceStyle::AsteroidImpact: render_asteroid(frame, matrix, t, frame_time.total_ms); break;
        case SpaceStyle::PacmanOrbit: render_pacman(frame, matrix, t, frame_time.total_ms); break;
        case SpaceStyle::SpacePolice: render_space_police(frame, matrix, t); break;
        case SpaceStyle::PurpleMeteors: render_purple_meteors(frame, matrix, t); break;
        case SpaceStyle::LightSpeed: render_light_speed(frame, matrix, t); break;
        case SpaceStyle::Supernova: render_supernova(frame, matrix, frame_time.total_ms); break;
        case SpaceStyle::MarsFlight: render_mars_flight(frame, matrix, t, frame_time.total_ms); break;
        case SpaceStyle::Singularity: render_singularity(frame, matrix, t); break;
        case SpaceStyle::RelicRadiation: render_relic(frame, matrix, frame_time.total_ms); break;
    }

    for (uint16_t i = 0; i < canvas.size(); i++) canvas.set_pixel(i, frame[i]);
}

} // namespace ambient_matrix
