#include "ambient_matrix/animation.h"
#include <cassert>

using ambient_matrix::AnimationClock;
using ambient_matrix::FixedStep;
using ambient_matrix::FrameInfo;
using ambient_matrix::PhaseAccumulator;

int main() {
    {
        AnimationClock clock;
        FrameInfo frame = clock.tick(1000);
        assert(frame.delta_ms == 0);
        assert(frame.total_ms == 0);

        frame = clock.tick(1033);
        assert(frame.delta_ms == 33);
        assert(frame.total_ms == 33);

        frame = clock.tick(1333);
        assert(frame.delta_ms == 80);
        assert(frame.total_ms == 113);
    }

    {
        PhaseAccumulator phase;
        FrameInfo frame{1000, 16, 16};
        for (int i = 0; i < 64; i++) phase.advance_centered8(frame, 255, 1024);
        assert(phase.byte() > 0);

        phase.reset();
        for (int i = 0; i < 64; i++) phase.advance_linear8(frame, 255, 8192);
        assert(phase.byte() > 0);

        phase.reset();
        for (int i = 0; i < 64; i++) phase.advance_linear16(frame, 255, 2048);
        assert(phase.word() > 0);
    }

    {
        FixedStep stepper(30);
        FrameInfo frame{1000, 16, 16};
        assert(stepper.consume(frame) == 0);

        frame = FrameInfo{1016, 16, 32};
        assert(stepper.consume(frame) == 1);

        frame = FrameInfo{1032, 75, 107};
        assert(stepper.consume(frame) == 2);
    }

    return 0;
}
