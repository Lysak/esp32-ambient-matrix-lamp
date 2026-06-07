add_library(effects-core
    src/matrix.cpp
    src/engine.cpp
    src/noise.cpp
    src/palette.cpp
    src/effect_rainbow.cpp
    src/effect_color.cpp
    src/effect_color_shift.cpp
    src/effect_gradient.cpp
    src/effect_perlin.cpp
    src/effect_particles.cpp
    src/effect_fire.cpp
    src/effect_fire2020.cpp
    src/effect_confetti.cpp
    src/effect_tornado.cpp
)

target_include_directories(effects-core PUBLIC include)
target_compile_features(effects-core PUBLIC cxx_std_17)
