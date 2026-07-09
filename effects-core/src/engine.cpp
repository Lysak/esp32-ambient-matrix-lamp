#include "ambient_matrix/engine.h"
#include "ambient_matrix/effect_rainbow.h"
#include "ambient_matrix/effect_color.h"
#include "ambient_matrix/effect_color_shift.h"
#include "ambient_matrix/effect_gradient.h"
#include "ambient_matrix/effect_scanner.h"
#include "ambient_matrix/effect_patterns.h"
#include "ambient_matrix/effect_matrix_rain.h"
#include "ambient_matrix/effect_comets.h"
#include "ambient_matrix/effect_starfield.h"
#include "ambient_matrix/effect_space.h"
#include "ambient_matrix/effect_perlin.h"
#include "ambient_matrix/effect_particles.h"
#include "ambient_matrix/effect_campfire.h"
#include "ambient_matrix/effect_fire.h"
#include "ambient_matrix/effect_fire2020.h"
#include "ambient_matrix/effect_confetti.h"
#include "ambient_matrix/effect_tornado.h"
#include "ambient_matrix/effect_benchmark.h"

namespace ambient_matrix {

EffectEngine::EffectEngine(Matrix matrix) : matrix_(matrix) {}

void EffectEngine::set_effect(EffectId id) {
    switch (id) {
        case EffectId::Rainbow:    effect_ = std::make_unique<EffectRainbow>();    break;
        case EffectId::Color:      effect_ = std::make_unique<EffectColor>();      break;
        case EffectId::ColorShift: effect_ = std::make_unique<EffectColorShift>(); break;
        case EffectId::Gradient:   effect_ = std::make_unique<EffectGradient>();   break;
        case EffectId::Scanner:    effect_ = std::make_unique<EffectScanner>(false); break;
        case EffectId::BlueScanner: effect_ = std::make_unique<EffectScanner>(true); break;
        case EffectId::Plasma: effect_ = std::make_unique<EffectPatterns>(PatternStyle::Plasma); break;
        case EffectId::Aurora: effect_ = std::make_unique<EffectPatterns>(PatternStyle::Aurora); break;
        case EffectId::OceanWaves: effect_ = std::make_unique<EffectPatterns>(PatternStyle::OceanWaves); break;
        case EffectId::LavaLamp: effect_ = std::make_unique<EffectPatterns>(PatternStyle::LavaLamp); break;
        case EffectId::Kaleidoscope: effect_ = std::make_unique<EffectPatterns>(PatternStyle::Kaleidoscope); break;
        case EffectId::NeonRings: effect_ = std::make_unique<EffectPatterns>(PatternStyle::NeonRings); break;
        case EffectId::MatrixRain: effect_ = std::make_unique<EffectMatrixRain>(); break;
        case EffectId::Comets: effect_ = std::make_unique<EffectComets>(); break;
        case EffectId::Starfield: effect_ = std::make_unique<EffectStarfield>(); break;
        case EffectId::CosmicNebula: effect_ = std::make_unique<EffectSpace>(SpaceStyle::CosmicNebula); break;
        case EffectId::Wormhole: effect_ = std::make_unique<EffectSpace>(SpaceStyle::Wormhole); break;
        case EffectId::OrbitalDance: effect_ = std::make_unique<EffectSpace>(SpaceStyle::OrbitalDance); break;
        case EffectId::SolarStorm: effect_ = std::make_unique<EffectSpace>(SpaceStyle::SolarStorm); break;
        case EffectId::Pulsar: effect_ = std::make_unique<EffectSpace>(SpaceStyle::Pulsar); break;
        case EffectId::StarrySky: effect_ = std::make_unique<EffectSpace>(SpaceStyle::StarrySky); break;
        case EffectId::AsteroidImpact: effect_ = std::make_unique<EffectSpace>(SpaceStyle::AsteroidImpact); break;
        case EffectId::PacmanOrbit: effect_ = std::make_unique<EffectSpace>(SpaceStyle::PacmanOrbit); break;
        case EffectId::SpacePolice: effect_ = std::make_unique<EffectSpace>(SpaceStyle::SpacePolice); break;
        case EffectId::PurpleMeteors: effect_ = std::make_unique<EffectSpace>(SpaceStyle::PurpleMeteors); break;
        case EffectId::LightSpeed: effect_ = std::make_unique<EffectSpace>(SpaceStyle::LightSpeed); break;
        case EffectId::Supernova: effect_ = std::make_unique<EffectSpace>(SpaceStyle::Supernova); break;
        case EffectId::MarsFlight: effect_ = std::make_unique<EffectSpace>(SpaceStyle::MarsFlight); break;
        case EffectId::Singularity: effect_ = std::make_unique<EffectSpace>(SpaceStyle::Singularity); break;
        case EffectId::RelicRadiation: effect_ = std::make_unique<EffectSpace>(SpaceStyle::RelicRadiation); break;
        case EffectId::Perlin:     effect_ = std::make_unique<EffectPerlin>();     break;
        case EffectId::Particles:  effect_ = std::make_unique<EffectParticles>();  break;
        case EffectId::Campfire:   effect_ = std::make_unique<EffectCampfire>();   break;
        case EffectId::Fire:       effect_ = std::make_unique<EffectFire>();       break;
        case EffectId::Fire2020:   effect_ = std::make_unique<EffectFire2020>();   break;
        case EffectId::Confetti:   effect_ = std::make_unique<EffectConfetti>();   break;
        case EffectId::Tornado:         effect_ = std::make_unique<EffectTornado>();    break;
        case EffectId::BenchmarkCircle: effect_ = std::make_unique<EffectBenchmark>(BenchmarkStyle::Circle); break;
        case EffectId::BenchmarkBall:   effect_ = std::make_unique<EffectBenchmark>(BenchmarkStyle::Ball);   break;
        case EffectId::BenchmarkSine:   effect_ = std::make_unique<EffectBenchmark>(BenchmarkStyle::Sine);   break;
    }
    if (effect_) effect_->reset();
}

void EffectEngine::set_params(EffectParams params) {
    params_ = params;
}

void EffectEngine::tick(MatrixCanvas& canvas, uint32_t now_ms) {
    if (!effect_) return;
    canvas.clear();
    effect_->tick(canvas, matrix_, params_, now_ms);
}

} // namespace ambient_matrix
