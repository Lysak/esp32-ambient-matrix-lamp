#pragma once
#include <cstdint>
#include "ambient_matrix/canvas.h"
#include "ambient_matrix/engine.h"

// Adapts ESPHome AddressableLight to MatrixCanvas.
// Created per-tick inside the addressable_lambda — zero allocation on the hot path.
class ESPHomeMatrixCanvas : public ambient_matrix::MatrixCanvas {
public:
    explicit ESPHomeMatrixCanvas(esphome::light::AddressableLight& light)
        : light_(light) {}

    void set_pixel(uint16_t index, ambient_matrix::Rgb color) override {
        light_[index] = esphome::Color(color.r, color.g, color.b);
    }

    void clear() override {
        for (int i = 0; i < light_.size(); i++)
            light_[i] = esphome::Color(0, 0, 0);
    }

    uint16_t size() const override {
        return static_cast<uint16_t>(light_.size());
    }

private:
    esphome::light::AddressableLight& light_;
};
