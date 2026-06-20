#include "ambient_matrix/effect_patterns.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/palette.h"
#include <cmath>

namespace ambient_matrix {
namespace {

static constexpr float kTau = 6.28318531f;

float clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

uint8_t byte_from_float(float value) {
    if (value <= 0.0f) return 0;
    if (value >= 255.0f) return 255;
    return (uint8_t)value;
}

} // namespace

void EffectPatterns::tick(MatrixCanvas& canvas, const Matrix& matrix,
                          const EffectParams&, uint32_t now_ms) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0) return;

    const float t = now_ms * 0.001f;
    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;

    for (uint8_t y = 0; y < h; y++) {
        for (uint8_t x = 0; x < w; x++) {
            Rgb color;
            const float angle = kTau * x / w;
            const float cylinder_x = matrix.shortest_x_delta(x, cx);

            switch (style_) {
                case PatternStyle::Plasma: {
                    const float wave = std::sin(angle * 2.0f + t * 1.25f)
                                     + std::sin(y * 0.61f - t * 0.92f)
                                     + std::sin(angle * 3.0f + y * 0.34f + t * 0.58f);
                    const uint8_t hue = (uint8_t)((int)(wave * 30.0f)
                                                + (int)(now_ms / 20));
                    const uint8_t value = byte_from_float(185.0f
                        + 55.0f * std::sin(wave + t * 0.4f));
                    color = Rgb::from_hsv(hue, 245, value);
                    break;
                }

                case PatternStyle::Aurora: {
                    const float drift = ((int)cylindrical_noise8(x, y, w, 170,
                                                                 (uint16_t)(now_ms / 5)) - 128)
                                      / 70.0f;
                    const float curtain = cy + std::sin(angle * 2.0f + t * 0.62f) * 2.2f
                                           + drift;
                    const float distance = std::fabs(y - curtain);
                    float glow = clamp01(1.0f - distance / 5.2f);
                    glow *= glow;
                    const uint8_t shimmer = cylindrical_noise8(x, y, w, 110,
                                                                (uint16_t)(now_ms / 7));
                    const uint8_t value = byte_from_float(glow * (145.0f
                        + shimmer * 0.42f));
                    const uint8_t hue = (uint8_t)(112
                        + std::sin(angle + t * 0.2f) * 24.0f + now_ms / 180);
                    color = Rgb::from_hsv(hue, 220, value);
                    break;
                }

                case PatternStyle::OceanWaves: {
                    const float wave = std::sin(y * 0.72f + angle * 2.0f - t * 1.35f)
                                     + std::sin(y * 0.31f - angle * 3.0f + t * 0.74f);
                    const float normalized = (wave + 2.0f) * 0.25f;
                    const uint8_t texture = cylindrical_noise8(x, y, w, 105,
                                                                (uint16_t)(now_ms / 8));
                    const uint8_t index = byte_from_float(45.0f
                        + normalized * 170.0f + texture * 0.16f);
                    const uint8_t value = byte_from_float(85.0f
                        + normalized * 165.0f);
                    color = color_from_palette(kOceanColors, index, value);
                    break;
                }

                case PatternStyle::LavaLamp: {
                    const uint8_t n1 = cylindrical_noise8(x, y, w, 155,
                                                           (uint16_t)(now_ms / 7));
                    const uint8_t n2 = cylindrical_noise8(matrix.wrap_x(x + 3), y,
                                                           w, 83,
                                                           (uint16_t)(now_ms / 11));
                    const uint8_t blend = (uint8_t)(((uint16_t)n1 * 3 + n2) / 4);
                    const uint16_t hot = blend > 82
                        ? (uint16_t)(blend - 82) * 3 / 2 + 28
                        : blend / 4;
                    const uint8_t shaped = hot > 255 ? 255 : (uint8_t)hot;
                    color = color_from_palette(kLavaColors, shaped,
                                               qadd8(80, shaped));
                    break;
                }

                case PatternStyle::Kaleidoscope: {
                    const float mx = std::fabs(std::sin(angle * 2.0f)) * 4.0f;
                    const float my = std::fabs(y - cy);
                    const float facets = std::sin((mx + my) * 1.18f + t * 1.15f)
                                       + std::sin(std::cos(angle * 3.0f) * 3.0f
                                                  - my * 1.72f - t * 0.83f);
                    const uint8_t hue = (uint8_t)((int)(facets * 42.0f)
                                                + (int)(now_ms / 24));
                    const uint8_t value = byte_from_float(150.0f
                        + std::fabs(facets) * 52.0f);
                    color = Rgb::from_hsv(hue, 245, value);
                    break;
                }

                case PatternStyle::NeonRings: {
                    const float dx = cylinder_x;
                    const float dy = y - cy;
                    const float radius = std::sqrt(dx * dx + dy * dy);
                    const float wave = std::sin(radius * 2.15f - t * 2.35f);
                    float ring = clamp01((wave - 0.15f) / 0.85f);
                    ring *= ring;
                    const uint8_t value = byte_from_float(18.0f + ring * 237.0f);
                    const uint8_t hue = (uint8_t)(radius * 21.0f + now_ms / 18);
                    color = Rgb::from_hsv(hue, 250, value);
                    break;
                }
            }

            canvas.set_pixel(matrix.xy(x, y), color);
        }
    }
}

} // namespace ambient_matrix
