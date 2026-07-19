#pragma once
#include <cstdint>

namespace ambient_matrix {

// Serpentine (zigzag) layout, origin at bottom-left.
// y=0 is the bottom row; even rows go left→right, odd rows right→left.
// This matches the physical panel wiring exactly (LED#0 at bottom-left), so no
// orientation correction is needed — verified with the Matrix Debug Corners test.
class Matrix {
public:
    Matrix(uint8_t width, uint8_t height)
        : width_(width), height_(height) {}

    uint16_t xy(uint8_t x, uint8_t y) const {
        if (y % 2 == 0)
            return (uint16_t)y * width_ + x;
        else
            return (uint16_t)y * width_ + (width_ - 1 - x);
    }

    bool in_bounds_y(int16_t y) const {
        return y >= 0 && y < height_;
    }

    uint8_t wrap_x(int16_t x) const {
        if (width_ == 0) return 0;
        x %= width_;
        if (x < 0) x += width_;
        return (uint8_t)x;
    }

    uint16_t xy_wrap(int16_t x, uint8_t y) const {
        return xy(wrap_x(x), y);
    }

    float shortest_x_delta(float x, float origin) const {
        if (width_ == 0) return 0.0f;
        float delta = x - origin;
        const float half = width_ * 0.5f;
        while (delta > half) delta -= width_;
        while (delta < -half) delta += width_;
        return delta;
    }

    uint8_t width()  const { return width_; }
    uint8_t height() const { return height_; }

private:
    uint8_t width_;
    uint8_t height_;
};

} // namespace ambient_matrix
