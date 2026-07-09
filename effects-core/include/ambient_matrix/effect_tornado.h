#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// Perlin-noise driven spiral tornado.
// Based on case 9 from GyverLamp2/effects.ino.
class EffectTornado : public Effect {
public:
    void reset() override { clock_.reset(); phase_slow_.reset(); phase_fast_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    PhaseAccumulator phase_slow_;
    PhaseAccumulator phase_fast_;
};

} // namespace ambient_matrix
