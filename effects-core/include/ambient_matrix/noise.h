#pragma once
#include <cstdint>

namespace ambient_matrix {

// Value noise using Perlin permutation table. Returns 0-255, smooth and pseudo-random.
// Adapted from public-domain Perlin noise (Ken Perlin).
uint8_t  inoise8(uint16_t x);
uint8_t  inoise8(uint16_t x, uint16_t y);
uint8_t  inoise8(uint16_t x, uint16_t y, uint16_t z);
uint16_t inoise16(uint32_t x, uint32_t y = 0, uint32_t z = 0);

// Periodic noise around the X axis of a cylindrical matrix.
uint8_t cylindrical_noise8(uint8_t x, uint8_t y, uint8_t width,
                           uint16_t spatial_scale, uint16_t time);

} // namespace ambient_matrix
