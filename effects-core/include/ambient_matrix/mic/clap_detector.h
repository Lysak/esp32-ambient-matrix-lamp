#pragma once
#include <cstdint>

namespace ambient_matrix {
namespace mic {

// Derivative edge-detection clap counter.
// Direct port of Clap.h from GyverLamp2 by AlexGyver.
//   - Arduino.h dependency removed; millis() replaced with now_ms parameter.
//   - State machine and timing constants kept identical.
//
// Usage:
//   Call tick(amplitude, now_ms) every ~10 ms with the raw amplitude peak
//   from VolAnalyzer::raw().  After a clap burst, getClaps() returns the
//   count and clears it.
class ClapDetector {
public:
    // amplitude: unsmoothed peak from VolAnalyzer; now_ms: millis()-style counter.
    void tick(uint32_t amplitude, uint32_t now_ms) {
        if (now_ms - tick_tmr_ < 10u) return;
        tick_tmr_ = now_ms;

        int32_t val = (int32_t)amplitude;
        int32_t der = val - prev_val_;
        prev_val_   = val;

        int8_t signal = 0;
        if (der >  (int32_t)trsh_) signal =  1;
        if (der < -(int32_t)trsh_) signal = -1;

        int8_t front = 0;
        if (prev_signal_ == 0 && signal ==  1) front =  1;
        if (prev_signal_ == 0 && signal == -1) front = -1;
        prev_signal_ = signal;

        uint32_t deb = now_ms - edge_tmr_;

        if (front == 1 && state_ == 0) {
            state_ = 1;
            if (!burst_active_) { claps_ = 0; ready_ = false; }
            burst_active_ = true;
            single_clap_  = false;
            edge_tmr_     = now_ms;
        } else if (front == -1 && state_ == 1 && deb <= 200u) {
            state_    = 2;
            edge_tmr_ = now_ms;
        } else if (front == 0 && state_ == 2 && deb <= 200u) {
            state_       = 0;
            claps_++;
            single_clap_ = true;
            edge_tmr_    = now_ms;
        } else if (burst_active_ && deb > tout_ms_) {
            state_        = 0;
            burst_active_ = false;
            if (claps_ != 0) ready_ = true;
        }
    }

    // Derivative threshold on amplitude signal (default matches scaled INMP441 range).
    void setThreshold(uint32_t trsh)   { trsh_    = trsh; }
    // Burst window: ms to wait for additional claps before reporting (default 500 ms).
    void setTimeout(uint32_t tout_ms)  { tout_ms_ = tout_ms; }

    // Returns clap count after a burst and clears it; returns 0 if no burst yet.
    uint8_t getClaps() {
        if (!ready_) return 0;
        ready_ = false;
        uint8_t c = claps_;
        claps_    = 0;
        return c;
    }

    bool hasClaps(uint8_t n) {
        if (ready_ && claps_ == n) {
            ready_ = false;
            claps_ = 0;
            return true;
        }
        return false;
    }

    bool isSingleClap() {
        if (!single_clap_) return false;
        single_clap_ = false;
        return true;
    }

private:
    uint32_t tick_tmr_    = 0;
    uint32_t edge_tmr_    = 0;
    int32_t  prev_val_    = 0;
    uint32_t trsh_        = 3000;  // ~3 % of INMP441 24-bit range; tune for room acoustics
    uint32_t tout_ms_     = 500;   // burst window ms (GyverLamp2 default: 500 ms)
    int8_t   prev_signal_ = 0;
    uint8_t  state_       = 0;
    uint8_t  claps_       = 0;
    bool     ready_       = false;
    bool     single_clap_ = false;
    bool     burst_active_= false;
};

} // namespace mic
} // namespace ambient_matrix
