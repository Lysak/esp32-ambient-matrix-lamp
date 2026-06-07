#pragma once
#include <cstdint>

namespace ambient_matrix {

// Value noise using Perlin permutation table. Returns 0-255, smooth and pseudo-random.
// Adapted from public-domain Perlin noise (Ken Perlin).
uint8_t  inoise8(uint16_t x);
uint8_t  inoise8(uint16_t x, uint16_t y);
uint8_t  inoise8(uint16_t x, uint16_t y, uint16_t z);
uint16_t inoise16(uint32_t x, uint32_t y = 0, uint32_t z = 0);

} // namespace ambient_matrix
