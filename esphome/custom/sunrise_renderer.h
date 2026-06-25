#pragma once

#include <cstdint>

class SunriseRenderer {
public:
    // Based on GyverLamp2 dawn logic from time.ino + effects.ino.
    static void render(esphome::light::AddressableLight& light,
                       uint8_t progress,
                       uint8_t max_brightness) {
        const esphome::Color color = scale(heat_color(progress), scale8(progress, max_brightness));
        for (int i = 0; i < light.size(); i++) {
            light[i] = color;
        }
    }

private:
    static esphome::Color heat_color(uint8_t temperature) {
        const uint8_t t192 = static_cast<uint8_t>((static_cast<uint16_t>(temperature) * 191) / 255);
        const uint8_t heatramp = static_cast<uint8_t>((t192 & 0x3F) << 2);

        if (t192 & 0x80) return esphome::Color(255, 255, heatramp);
        if (t192 & 0x40) return esphome::Color(255, heatramp, 0);
        return esphome::Color(heatramp, 0, 0);
    }

    static esphome::Color scale(esphome::Color color, uint8_t brightness) {
        return esphome::Color(scale8(color.r, brightness),
                              scale8(color.g, brightness),
                              scale8(color.b, brightness));
    }

    static uint8_t scale8(uint8_t value, uint8_t brightness) {
        return static_cast<uint8_t>((static_cast<uint16_t>(value) * brightness) / 255);
    }
};
