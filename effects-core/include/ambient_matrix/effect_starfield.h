#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

class EffectStarfield : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    static constexpr uint8_t kCount = 32;
    struct Star {
        float x = 0.0f;
        float y = 0.0f;
        float z = 1.0f;
        float speed = 0.2f;
        uint8_t hue = 160;
    };

    void spawn(Star& star, bool initial);

    Star stars_[kCount]{};
    AnimationClock clock_;
    bool initialized_ = false;
};

} // namespace ambient_matrix
