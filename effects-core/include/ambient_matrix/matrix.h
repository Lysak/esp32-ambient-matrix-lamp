#pragma once
#include <cstdint>

namespace ambient_matrix {

// Serpentine (zigzag) layout, origin at bottom-left.
// y=0 is the bottom row; even rows go left→right, odd rows right→left.
// This matches the standard GyverLamp-style 16×16 matrix wiring.
class Matrix {
public:
    Matrix(uint8_t width, uint8_t height) : width_(width), height_(height) {}

    uint16_t xy(uint8_t x, uint8_t y) const {
        if (y % 2 == 0)
            return (uint16_t)y * width_ + x;
        else
            return (uint16_t)y * width_ + (width_ - 1 - x);
    }

    uint8_t width()  const { return width_; }
    uint8_t height() const { return height_; }

private:
    uint8_t width_;
    uint8_t height_;
};

} // namespace ambient_matrix
