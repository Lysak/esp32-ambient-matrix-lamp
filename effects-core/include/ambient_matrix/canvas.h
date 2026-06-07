#pragma once
#include "types.h"
#include <cstdint>

namespace ambient_matrix {

class MatrixCanvas {
public:
    virtual void     set_pixel(uint16_t index, Rgb color) = 0;
    virtual void     clear() = 0;
    virtual uint16_t size() const = 0;
    virtual ~MatrixCanvas() = default;
};

} // namespace ambient_matrix
