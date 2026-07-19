#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// Perlin-noise fire with rising sparks.
// Based on fire2020.ino (c) SottNick, Perlin noise fire by Yaroslaw Turbin.
// Source: https://github.com/AlexGyver/GyverLamp2
class EffectFire2020 : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    static constexpr uint8_t kMaxSparks = 8;

    uint16_t ff_y_ = 0;
    uint16_t ff_z_ = 0;
    float spark_x_[kMaxSparks]{};
    float spark_y_[kMaxSparks]{};
    uint8_t delta_value_ = 64;
    uint8_t delta_hue_ = 32;
    uint8_t step_ = 16;
    uint16_t last_scale_ = 256; // 256 = sentinel "never initialized"
    AnimationClock clock_;
    FixedStep stepper_{30};
};

} // namespace ambient_matrix
