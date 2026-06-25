// Based on fillNoiseLED() + mapNoiseToLEDsUsingPalette() from GyverLamp v1 effects.ino
#include "ambient_matrix/effect_noise.h"
#include "ambient_matrix/noise.h"
#include "ambient_matrix/math_utils.h"
#include "ambient_matrix/palette.h"
#include "ambient_matrix/types.h"
#include <cstring>

namespace ambient_matrix {

void EffectNoise::reset() {
    x_ = y_ = ihue_ = 0;
    z_ = 0;
    memset(noise_, 0, sizeof(noise_));
}

void EffectNoise::fill_noise(uint8_t w, uint8_t h, uint8_t speed, uint8_t scale) {
    // dataSmoothing: blend previous noise with new when speed is slow
    uint8_t smoothing = (speed < 50) ? (uint8_t)(200 - speed * 4) : 0;

    for (uint8_t i = 0; i < w; i++) {
        uint16_t ioff = (uint16_t)scale * i;
        for (uint8_t j = 0; j < h; j++) {
            uint16_t joff = (uint16_t)scale * j;
            uint8_t data = inoise8((uint16_t)(x_ + ioff),
                                   (uint16_t)(y_ + joff),
                                   (uint16_t)(z_ & 0xFFFF));
            // Expand the lower range (mirrors v1 post-processing)
            data = qsub8(data, 16);
            data = qadd8(data, scale8(data, 39));
            if (smoothing) {
                uint8_t old = noise_[i][j];
                data = qadd8(scale8(old, smoothing),
                             scale8(data, (uint8_t)(255 - smoothing)));
            }
            noise_[i][j] = data;
        }
    }
    z_ += speed;
    x_ += speed / 8;
    y_ -= speed / 16;
    ihue_++;
}

void EffectNoise::render_madness(MatrixCanvas& canvas, const Matrix& m) {
    uint8_t w = m.width(), h = m.height();
    for (uint8_t i = 0; i < w; i++) {
        for (uint8_t j = 0; j < h; j++) {
            // v1: CHSV(noise[j][i], 255, noise[i][j]) — transposed axes
            canvas.set_pixel(m.xy(i, j),
                Rgb::from_hsv(noise_[j][i], 255, noise_[i][j]));
        }
    }
}

void EffectNoise::render_palette(MatrixCanvas& canvas, const Matrix& m,
                                  const Palette16& pal, bool color_loop) {
    uint8_t w = m.width(), h = m.height();
    for (uint8_t i = 0; i < w; i++) {
        for (uint8_t j = 0; j < h; j++) {
            // v1: index from noise[j][i], brightness from noise[i][j] (transposed)
            uint8_t idx = noise_[j][i];
            uint8_t bri = noise_[i][j];
            if (color_loop) idx = (uint8_t)(idx + ihue_);
            // Reshape brightness: above midpoint → full; below → quadratic roll-off
            if (bri > 127) {
                bri = 255;
            } else {
                bri = dim8_raw((uint8_t)(bri * 2));
            }
            canvas.set_pixel(m.xy(i, j), color_from_palette(pal, idx, bri));
        }
    }
}

void EffectNoise::tick(MatrixCanvas& canvas, const Matrix& matrix,
                       const EffectParams& params, uint32_t) {
    uint8_t w = matrix.width(), h = matrix.height();
    fill_noise(w, h, params.speed ? params.speed : 1, params.scale ? params.scale : 1);

    switch (style_) {
        case NoiseStyle::Madness:
            render_madness(canvas, matrix);
            break;
        case NoiseStyle::Clouds:
            render_palette(canvas, matrix, kCloudColors, false);
            break;
        case NoiseStyle::Lava:
            render_palette(canvas, matrix, kLavaColors, false);
            break;
        case NoiseStyle::Plasma:
            render_palette(canvas, matrix, kPartyColors, true);
            break;
        case NoiseStyle::Rainbow3D:
            render_palette(canvas, matrix, kRainbowColors, true);
            break;
        case NoiseStyle::Peacock:
            render_palette(canvas, matrix, kRainbowStripeColors, true);
            break;
        case NoiseStyle::Zebra:
            render_palette(canvas, matrix, kZebraColors, true);
            break;
        case NoiseStyle::Forest:
            render_palette(canvas, matrix, kForestColors, false);
            break;
        case NoiseStyle::Ocean:
            render_palette(canvas, matrix, kOceanColors, false);
            break;
    }
}

} // namespace ambient_matrix
