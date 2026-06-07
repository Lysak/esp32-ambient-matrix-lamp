# esp32-ambient-matrix-lamp

Ambient LED matrix lamp powered by ESP32-WROOM (CP2102 USB) and ESPHome.

---

## Hardware

- **Board:** ESP32-WROOM-32 with CP2102 USB-to-UART chip
- **LEDs:** WS2812B strip wired to GPIO13 through a 470 Ω resistor

---

## Prerequisites

Install ESPHome CLI once:

```bash
uv tool install esphome
```

Install the CP2102 USB driver if not already present:

- macOS/Linux — usually works out of the box
- Windows — download from Silicon Labs: https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers

---

## Make commands

| Command | Description |
|---|---|
| `make build` | Compile native preview + tests |
| `make test` | Run all unit tests |
| `make preview` | Run Rainbow effect in terminal (Ctrl+C to stop) |
| `make clean` | Remove build directory |
| `make esphome-compile` | Validate ESPHome firmware (no device needed) |
| `make esphome-flash` | Flash via OTA (device on Wi-Fi) |
| `make esphome-flash DEVICE=/dev/tty.xxx` | Flash via USB serial |
| `make gen-api-key` | Generate a random ESPHome API encryption key |

---

## First-time flash (USB)

### 1. Fill in secrets

Copy the example and edit with your values:

```bash
cp esphome/secrets.yaml esphome/secrets.local.yaml
```

Edit `esphome/secrets.yaml` (already gitignored — never commit real credentials):

```yaml
project_root: /absolute/path/to/esp32-ambient-matrix-lamp   # path to repo root
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
ap_password: "SomeHotspotPassword"
api_encryption_key: "..."     # see below
ota_password: "SomeOTAPassword"
```

Generate the API encryption key:

```bash
make gen-api-key
# or: python3 -c "import base64, os; print(base64.b64encode(os.urandom(32)).decode())"
```

Paste the output into `api_encryption_key: "..."` in `secrets.yaml`.

### 2. Find the serial port

Plug in the ESP32 via USB, then:

```bash
# macOS / Linux
ls /dev/tty.*       # look for /dev/tty.usbserial-* or /dev/ttyUSB0

# Windows
# check Device Manager → Ports (COM & LPT) for COM3 / COM4 etc.
```

### 3. Compile and flash

```bash
make esphome-flash DEVICE=/dev/tty.usbserial-XXXX
```

ESPHome will compile the firmware, upload it via serial, then reboot the board.
On first boot the device connects to Wi-Fi using the credentials from secrets.

---

## OTA updates (after first flash)

Once the device is on Wi-Fi you no longer need the USB cable:

```bash
make esphome-flash
```

ESPHome auto-discovers the device on the local network and uploads over the air.

---

## Adding to Home Assistant

1. In Home Assistant go to **Settings → Integrations → Add integration → ESPHome**.
2. Enter the device hostname: `ambient-matrix-lamp.local` (or its IP address).
3. Enter the `api_encryption_key` from your secrets file.

---

## Project structure

```
esphome/
  ambient_matrix_esp32.yaml   — main config entry point
  boards/esp32-cp2102.yaml    — board definition (esp32dev + Arduino framework)
  common/base.yaml            — Wi-Fi, API, OTA, logging
  common/led_matrix.yaml      — WS2812B matrix entity
  custom/adapter.h            — ESPHome → effects-core bridge
  secrets.yaml                — secrets template (committed, no real values)

effects-core/                 — portable C++ effects engine (no ESPHome dependency)
```
