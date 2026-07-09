#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// Classic 2D heat-diffusion fire. Each column has an independent heat array.
// Based on case 6 from GyverLamp2/effects.ino.
class EffectFire : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    uint8_t heat_[16][16]{};  // heat_[x][y], y=0 is bottom (hottest)
    AnimationClock clock_;
    FixedStep stepper_{30};
};

} // namespace ambient_matrix
