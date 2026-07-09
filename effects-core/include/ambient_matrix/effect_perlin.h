#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// 2D Perlin noise plasma mapped to a palette.
// Based on case 1 from GyverLamp2/effects.ino.
class EffectPerlin : public Effect {
public:
    void reset() override { clock_.reset(); phase_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    PhaseAccumulator phase_;
};

} // namespace ambient_matrix
