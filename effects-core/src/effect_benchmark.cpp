#include "ambient_matrix/effect_benchmark.h"
#include "ambient_matrix/math_utils.h"
#include <cmath>
#include <cstring>

namespace ambient_matrix {

static constexpr float kTau = 6.28318530f;

void EffectBenchmark::reset() {
    clock_.reset();
    stepper_.reset();
    memset(buf_, 0, sizeof(buf_));
    ball_x_ = 0.0f;
    ball_y_ = 0.0f;
    ball_vx_ = 0.08f;
    ball_vy_ = 0.05f;
}

void EffectBenchmark::tick(MatrixCanvas& canvas, const Matrix& matrix, const EffectParams& params,
                           uint32_t now_ms) {
    const FrameInfo frame = clock_.tick(now_ms);
    switch (style_) {
    case BenchmarkStyle::Circle:
        tick_circle(canvas, matrix, params, frame);
        break;
    case BenchmarkStyle::Ball:
        tick_ball(canvas, matrix, params, frame);
        break;
    case BenchmarkStyle::Sine:
        tick_sine(canvas, matrix, params, frame);
        break;
    }
}

// ---------------------------------------------------------------------------
// Circle — continuous sin/cos from total_s, fading trail shows frame history.
// ---------------------------------------------------------------------------
void EffectBenchmark::tick_circle(MatrixCanvas& canvas, const Matrix& matrix,
                                  const EffectParams& params, const FrameInfo& frame) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0) return;
    const uint16_t n = (uint16_t)w * h;

    for (uint16_t i = 0; i < n; i++)
        fade_to_black(buf_[i], 56);

    const float omega = (params.speed / 255.0f) * kTau * 0.5f;
    const float angle = frame.total_s() * omega;
    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;
    const float x = cx + cx * 0.8f * std::cos(angle);
    const float y = cy + cy * 0.8f * std::sin(angle);

    const int ix = (int)x;
    const int iy = (int)y;
    const uint8_t fx = (uint8_t)((x - (float)ix) * 255.0f);
    const uint8_t fy = (uint8_t)((y - (float)iy) * 255.0f);
    const uint8_t hue = (uint8_t)(frame.total_ms / 16);

    auto paint = [&](int px, int py, uint8_t w8) {
                     if (px < 0 || px >= (int)w || py < 0 || py >= (int)h) return;
                     const uint16_t idx = matrix.xy((uint8_t)px, (uint8_t)py);
                     if (idx < n) buf_[idx] = add_rgb(buf_[idx], Rgb::from_hsv(hue, 220, w8));
                 };
    paint(ix,     iy,     scale8(255 - fx, 255 - fy));
    paint(ix + 1, iy,     scale8(fx, 255 - fy));
    paint(ix,     iy + 1, scale8(255 - fx, fy));
    paint(ix + 1, iy + 1, scale8(fx, fy));

    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, buf_[i]);
}

// ---------------------------------------------------------------------------
// Ball — gravity + wall bounce via FixedStep. If steps clump, ball teleports.
// ---------------------------------------------------------------------------
void EffectBenchmark::tick_ball(MatrixCanvas& canvas, const Matrix& matrix,
                                const EffectParams& params, const FrameInfo& frame) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0) return;
    const uint16_t n = (uint16_t)w * h;

    if (ball_x_ == 0.0f && ball_y_ == 0.0f) {
        ball_x_ = (w - 1) * 0.5f;
        ball_y_ = (h - 1) * 0.25f;
    }

    const float gravity = 0.004f * (params.speed / 128.0f);
    const uint8_t steps = stepper_.consume(frame, 6);
    for (uint8_t s = 0; s < steps; s++) {
        ball_vy_ -= gravity;
        ball_x_ += ball_vx_;
        ball_y_ += ball_vy_;

        const float max_x = (float)(w - 1);
        const float max_y = (float)(h - 1);
        if (ball_x_ < 0.0f) {
            ball_x_ = -ball_x_;
            ball_vx_ = std::abs(ball_vx_);
        }
        if (ball_x_ > max_x) {
            ball_x_ = 2.0f * max_x - ball_x_;
            ball_vx_ = -std::abs(ball_vx_);
        }
        if (ball_y_ < 0.0f) {
            ball_y_ = -ball_y_;
            ball_vy_ = std::abs(ball_vy_) * 0.85f;
        }
        if (ball_y_ > max_y) {
            ball_y_ = 2.0f * max_y - ball_y_;
            ball_vy_ = -std::abs(ball_vy_) * 0.85f;
        }
    }

    for (uint16_t i = 0; i < n; i++)
        fade_to_black(buf_[i], 48);

    const int ix = (int)ball_x_;
    const int iy = (int)ball_y_;
    const uint8_t fx = (uint8_t)((ball_x_ - (float)ix) * 255.0f);
    const uint8_t fy = (uint8_t)((ball_y_ - (float)iy) * 255.0f);
    const uint8_t hue = (uint8_t)(frame.total_ms / 20);

    auto paint = [&](int px, int py, uint8_t w8) {
                     if (px < 0 || px >= (int)w || py < 0 || py >= (int)h) return;
                     const uint16_t idx = matrix.xy((uint8_t)px, (uint8_t)py);
                     if (idx < n) buf_[idx] = add_rgb(buf_[idx], Rgb::from_hsv(hue, 200, w8));
                 };
    paint(ix,     iy,     scale8(255 - fx, 255 - fy));
    paint(ix + 1, iy,     scale8(fx, 255 - fy));
    paint(ix,     iy + 1, scale8(255 - fx, fy));
    paint(ix + 1, iy + 1, scale8(fx, fy));

    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, buf_[i]);
}

// ---------------------------------------------------------------------------
// Sine — full-width wave scrolling left. Even column spacing = no phase drift.
// ---------------------------------------------------------------------------
void EffectBenchmark::tick_sine(MatrixCanvas& canvas, const Matrix& matrix,
                                const EffectParams& params, const FrameInfo& frame) {
    const uint8_t w = matrix.width();
    const uint8_t h = matrix.height();
    if (w == 0 || h == 0) return;
    const uint16_t n = (uint16_t)w * h;

    for (uint16_t i = 0; i < n; i++)
        fade_to_black(buf_[i], 80);

    const float scroll_speed = (params.speed / 255.0f) * kTau * 0.6f;
    const float phase = frame.total_s() * scroll_speed;
    const float amp = (h - 1) * 0.4f;
    const float cy = (h - 1) * 0.5f;
    const uint8_t hue = (uint8_t)(frame.total_ms / 12);

    for (uint8_t x = 0; x < w; x++) {
        const float angle = phase + (float)x * kTau / (float)w * 2.0f;
        const float y = cy + amp * std::sin(angle);

        const int iy = (int)y;
        const uint8_t fy = (uint8_t)((y - (float)iy) * 255.0f);
        const uint8_t col_hue = (uint8_t)(hue + x * 8);

        auto paint = [&](int py, uint8_t w8) {
                         if (py < 0 || py >= (int)h) return;
                         const uint16_t idx = matrix.xy(x, (uint8_t)py);
                         if (idx < n) buf_[idx] = add_rgb(buf_[idx],
                                                          Rgb::from_hsv(col_hue, 230, w8));
                     };
        paint(iy,     255 - fy);
        paint(iy + 1, fy);
    }

    for (uint16_t i = 0; i < n; i++)
        canvas.set_pixel(i, buf_[i]);
}

} // namespace ambient_matrix
