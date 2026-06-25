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
    effects-core/src/effect_scanner.cpp \
    effects-core/src/effect_patterns.cpp \
    effects-core/src/effect_matrix_rain.cpp \
    effects-core/src/effect_comets.cpp \
    effects-core/src/effect_starfield.cpp \
    effects-core/src/effect_space.cpp \
    effects-core/src/effect_perlin.cpp \
    effects-core/src/effect_particles.cpp \
    effects-core/src/effect_campfire.cpp \
    effects-core/src/effect_fire.cpp \
    effects-core/src/effect_fire2020.cpp \
    effects-core/src/effect_confetti.cpp \
    effects-core/src/effect_tornado.cpp

.PHONY: build test preview clean esphome-clean esphome-compile esphome-flash esphome-upload esphome-logs esphome-rebuild esphome-reflash gen-api-key help

help:
	@echo "build           compile preview + tests"
	@echo "test            run all tests"
	@echo "preview         run Rainbow effect in terminal (Ctrl+C to stop)"
	@echo "clean           remove build directory"
	@echo "esphome-clean   remove ESPHome build artifacts"
	@echo "esphome-compile validate ESPHome firmware (no device needed)"
	@echo "esphome-flash   clean, compile, then flash via USB (DEVICE=/dev/tty.xxx) or OTA"
	@echo "esphome-logs    stream device logs via USB or network"
	@echo "esphome-rebuild clean ESPHome build artifacts, then compile"
	@echo "esphome-reflash clean ESPHome build artifacts, compile, then flash"
	@echo "gen-api-key     generate a random ESPHome API encryption key"

gen-api-key:
	@python3 -c "import base64, os; print(base64.b64encode(os.urandom(32)).decode())"

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

esphome-clean:
	rm -rf esphome/.esphome/build esphome/.esphome/idedata

# Override with DEVICE=<ip-or-hostname> or DEVICE=/dev/tty.xxx for USB.
# Without DEVICE, ESPHome discovers the lamp via mDNS (ambient-matrix-lamp.local).
DEVICE ?=

esphome-compile:
	cd esphome && uvx esphome compile ambient_matrix_esp32.yaml

esphome-upload:
	cd esphome && uvx esphome upload ambient_matrix_esp32.yaml $(if $(DEVICE),--device $(DEVICE),)

esphome-flash: esphome-clean esphome-compile esphome-upload

esphome-logs:
	cd esphome && uvx esphome logs ambient_matrix_esp32.yaml $(if $(DEVICE),--device $(DEVICE),)

esphome-rebuild: esphome-clean esphome-compile

esphome-reflash: esphome-flash
