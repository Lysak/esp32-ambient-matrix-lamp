#pragma once
#include "engine.h"

namespace ambient_matrix {

class EffectComets : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    static constexpr uint8_t kCount = 5;
    struct Comet {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        uint8_t hue = 0;
    };

    Comet comets_[kCount]{};
    uint32_t last_ms_ = 0;
    bool initialized_ = false;
};

} // namespace ambient_matrix
