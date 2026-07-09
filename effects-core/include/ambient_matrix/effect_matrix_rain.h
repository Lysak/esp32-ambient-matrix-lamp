#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

class EffectMatrixRain : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    static constexpr uint8_t kMaxColumns = 16;
    float heads_[kMaxColumns]{};
    float speeds_[kMaxColumns]{};
    AnimationClock clock_;
    bool initialized_ = false;
};

} // namespace ambient_matrix
