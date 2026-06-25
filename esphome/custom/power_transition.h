#pragma once

#include <cstdint>

#include "ambient_matrix/matrix.h"
#include "ambient_matrix/types.h"

class PowerTransitionRenderer {
public:
    static void render_helix(esphome::light::AddressableLight& light,
                             const ambient_matrix::Matrix& matrix,
                             float progress,
                             bool turning_on) {
        clear(light);

        const uint8_t  width  = matrix.width();
        const uint8_t  height = matrix.height();
        const uint16_t total  = static_cast<uint16_t>(width) * height;

        if (width == 0 || height == 0 || total == 0) return;

        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        uint16_t visible = turning_on
            ? static_cast<uint16_t>(progress * total + 0.5f)
            : static_cast<uint16_t>((1.0f - progress) * total + 0.5f);
        if (visible > total) visible = total;

        const ambient_matrix::Rgb base_color{255, 170, 70};

        for (uint8_t y = 0; y < height; y++) {
            const uint8_t row_phase = width > 1 ? static_cast<uint8_t>((y * 2) % width) : 0;
            for (uint8_t x = 0; x < width; x++) {
                const uint16_t order = static_cast<uint16_t>(y) * width
                    + static_cast<uint16_t>((x + width - row_phase) % width);

                const bool pixel_on = turning_on
                    ? order < visible
                    : order >= static_cast<uint16_t>(total - visible);

                if (!pixel_on) continue;

                const uint16_t edge_distance = turning_on
                    ? static_cast<uint16_t>(visible - 1 - order)
                    : static_cast<uint16_t>(order - (total - visible));

                light[matrix.xy(x, y)] = to_color(base_color, edge_distance);
            }
        }
    }

    static void clear(esphome::light::AddressableLight& light) {
        for (int i = 0; i < light.size(); i++)
            light[i] = esphome::Color(0, 0, 0);
    }

private:
    static esphome::Color to_color(ambient_matrix::Rgb color, uint16_t edge_distance) {
        const uint8_t brightness = edge_distance < 10
            ? static_cast<uint8_t>(255 - edge_distance * 13)
            : 128;
        return esphome::Color(scale(color.r, brightness),
                              scale(color.g, brightness),
                              scale(color.b, brightness));
    }

    static uint8_t scale(uint8_t value, uint8_t brightness) {
        return static_cast<uint8_t>((static_cast<uint16_t>(value) * brightness) / 255);
    }
};
