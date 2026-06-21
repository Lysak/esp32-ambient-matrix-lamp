# esp32-ambient-matrix-lamp

Ambient LED matrix lamp powered by ESP32-WROOM (CP2102 USB) and ESPHome.

---

## Hardware

Full component list is in [`idea.md`](idea.md) — "Hardware already bought / selected" section.

### Wiring

```
WS2812B DIN  →  GPIO25 (P25)  via 470 Ω resistor
WS2812B 5V   →  external 5 V PSU
WS2812B GND  →  PSU GND  +  ESP32 GND (common ground)

TTP223 VCC   →  3.3 V (ESP32 3V3 pin)
TTP223 GND   →  GND
TTP223 OUT   →  GPIO27 (P27)
```

> **Do not connect 5 V to the ESP32 3V3 pin.** If the ESP32 is powered via USB or VIN/5V, the 3V3 pin outputs regulated 3.3 V — safe for TTP223 VCC.

> **TTP223 mode:** the firmware expects momentary mode — OUT goes HIGH while touched, LOW on release.
> If the module "latches" (toggle mode), change jumper A/B on the module to switch to momentary.

### Touch button behaviour

Multi-click logic mirrors GyverLamp2 (800 ms window after the last tap):

| Gesture | Action |
|---|---|
| 1 tap | Toggle power on / off |
| 2 taps | Next effect |
| 3 taps | Previous effect |
| 4 taps | First effect (Rainbow) |
| Hold ≥ 800 ms (lamp on) | Ramp brightness ±5/255 every 80 ms |
| Release after hold | Flip ramp direction for the next hold |

The lamp powers on automatically at boot (Rainbow, 35% brightness) without needing Home Assistant.

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

By default this resolves the device via mDNS (`ambient-matrix-lamp.local`), which can be flaky on some networks. To flash by IP instead, write it to `esphome/.device_ip` (gitignored, local-only):

```bash
echo "192.168.1.123" > esphome/.device_ip
```

`make esphome-flash` picks it up automatically; `DEVICE=...` on the command line still overrides it.

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
