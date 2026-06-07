#pragma once
#include "engine.h"

namespace ambient_matrix {

// Perlin-noise driven spiral tornado.
// Based on case 9 from GyverLamp2/effects.ino.
class EffectTornado : public Effect {
public:
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;
};

} // namespace ambient_matrix
