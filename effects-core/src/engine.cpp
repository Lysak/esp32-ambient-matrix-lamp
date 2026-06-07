#include "ambient_matrix/engine.h"
#include "ambient_matrix/effect_rainbow.h"
#include "ambient_matrix/effect_color.h"
#include "ambient_matrix/effect_color_shift.h"
#include "ambient_matrix/effect_gradient.h"
#include "ambient_matrix/effect_perlin.h"
#include "ambient_matrix/effect_particles.h"
#include "ambient_matrix/effect_fire.h"
#include "ambient_matrix/effect_fire2020.h"
#include "ambient_matrix/effect_confetti.h"
#include "ambient_matrix/effect_tornado.h"

namespace ambient_matrix {

EffectEngine::EffectEngine(Matrix matrix) : matrix_(matrix) {}

void EffectEngine::set_effect(EffectId id) {
    switch (id) {
        case EffectId::Rainbow:    effect_ = std::make_unique<EffectRainbow>();    break;
        case EffectId::Color:      effect_ = std::make_unique<EffectColor>();      break;
        case EffectId::ColorShift: effect_ = std::make_unique<EffectColorShift>(); break;
        case EffectId::Gradient:   effect_ = std::make_unique<EffectGradient>();   break;
        case EffectId::Perlin:     effect_ = std::make_unique<EffectPerlin>();     break;
        case EffectId::Particles:  effect_ = std::make_unique<EffectParticles>();  break;
        case EffectId::Fire:       effect_ = std::make_unique<EffectFire>();       break;
        case EffectId::Fire2020:   effect_ = std::make_unique<EffectFire2020>();   break;
        case EffectId::Confetti:   effect_ = std::make_unique<EffectConfetti>();   break;
        case EffectId::Tornado:    effect_ = std::make_unique<EffectTornado>();    break;
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
