#include <cassert>
#include <cstdio>
#include "ambient_matrix/effect_color_source.h"

using ambient_matrix::EffectParams;
using ambient_matrix::PaletteId;
using ambient_matrix::indexed_effect_color;
using ambient_matrix::palette_by_id;

static void test_palette_mode_uses_palette_color() {
    EffectParams params;
    params.color = 96;
    params.palette = PaletteId::Heat;
    params.from_palette = true;

    auto expected = ambient_matrix::color_from_palette(palette_by_id(params.palette), 128, 255);
    auto actual = indexed_effect_color(params, palette_by_id(params.palette), 128);

    assert(actual.r == expected.r);
    assert(actual.g == expected.g);
    assert(actual.b == expected.b);
}

static void test_solid_mode_uses_effect_color() {
    EffectParams params;
    params.color = 96;
    params.palette = PaletteId::Heat;
    params.from_palette = false;

    auto expected = ambient_matrix::Rgb::from_hsv(96, 255, 255);
    auto actual = indexed_effect_color(params, palette_by_id(params.palette), 128);

    assert(actual.r == expected.r);
    assert(actual.g == expected.g);
    assert(actual.b == expected.b);
}

int main() {
    test_palette_mode_uses_palette_color();
    test_solid_mode_uses_effect_color();
    std::printf("all tests passed\n");
    return 0;
}
