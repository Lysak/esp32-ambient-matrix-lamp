BUILD_DIR := build
CXX       := clang++
CXXFLAGS  := -std=c++17 -Ieffects-core/include

CORE_SRCS := \
    effects-core/src/matrix.cpp \
    effects-core/src/engine.cpp \
    effects-core/src/noise.cpp \
    effects-core/src/palette.cpp \
    effects-core/src/effect_rainbow.cpp \
    effects-core/src/effect_color.cpp \
    effects-core/src/effect_color_shift.cpp \
    effects-core/src/effect_gradient.cpp \
    effects-core/src/effect_perlin.cpp \
    effects-core/src/effect_particles.cpp \
    effects-core/src/effect_fire.cpp \
    effects-core/src/effect_fire2020.cpp \
    effects-core/src/effect_confetti.cpp \
    effects-core/src/effect_tornado.cpp

.PHONY: build test preview clean esphome-compile help

help:
	@echo "build           compile preview + tests"
	@echo "test            run all tests"
	@echo "preview         run Rainbow effect in terminal (Ctrl+C to stop)"
	@echo "clean           remove build directory"
	@echo "esphome-compile validate ESPHome firmware (no device needed)"

build: $(BUILD_DIR)/preview $(BUILD_DIR)/test_xy_mapping

$(BUILD_DIR)/preview: $(CORE_SRCS) preview/main.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_xy_mapping: $(CORE_SRCS) tests/test_xy_mapping.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(BUILD_DIR)/test_xy_mapping
	@$(BUILD_DIR)/test_xy_mapping

preview: $(BUILD_DIR)/preview
	@$(BUILD_DIR)/preview

clean:
	rm -rf $(BUILD_DIR)

esphome-compile:
	esphome compile esphome/ambient_matrix_esp32.yaml
