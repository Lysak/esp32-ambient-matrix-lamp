#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

// A horizontal beam that smoothly scans from top to bottom and back.
class EffectScanner : public Effect {
public:
    explicit EffectScanner(bool blue_only) : blue_only_(blue_only) {}

    void reset() override { clock_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    bool blue_only_;
};

} // namespace ambient_matrix
