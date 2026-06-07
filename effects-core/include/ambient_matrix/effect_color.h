#pragma once
#include "engine.h"

namespace ambient_matrix {

// Solid color fill. Based on case 2 from GyverLamp2/effects.ino.
class EffectColor : public Effect {
public:
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;
};

} // namespace ambient_matrix
