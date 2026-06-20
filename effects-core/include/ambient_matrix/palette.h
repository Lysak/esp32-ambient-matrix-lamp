#pragma once
#include "types.h"
#include <array>
#include <cstdint>

namespace ambient_matrix {

using Palette16 = std::array<Rgb, 16>;

// Interpolate within a 16-entry palette. index 0-255 spans the full palette.
// brightness 255 = full brightness.
Rgb color_from_palette(const Palette16& pal, uint8_t index, uint8_t brightness = 255);

extern const Palette16 kHeatColors;
extern const Palette16 kRainbowColors;
extern const Palette16 kOceanColors;
extern const Palette16 kForestColors;
extern const Palette16 kLavaColors;
extern const Palette16 kCampfireColors;

} // namespace ambient_matrix
