#pragma once
#include "engine.h"

namespace ambient_matrix {

enum class PatternStyle : uint8_t {
    Plasma,
    Aurora,
    OceanWaves,
    LavaLamp,
    Kaleidoscope,
    NeonRings,
};

// Smooth time-based patterns that do not require persistent frame state.
class EffectPatterns : public Effect {
public:
    explicit EffectPatterns(PatternStyle style) : style_(style) {}

    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    PatternStyle style_;
};

} // namespace ambient_matrix
