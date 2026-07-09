#pragma once
#include "animation.h"
#include "engine.h"
#include "types.h"

namespace ambient_matrix {

enum class BenchmarkStyle : uint8_t { Circle, Ball, Sine };

// Three visual timing probes:
//   Circle — dot on a circle with fading trail; tests AnimationClock continuity.
//   Ball   — bouncing ball with gravity via FixedStep; tests step accumulator.
//   Sine   — scrolling sine wave; tests steady phase advance across full width.
class EffectBenchmark : public Effect {
public:
    explicit EffectBenchmark(BenchmarkStyle style) : style_(style) {}

    void reset() override;
    void tick(MatrixCanvas&, const Matrix&, const EffectParams&, uint32_t now_ms) override;

private:
    void tick_circle(MatrixCanvas&, const Matrix&, const EffectParams&, const FrameInfo&);
    void tick_ball  (MatrixCanvas&, const Matrix&, const EffectParams&, const FrameInfo&);
    void tick_sine  (MatrixCanvas&, const Matrix&, const EffectParams&, const FrameInfo&);

    BenchmarkStyle style_;
    Rgb buf_[256]{};
    AnimationClock clock_;
    FixedStep stepper_{20};

    // Ball state
    float ball_x_ = 0.0f, ball_y_ = 0.0f;
    float ball_vx_ = 0.08f, ball_vy_ = 0.05f;
};

} // namespace ambient_matrix
