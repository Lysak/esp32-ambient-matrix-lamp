#pragma once
#include <cstdint>

namespace ambient_matrix {

// Serpentine (zigzag) layout, origin at bottom-left by default.
// y=0 is the bottom row; even rows go left→right, odd rows right→left.
// Optional X/Y flips allow compensating for a physically mirrored or upside-
// down matrix without rewriting any effect code.
class Matrix {
public:
    Matrix(uint8_t width, uint8_t height, bool flip_x = false, bool flip_y = false)
        : width_(width), height_(height), flip_x_(flip_x), flip_y_(flip_y) {}

    uint16_t xy(uint8_t x, uint8_t y) const {
        if (flip_x_) x = width_ - 1 - x;
        if (flip_y_) y = height_ - 1 - y;
        if (y % 2 == 0)
            return (uint16_t)y * width_ + x;
        else
            return (uint16_t)y * width_ + (width_ - 1 - x);
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
    bool    flip_x_;
    bool    flip_y_;
};

} // namespace ambient_matrix
