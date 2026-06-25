// Palette definitions ported from FastLED (MIT license).
// Source: https://github.com/FastLED/FastLED/blob/master/src/colorpalettes.cpp

#include "ambient_matrix/palette.h"
#include "ambient_matrix/math_utils.h"

namespace ambient_matrix {

Rgb color_from_palette(const Palette16& pal, uint8_t index, uint8_t brightness) {
    uint8_t slot  = index >> 4;
    uint8_t blend = (uint8_t)((index & 0x0F) << 4);
    const Rgb& c0 = pal[slot];
    const Rgb& c1 = pal[slot < 15 ? slot + 1 : 15];
    Rgb r{
        lerp8u(c0.r, c1.r, blend),
        lerp8u(c0.g, c1.g, blend),
        lerp8u(c0.b, c1.b, blend)
    };
    if (brightness < 255) {
        r.r = scale8(r.r, brightness);
        r.g = scale8(r.g, brightness);
        r.b = scale8(r.b, brightness);
    }
    return r;
}

// Black -> dark red -> orange -> yellow -> white
const Palette16 kHeatColors = {{
    {  0,  0,  0}, { 32,  0,  0}, { 64,  0,  0}, { 96,  0,  0},
    {128,  0,  0}, {160,  0,  0}, {192,  0,  0}, {224,  0,  0},
    {255, 64,  0}, {255,128,  0}, {255,192,  0}, {255,255,  0},
    {255,255, 64}, {255,255,128}, {255,255,192}, {255,255,255},
}};

// Full hue cycle
const Palette16 kRainbowColors = {{
    Rgb::from_hsv(  0, 255, 255), Rgb::from_hsv( 17, 255, 255),
    Rgb::from_hsv( 34, 255, 255), Rgb::from_hsv( 51, 255, 255),
    Rgb::from_hsv( 68, 255, 255), Rgb::from_hsv( 85, 255, 255),
    Rgb::from_hsv(102, 255, 255), Rgb::from_hsv(119, 255, 255),
    Rgb::from_hsv(136, 255, 255), Rgb::from_hsv(153, 255, 255),
    Rgb::from_hsv(170, 255, 255), Rgb::from_hsv(187, 255, 255),
    Rgb::from_hsv(204, 255, 255), Rgb::from_hsv(221, 255, 255),
    Rgb::from_hsv(238, 255, 255), Rgb::from_hsv(255, 255, 255),
}};

// Dark blue -> blue -> teal -> cyan -> white
const Palette16 kOceanColors = {{
    {  0,  0, 16}, {  0,  0, 48}, {  0,  0, 96}, {  0, 16,128},
    {  0, 32,160}, {  0, 64,192}, {  0,128,224}, { 32,160,224},
    { 64,192,240}, {128,224,255}, {160,240,255}, {192,248,255},
    {224,252,255}, {240,255,255}, {248,255,255}, {255,255,255},
}};

// Deep green -> lime -> yellow-green -> light green
const Palette16 kForestColors = {{
    {  0, 16,  0}, {  0, 32,  0}, {  0, 64,  0}, { 16, 96,  0},
    {  0,128,  0}, { 32,128, 16}, { 64,160, 16}, { 96,192, 32},
    {128,208, 48}, {160,224, 64}, {192,240, 96}, {224,255,128},
    {200,255,160}, {220,255,192}, {240,255,224}, {255,255,255},
}};

// Black -> deep red -> red -> orange
const Palette16 kLavaColors = {{
    {  0,  0,  0}, {  8,  0,  0}, { 24,  0,  0}, { 48,  0,  0},
    { 80,  0,  0}, {128,  0,  0}, {180,  0,  0}, {220, 20,  0},
    {255, 60,  0}, {255,100,  0}, {255,140,  0}, {255,180,  0},
    {255,210, 40}, {255,235,100}, {255,245,180}, {255,255,255},
}};

// Ember black -> deep red -> orange -> warm yellow, without cold white.
const Palette16 kCampfireColors = {{
    {  0,  0,  0}, {  2,  0,  0}, {  7,  0,  0}, { 16,  0,  0},
    { 32,  1,  0}, { 55,  2,  0}, { 82,  4,  0}, {118,  8,  0},
    {158, 15,  0}, {198, 28,  0}, {232, 48,  0}, {250, 75,  0},
    {255,110,  2}, {255,150, 10}, {255,195, 42}, {255,230,110},
}};

// Blue/grey/white sky gradient (mirrors FastLED CloudColors)
const Palette16 kCloudColors = {{
    {  0,   0, 255}, {  0,   0, 220}, { 30,  30, 255}, { 60,  60, 255},
    {128, 128, 255}, {170, 170, 255}, {200, 200, 255}, {220, 220, 255},
    {255, 255, 255}, {220, 220, 255}, {180, 180, 220}, {128, 128, 200},
    { 60,  60, 180}, { 30,  30, 160}, {  0,   0, 200}, {  0,   0, 255},
}};

// Bright party colors: purple/magenta/blue/cyan/yellow/red cycle (mirrors FastLED PartyColors)
const Palette16 kPartyColors = {{
    { 85,   0, 128}, {129,   0, 147}, {170,   0, 154}, {186,   0, 122},
    {215,   0, 217}, {151,   0, 255}, { 12,   0, 255}, {  0,  55, 255},
    {  0, 255, 213}, {  0, 255,  85}, {  0, 255,  42}, { 55, 255,   0},
    {255, 255,   0}, {255, 128,   0}, {255,  85,   0}, {255,  42,   0},
}};

// Rainbow stripes: each hue block separated by a black entry (mirrors FastLED RainbowStripeColors)
const Palette16 kRainbowStripeColors = {{
    {255,   0,   0}, {  0,   0,   0}, {171, 171,   0}, {  0,   0,   0},
    {  0, 255,   0}, {  0,   0,   0}, {  0, 171, 171}, {  0,   0,   0},
    {  0,   0, 255}, {  0,   0,   0}, {171,   0, 171}, {  0,   0,   0},
    {255,   0,  85}, {  0,   0,   0}, {255,   0,   0}, {  0,   0,   0},
}};

// Zebra: white stripes on black, white every 4th entry
const Palette16 kZebraColors = {{
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    {255, 255, 255}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
}};

} // namespace ambient_matrix
