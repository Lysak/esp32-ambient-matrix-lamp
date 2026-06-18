#pragma once

// ── LAMP_MIC_ENABLED ──────────────────────────────────────────────────────────
// Comment out the line below to disable the INMP441 microphone at C++ level.
// (To disable the whole mic subsystem including HA entities, also comment out
//  the  mic: !include common/microphone.yaml  line in ambient_matrix_esp32.yaml.)
#define LAMP_MIC_ENABLED
// ─────────────────────────────────────────────────────────────────────────────

#include "ambient_matrix/mic/vol_analyzer.h"
#include "ambient_matrix/mic/clap_detector.h"

#ifdef LAMP_MIC_ENABLED
#include "driver/i2s_std.h"  // ESP-IDF 5.x new I2S API
#include "hal/gpio_types.h"
#endif

// INMP441 I²S MEMS microphone reader for ESP32.
//
// Wiring:
//   VDD  → 3.3 V
//   GND  → GND
//   SCK  → mic_sck pin (e.g. GPIO32)
//   WS   → mic_ws  pin (e.g. GPIO33)
//   SD   → mic_sd  pin (e.g. GPIO34, input-only GPIO)
//   L/R  → GND  (selects left channel / I²S address 0)
//
// call begin() once, then tick(millis()) every ≤ 10 ms.
class MicReader {
public:
#ifdef LAMP_MIC_ENABLED

    void begin(int pin_sck, int pin_ws, int pin_sd) {
        // Allocate RX-only I²S channel (ESP-IDF 5.x new API).
        i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
        i2s_new_channel(&chan_cfg, nullptr, &rx_handle_);

        // Standard Philips I²S — matches INMP441 output format.
        i2s_std_config_t std_cfg = {};
        std_cfg.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE);
        std_cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                               I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO);
        std_cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
        std_cfg.gpio_cfg.bclk = (gpio_num_t)pin_sck;
        std_cfg.gpio_cfg.ws   = (gpio_num_t)pin_ws;
        std_cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
        std_cfg.gpio_cfg.din  = (gpio_num_t)pin_sd;
        // invert_flags left zero-initialised — no inversion needed for INMP441

        i2s_channel_init_std_mode(rx_handle_, &std_cfg);
        i2s_channel_enable(rx_handle_);
    }

    // Drain the I²S DMA buffer and feed samples to the analyzers.
    // Call at ≤ 10 ms intervals for accurate clap detection.
    void tick(uint32_t now_ms) {
        int32_t buf[BUF_SIZE];
        size_t  bytes = 0;
        // portMAX_DELAY 0 → non-blocking: read whatever is available now.
        i2s_channel_read(rx_handle_, buf, sizeof(buf), &bytes, 0);
        int n = (int)(bytes / sizeof(int32_t));
        for (int i = 0; i < n; i++) vol_.push(buf[i]);
        clap_.tick(vol_.raw(), now_ms);
    }

    uint8_t  vol()               { return vol_.vol(); }
    bool     pulse()             { return vol_.pulse(); }
    uint8_t  getClaps()          { return clap_.getClaps(); }
    bool     hasClaps(uint8_t n) { return clap_.hasClaps(n); }
    void     setThreshold(uint32_t t) { clap_.setThreshold(t); }
    void     setTimeout(uint32_t ms)  { clap_.setTimeout(ms); }

private:
    static constexpr int SAMPLE_RATE = 16000;
    // One read covers up to 4 DMA frames × 64 samples = 256 samples ≈ 16 ms.
    static constexpr int BUF_SIZE    = 256;

    i2s_chan_handle_t rx_handle_ = nullptr;
    ambient_matrix::mic::VolAnalyzer  vol_;
    ambient_matrix::mic::ClapDetector clap_;

#else  // LAMP_MIC_ENABLED not defined — compile-away stubs

    void    begin(int, int, int)      {}
    void    tick(uint32_t)            {}
    uint8_t vol()                     { return 0; }
    bool    pulse()                   { return false; }
    uint8_t getClaps()                { return 0; }
    bool    hasClaps(uint8_t)         { return false; }
    void    setThreshold(uint32_t)    {}
    void    setTimeout(uint32_t)      {}

#endif // LAMP_MIC_ENABLED
};
