#pragma once
#include "animation.h"
#include "engine.h"
#include "types.h"

namespace ambient_matrix {

// Random colored sparks that fade over time.
// Based on case 8 from GyverLamp2/effects.ino.
class EffectConfetti : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    Rgb buf_[256]{};
    AnimationClock clock_;
    FixedStep stepper_{30};
};

} // namespace ambient_matrix
