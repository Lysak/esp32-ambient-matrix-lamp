#pragma once
#include <cstdint>

namespace ambient_matrix {
namespace mic {

// Volume analyzer for I2S microphone input.
// Adapted from VolAnalyzer.h in GyverLamp2 by AlexGyver.
//
// Call push() for every raw int32 I2S sample (INMP441: 24-bit data
// in the upper 24 bits of a 32-bit frame).  When push() returns true
// a new smoothed volume is ready.  Call raw() to get the un-smoothed
// amplitude window peak — feed it to ClapDetector.
class VolAnalyzer {
public:
    // Returns true when the current sample window is complete.
    bool push(int32_t sample) {
        // INMP441 puts 24-bit data in bits 31..8 → right-shift to recover it.
        int32_t  s     = sample >> 8;
        uint32_t abs_s = s < 0 ? (uint32_t)(-s) : (uint32_t)s;
        if (abs_s > win_max_) win_max_ = abs_s;

        if (++count_ < WINDOW) return false;
        count_ = 0;

        raw_     = win_max_;
        win_max_ = 0;

        // Slow-tracking dynamic amplitude range — mirrors GyverLamp2's
        // maxF / minF exponential filters (peak_max decays, peak_min rises).
        if (raw_ > peak_max_) peak_max_ = raw_;
        else                  peak_max_ -= peak_max_ >> 9;

        if (raw_ < peak_min_) peak_min_ = raw_;
        else                  peak_min_ += (raw_ - peak_min_) >> 9;

        // Map window peak → 0-100 using the dynamic range.
        // Below noise threshold → 0 so silence reads clean.
        uint8_t new_vol = 0;
        if (peak_max_ > peak_min_ + NOISE_THRESHOLD) {
            int32_t mapped = (int32_t)(raw_ - peak_min_) * 100 /
                             (int32_t)(peak_max_ - peak_min_);
            if (mapped < 0)   mapped = 0;
            if (mapped > 100) mapped = 100;
            new_vol = (uint8_t)mapped;
        }

        // Exponential smoothing — same formula as GyverLamp2 FastFilter
        // with k1=26, k2=6:  out = (26*out + 6*in) >> 5
        vol_ = (uint8_t)(((uint32_t)26 * vol_ + (uint32_t)6 * new_vol) >> 5);

        if (new_vol > vol_ + 15) pulse_ = true;
        return true;
    }

    uint8_t  vol()  const { return vol_; }   // smoothed 0–100
    uint32_t raw()  const { return raw_; }   // unsmoothed window peak (for clap)

    bool pulse() {
        if (!pulse_) return false;
        pulse_ = false;
        return true;
    }

    void reset() {
        count_ = 0; win_max_ = 0; raw_ = 0;
        peak_max_ = 0; peak_min_ = 0x7FFFFFu;
        vol_ = 0; pulse_ = false;
    }

private:
    // 160 samples @ 16 kHz  →  10 ms window, matching GyverLamp2 Clap.h tick rate.
    static constexpr int      WINDOW          = 160;
    // Minimum amplitude spread before reporting non-zero volume.
    static constexpr uint32_t NOISE_THRESHOLD = 800;

    uint32_t win_max_  = 0;
    uint32_t raw_      = 0;
    uint32_t peak_max_ = 0;
    uint32_t peak_min_ = 0x7FFFFFu;
    uint8_t  vol_      = 0;
    int      count_    = 0;
    bool     pulse_    = false;
};

} // namespace mic
} // namespace ambient_matrix
