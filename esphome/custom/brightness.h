#pragma once

inline float lamp_brightness_fraction(int level) {
    if (level < 1) level = 1;
    if (level > 10) level = 10;
    return 0.08f + (float)(level - 1) * (0.92f / 9.0f);
}
