#pragma once
#include "engine.h"
#include "types.h"

namespace ambient_matrix {

// Palette-colored particles on Perlin-noise trajectories with fade trail.
// Based on case 5 from GyverLamp2/effects.ino.
class EffectParticles : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    Rgb buf_[256]{};
};

} // namespace ambient_matrix
