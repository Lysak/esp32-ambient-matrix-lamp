#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

class EffectRainbow : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    PhaseAccumulator phase_;
};

} // namespace ambient_matrix
