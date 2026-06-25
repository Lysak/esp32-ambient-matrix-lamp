// Based on colorWheel, rainbowVertical, rainbowHorizontal from GyverLamp v1 effects.ino
#pragma once
#include "canvas.h"
#include "engine.h"

namespace ambient_matrix {

class EffectColorChange : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t hue_ = 0;
};

class EffectRainbowVert : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t offset_ = 0;
};

class EffectRainbowHoriz : public Effect {
public:
    void tick(MatrixCanvas& canvas, const Matrix& matrix,
              const EffectParams& params, uint32_t now_ms) override;
private:
    uint8_t offset_ = 0;
};

} // namespace ambient_matrix
