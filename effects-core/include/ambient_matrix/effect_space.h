#pragma once
#include "animation.h"
#include "engine.h"

namespace ambient_matrix {

enum class SpaceStyle : uint8_t {
    CosmicNebula,
    Wormhole,
    OrbitalDance,
    SolarStorm,
    Pulsar,
    StarrySky,
    AsteroidImpact,
    PacmanOrbit,
    SpacePolice,
    PurpleMeteors,
    LightSpeed,
    Supernova,
    MarsFlight,
    Singularity,
    RelicRadiation,
};

class EffectSpace : public Effect {
public:
    explicit EffectSpace(SpaceStyle style) : style_(style) {}

    void reset() override { clock_.reset(); }
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    AnimationClock clock_;
    SpaceStyle style_;
};

} // namespace ambient_matrix
