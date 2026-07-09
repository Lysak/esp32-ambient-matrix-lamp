#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// Slowly cycling solid color. Based on case 3 from GyverLamp2/effects.ino.
class EffectColorShift : public Effect {
public:
    void reset() override { phase_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    PhaseAccumulator phase_;
};

} // namespace ambient_matrix
