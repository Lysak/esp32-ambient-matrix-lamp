#pragma once
#include "engine.h"
#include "palette.h"

namespace ambient_matrix {

inline Rgb indexed_effect_color(const EffectParams& params,
                                const Palette16& palette,
                                uint8_t index) {
    if (params.from_palette) {
        return color_from_palette(palette, index, 255);
    }
    return Rgb::from_hsv(params.color, 255, 255);
}

} // namespace ambient_matrix
