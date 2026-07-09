#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// Scrolling palette gradient along the matrix height.
// Based on case 4 from GyverLamp2/effects.ino.
class EffectGradient : public Effect {
public:
    void reset() override { clock_.reset(); phase_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    PhaseAccumulator phase_;
};

} // namespace ambient_matrix
