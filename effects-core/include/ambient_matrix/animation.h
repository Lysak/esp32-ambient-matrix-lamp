#pragma once

#include <cstdint>

namespace ambient_matrix {

struct FrameInfo {
    constexpr FrameInfo() = default;
    constexpr FrameInfo(uint32_t now_ms, uint16_t delta_ms, uint32_t total_ms)
        : now_ms(now_ms), delta_ms(delta_ms), total_ms(total_ms) {}

    uint32_t now_ms = 0;
    uint16_t delta_ms = 0;
    uint32_t total_ms = 0;

    float delta_s() const {
        return delta_ms * 0.001f;
    }

    float total_s() const {
        return total_ms * 0.001f;
    }
};

class AnimationClock {
public:
    explicit AnimationClock(uint16_t max_delta_ms = 80) : max_delta_ms_(max_delta_ms) {}

    void reset() {
        last_ms_ = 0;
        total_ms_ = 0;
    }

    FrameInfo tick(uint32_t now_ms) {
        if (last_ms_ == 0) {
            last_ms_ = now_ms;
            return FrameInfo{now_ms, 0, total_ms_};
        }

        uint32_t delta = now_ms - last_ms_;
        last_ms_ = now_ms;
        if (delta > max_delta_ms_) delta = max_delta_ms_;
        total_ms_ += delta;
        return FrameInfo{now_ms, static_cast<uint16_t>(delta), total_ms_};
    }

private:
    uint32_t last_ms_ = 0;
    uint32_t total_ms_ = 0;
    uint16_t max_delta_ms_ = 80;
};

class PhaseAccumulator {
public:
    void reset() {
        phase_q16_ = 0;
    }

    void advance_centered8(const FrameInfo& frame, uint8_t speed, uint16_t divisor_ms) {
        const int32_t centered = static_cast<int32_t>(speed) - 128;
        advance(frame, centered * 256, divisor_ms);
    }

    void advance_linear8(const FrameInfo& frame, uint8_t speed, uint16_t divisor_ms) {
        advance(frame, static_cast<int32_t>(speed) * 256, divisor_ms);
    }

    void advance_linear16(const FrameInfo& frame, uint8_t speed, uint16_t divisor_ms) {
        advance(frame, static_cast<int32_t>(speed), divisor_ms);
    }

    uint8_t byte() const {
        return static_cast<uint8_t>(phase_q16_ >> 24);
    }

    uint16_t word() const {
        return static_cast<uint16_t>(phase_q16_ >> 16);
    }

private:
    void advance(const FrameInfo& frame, int32_t numerator, uint16_t divisor_ms) {
        if (frame.delta_ms == 0 || divisor_ms == 0 || numerator == 0) return;
        const int64_t delta_q16 = (static_cast<int64_t>(frame.delta_ms) * numerator << 16) / divisor_ms;
        phase_q16_ = static_cast<uint32_t>(phase_q16_ + delta_q16);
    }

    uint32_t phase_q16_ = 0;
};

class FixedStep {
public:
    explicit FixedStep(uint16_t step_ms) : step_ms_(step_ms) {}

    void reset() {
        accumulated_ms_ = 0;
    }

    uint8_t consume(const FrameInfo& frame, uint8_t max_steps = 4) {
        accumulated_ms_ += frame.delta_ms;
        uint8_t steps = 0;
        while (accumulated_ms_ >= step_ms_ && steps < max_steps) {
            accumulated_ms_ -= step_ms_;
            steps++;
        }
        return steps;
    }

private:
    uint16_t step_ms_;
    uint16_t accumulated_ms_ = 0;
};

}  // namespace ambient_matrix
