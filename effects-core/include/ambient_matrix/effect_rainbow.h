#pragma once
#include "engine.h"

namespace ambient_matrix {

class EffectRainbow : public Effect {
public:
    void reset() override;
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;

private:
    uint32_t last_ms_    = 0;
    uint8_t  hue_offset_ = 0;
};

} // namespace ambient_matrix
