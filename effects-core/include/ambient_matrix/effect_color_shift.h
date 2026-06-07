#pragma once
#include "engine.h"

namespace ambient_matrix {

// Slowly cycling solid color. Based on case 3 from GyverLamp2/effects.ino.
class EffectColorShift : public Effect {
public:
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;
};

} // namespace ambient_matrix
