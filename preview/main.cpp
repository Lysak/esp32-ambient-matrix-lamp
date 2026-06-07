#include <cstdio>
#include <cstdint>
#include <vector>
#include <chrono>
#include <thread>
#include "ambient_matrix/canvas.h"
#include "ambient_matrix/matrix.h"
#include "ambient_matrix/engine.h"

static constexpr uint8_t W = 16;
static constexpr uint8_t H = 16;

class TerminalMatrixCanvas : public ambient_matrix::MatrixCanvas {
public:
    TerminalMatrixCanvas() : buf_(W * H) {}

    void set_pixel(uint16_t index, ambient_matrix::Rgb color) override {
        if (index < buf_.size()) buf_[index] = color;
    }

    void clear() override {
        for (auto& c : buf_) c = {};
    }

    uint16_t size() const override {
        return static_cast<uint16_t>(buf_.size());
    }

    void render(const ambient_matrix::Matrix& m, bool first) const {
        if (!first) printf("\033[%dA", H);
        for (int y = H - 1; y >= 0; y--) {
            for (int x = 0; x < W; x++) {
                const auto& c = buf_[m.xy(x, y)];
                printf("\033[48;2;%d;%d;%dm  ", c.r, c.g, c.b);
            }
            printf("\033[0m\n");
        }
        fflush(stdout);
    }

private:
    std::vector<ambient_matrix::Rgb> buf_;
};

int main() {
    ambient_matrix::Matrix        matrix(W, H);
    ambient_matrix::EffectEngine  engine(matrix);
    TerminalMatrixCanvas          canvas;

    engine.set_effect(ambient_matrix::EffectId::Rainbow);

    using clock = std::chrono::steady_clock;
    auto start  = clock::now();
    bool first  = true;

    while (true) {
        uint32_t now_ms = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            clock::now() - start).count();

        engine.tick(canvas, now_ms);
        canvas.render(matrix, first);
        first = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}
