#pragma once
#include "engine.h"

namespace ambient_matrix {

// Warm, slow-moving fire with inertia and a small number of rising sparks.
class EffectCampfire : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    static constexpr uint8_t kMaxSize = 16;
    static constexpr uint8_t kSparkCount = 5;

    struct Spark {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        uint8_t life = 0;
        bool active = false;
    };

    uint8_t heat_[kMaxSize][kMaxSize]{};
    uint8_t next_heat_[kMaxSize][kMaxSize]{};
    Spark sparks_[kSparkCount]{};
};

} // namespace ambient_matrix
