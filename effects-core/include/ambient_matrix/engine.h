#pragma once
#include "canvas.h"
#include "matrix.h"
#include "palette.h"
#include <cstdint>
#include <memory>
#include <utility>

#if __cplusplus < 201402L
namespace std {
template <typename T, typename... Args>
inline unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}  // namespace std
#endif

namespace ambient_matrix {

enum class EffectId : uint8_t {
    Rainbow = 0,
    Color, ColorShift, Gradient,
    Scanner, BlueScanner,
    Plasma, Aurora, OceanWaves, LavaLamp, Kaleidoscope, NeonRings,
    MatrixRain, Comets, Starfield,
    CosmicNebula, Wormhole, OrbitalDance, SolarStorm, Pulsar,
    StarrySky, AsteroidImpact, PacmanOrbit, SpacePolice, PurpleMeteors,
    LightSpeed, Supernova, MarsFlight, Singularity, RelicRadiation,
    Perlin, Particles,
    Campfire, Fire, Fire2020,
    Confetti, Tornado,
};

struct EffectParams {
    constexpr EffectParams() = default;
    constexpr EffectParams(uint8_t speed,
                           uint8_t scale,
                           uint8_t color,
                           PaletteId palette = PaletteId::Rainbow,
                           bool from_palette = false)
        : speed(speed),
          scale(scale),
          color(color),
          palette(palette),
          from_palette(from_palette) {}

    uint8_t speed = 128;
    uint8_t scale = 128;
    uint8_t color = 0;   // base hue 0-255
    PaletteId palette = PaletteId::Rainbow;
    bool from_palette = false;
};

class Effect {
public:
    virtual void reset() {}
    virtual void tick(MatrixCanvas& canvas, const Matrix& matrix,
                      const EffectParams& params, uint32_t now_ms) = 0;
    virtual ~Effect() = default;
};

class EffectEngine {
public:
    explicit EffectEngine(Matrix matrix);

    void set_effect(EffectId id);
    void set_params(EffectParams params);
    void tick(MatrixCanvas& canvas, uint32_t now_ms);

private:
    Matrix                 matrix_;
    EffectParams           params_;
    std::unique_ptr<Effect> effect_;
};

} // namespace ambient_matrix
