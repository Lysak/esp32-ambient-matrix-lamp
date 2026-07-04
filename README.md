# esp32-ambient-matrix-lamp

Ambient LED matrix lamp powered by ESP32-WROOM (CP2102 USB) and ESPHome.

This repository pins ESPHome in `.esphome-version` and Python in `.esphome-python-version` to stay compatible with Home Assistant `2024.3.3` on a low-memory Orange Pi host. Do not run ad-hoc `esphome` or `uvx esphome ...` commands from this repo; use the `make` targets so the pinned toolchain is always applied.

For compatibility with that older ESPHome line, the debug include is fixed at `esphome/common/debug_selected.yaml` instead of a dynamic include path from secrets.

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

Multi-click logic uses a short `350 ms` window after the last tap for faster response:

| Gesture | Action |
|---|---|
| 1 tap (lamp on) | Next effect |
| 1 tap (lamp off) | Power on, keep the last saved effect |
| 2 taps | Previous effect |
| 3 taps | No action |
| 4 taps | Toggle power on / off |
| Hold ≥ 800 ms (lamp on) | Ramp brightness level `1..10` every 500 ms |

### Sunrise schedule

`Sunrise` now follows the original GyverLamp2 dawn model as a scheduler, not a button gesture:

- time source: Home Assistant via the native ESPHome API
- per-day configuration: one enable switch and one alarm time entity for each weekday
- ramp: starts `Lamp Sunrise Duration` minutes before the alarm time
- peak: reaches full configured sunrise brightness exactly at the alarm time
- hold: stays at peak for `Lamp Sunrise Hold` minutes, then turns off

The sunrise only starts if the lamp is currently not being used as a normal effect lamp, meaning `Lamp Power` is off at the scheduled start minute. If the lamp is already on, that sunrise slot is skipped.

### Brightness and power limit

The firmware enforces a hard global output ceiling of about 33% of the raw LED matrix maximum, keeping the lamp inside the PSU budget.

Brightness is controlled separately through the `Lamp Brightness` number entity:

- range: `1..10`
- default: `8` (80% of the allowed 33% ceiling)
- persisted across reboot and power loss

The last selected effect is also persisted across reboot and power loss.

On boot, the lamp powers on automatically with the last saved effect and the last saved brightness level, without needing Home Assistant.

### Optional auto-rotation

Effect auto-rotation is disabled by default and exposed as a separate `Lamp Auto Rotate` switch in ESPHome / Home Assistant.
When enabled, it advances to the next effect every 1 minute while the lamp is on.

### Old-ESPHome compatibility note

For ESPHome `2024.3.2`, the two template dropdowns were replaced with numeric entities because that older release does not handle the newer `template select` automations used by this project.

- `Lamp Effect Index`: `0..8`
- `Effect Palette Index`: `0..7`

Lamp effect index mapping:

- `0` = `Color`
- `1` = `Color Shift`
- `2` = `Gradient`
- `3` = `Perlin`
- `4` = `Particles`
- `5` = `Fire`
- `6` = `Fire 2020`
- `7` = `Confetti`
- `8` = `Tornado`

Palette index mapping:

- `0` = `Heat`
- `1` = `Lava`
- `2` = `Party`
- `3` = `Rainbow`
- `4` = `Rainbow Stripe`
- `5` = `Cloud`
- `6` = `Ocean`
- `7` = `Forest`

---

## Prerequisites

Install `uv` once, then use the repository `Makefile` targets:

```bash
brew install uv
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
| `make esphome-version` | Show the pinned ESPHome version used by this repo |
| `make esphome-python-version` | Show the pinned Python version used for old ESPHome |
| `make esphome-compile` | Validate ESPHome firmware (no device needed) |
| `make esphome-flash` | Flash via OTA; uses `esphome/.device_ip` if present, otherwise mDNS |
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
debug_package: common/debug_off.yaml   # set common/debug_on.yaml to expose debug controls
```

Generate the API encryption key:

```bash
make gen-api-key
# or: python3 -c "import base64, os; print(base64.b64encode(os.urandom(32)).decode())"
```

Paste the output into `api_encryption_key: "..."` in `secrets.yaml`.

To expose the matrix orientation helper in Home Assistant, edit `esphome/common/debug_selected.yaml`.

The default committed version already exposes the debug button. To disable it, replace the file contents with:

```yaml
{}
```

When enabled, this adds a config button named `Matrix Debug Corners` that turns on a
four-corner test pattern:

- red `2x2`: `(0,0)`
- green `2x2`: `(15,0)`
- blue `2x2`: `(0,15)`
- yellow `2x2`: `(15,15)`

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

The pinned ESPHome + Python versions from `.esphome-version` and `.esphome-python-version` will compile the firmware, upload it via serial, then reboot the board.
On first boot the device connects to Wi-Fi using the credentials from secrets.

---

## OTA updates (after first flash)

Once the device is on Wi-Fi you no longer need the USB cable:

```bash
make esphome-flash
```

`make esphome-flash` first checks `esphome/.device_ip` (gitignored, local-only). If that file exists, OTA uses the saved IP address. Otherwise ESPHome falls back to mDNS (`ambient-matrix-lamp.local`), which can be flaky on some networks.

To force OTA by IP, write it to `esphome/.device_ip`:

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
