// Based on fillNoiseLED() from GyverLamp v1 effects.ino
#pragma once
#include "canvas.h"
#include "engine.h"
#include <cstdint>
#include <cstring>

namespace ambient_matrix {

enum class NoiseStyle : uint8_t {
    Madness,    // raw HSV from noise (no palette)
    Clouds,     // kCloudColors
    Lava,       // kLavaColors
    Plasma,     // kPartyColors   + colorLoop
    Rainbow3D,  // kRainbowColors + colorLoop
    Peacock,    // kRainbowStripeColors + colorLoop
    Zebra,      // kZebraColors   + colorLoop
    Forest,     // kForestColors
    Ocean,      // kOceanColors
};

class EffectNoise : public Effect {
public:
    explicit EffectNoise(NoiseStyle style) : style_(style) {}
    void reset() override;
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;

private:
    NoiseStyle style_;
    uint16_t   x_ = 0, y_ = 0;
    uint32_t   z_ = 0;
    uint8_t    ihue_ = 0;
    uint8_t    noise_[16][16] = {};

    void fill_noise(uint8_t w, uint8_t h, uint8_t speed, uint8_t scale);
    void render_palette(MatrixCanvas& canvas, const Matrix& m,
                        const Palette16& pal, bool color_loop);
    void render_madness(MatrixCanvas& canvas, const Matrix& m);
};

} // namespace ambient_matrix
