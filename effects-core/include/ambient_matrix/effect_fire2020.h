#pragma once
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

    uint16_t ff_y_  = 0;
    uint16_t ff_z_  = 0;
    float    spark_x_[kMaxSparks]{};
    float    spark_y_[kMaxSparks]{};
    uint8_t  delta_value_ = 64;
    uint8_t  delta_hue_   = 32;
    uint8_t  step_        = 16;
    bool     initialized_ = false;
};

} // namespace ambient_matrix
