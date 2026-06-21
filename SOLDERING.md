# Guide for Soldering — Wi-Fi LED Lamp (ESP32 DevKit V1)

> This document is aligned with the current firmware pin map in `esphome/ambient_matrix_esp32.yaml`.

## Components

- ESP32 DevKit V1 (30-pin, CP2102)
- WS2812B LED matrix 16x16 (256 LEDs)
- Power supply 5V 12A
- Capacitor 1000uF 35V (470uF minimum also works)
- Resistor 470 Ohm
- Touch button TTP223
- BH1750 / GY-30 light sensor
- INMP441 I2S microphone
- MAX98357A I2S amplifier + speaker

---

## 1. Overall Wiring Diagram

```text
                      ┌──────────────────────────────────────┐
                      │           ESP32 DevKit V1            │
                      ├───────────────┬──────────────────────┤
                      │ Left side     │ Right side           │
                      ├───────────────┼──────────────────────┤
PSU (+) 5V ---------->│ 5V            │ 3V3  ----------------+--> TTP223 VCC
                      │               │                      +--> BH1750 VCC
                      │               │                      +--> INMP441 VDD
                      │               │                      +--> MAX98357A SD/MODE
PSU (-) GND --------->│ GND           │ GND  ----------------+--> Common GND
                      │               │ GPIO27  -------------> TTP223 OUT
                      │               │ GPIO25  --[470 Ohm]--> Matrix DIN
                      │               │ GPIO21  -------------> BH1750 SDA
                      │               │ GPIO22  -------------> BH1750 SCL
                      │               │ GPIO32  -------------> INMP441 SCK
                      │               │ GPIO33  -------------> INMP441 WS
                      │               │ GPIO34  -------------> INMP441 SD
                      │               │ GPIO13  -------------> MAX98357A LRC
                      │               │ GPIO26  -------------> MAX98357A BCLK
                      │               │ GPIO14  -------------> MAX98357A DIN
                      └───────────────┴──────────────────────┘

PSU (+) 5V ------------------------------+-------------------> Matrix 5V
                                         └-------------------> MAX98357A VIN

PSU (-) GND -----------------------------+-------------------> Matrix GND
                                         └-------------------> Common GND rail

                                         +| |-
                                      [ 1000uF ]
                                         |   |
                                         +---+---- across Matrix 5V / GND
```

---

## 2. Current Firmware Pin Map

| Function | ESP32 pin | Connected module pin |
|---|---|---|
| LED matrix data | **GPIO25** | WS2812B DIN through 470 Ohm resistor |
| Touch button input | **GPIO27** | TTP223 OUT |
| Light sensor SDA | **GPIO21** | BH1750 / GY-30 SDA |
| Light sensor SCL | **GPIO22** | BH1750 / GY-30 SCL |
| Microphone SCK | **GPIO32** | INMP441 SCK |
| Microphone WS | **GPIO33** | INMP441 WS |
| Microphone SD | **GPIO34** | INMP441 SD |
| Speaker LRCLK | **GPIO13** | MAX98357A LRC |
| Speaker BCLK | **GPIO26** | MAX98357A BCLK |
| Speaker DIN | **GPIO14** | MAX98357A DIN |

> `GPIO34` is input-only, which is correct for microphone data input.

---

## 3. WS2812B Matrix — Three Wires

| Wire on matrix | Color | Connect to |
|---|---|---|
| **5V** (VCC) | Red | Power supply (+) |
| **GND** | Black | Power supply (-) |
| **DIN** | Green | G25 on ESP32 through 470 Ohm resistor |

---

## 4. ESP32 DevKit V1 — Pin Layout

```text
USB connector at top

TOP ROW (left to right):
┌────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ 5V │ GND │ SD3 │ SD2 │ G13 │ GND │ G12 │ G14 │ G27 │ G26 │ G25 │ G33 │ G32 │ G35 │ G34 │  VN │  VP │  EN │ 3V3 │
└────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
  │     │              │           │           │           │     │                                         │
  │     │              │           │           │           │     └── Matrix DIN via 470 Ohm
  │     │              │           │           │           └──────── MAX98357A BCLK
  │     │              │           │           └──────────────────── TTP223 OUT
  │     │              │           └──────────────────────────────── MAX98357A DIN
  │     │              └──────────────────────────────────────────── MAX98357A LRC
  │     └─────────────────────────────────────────────────────────── PSU GND
  └───────────────────────────────────────────────────────────────── PSU 5V

BOTTOM ROW (left to right):
┌─────┬─────┬─────┬─────┬────┬────┬────┬─────┬─────┬────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ CLK │ SD0 │ SD1 │ G15 │ G2 │ G0 │ G4 │ G16 │ G17 │ G5 │ G18 │ G19 │ G21 │ RX0 │ TX0 │ G22 │ G23 │ GND │
└─────┴─────┴─────┴─────┴────┴────┴────┴─────┴─────┴────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
                                                           │                         │
                                                           └── BH1750 SDA           └── BH1750 SCL
```

---

## 5. Power — How the Wires Split

```text
Power Supply
    +5V
      ├──> ESP32 5V
      ├──> Matrix 5V
      └──> MAX98357A VIN

    GND
      ├──> ESP32 GND
      ├──> Matrix GND
      ├──> TTP223 GND
      ├──> BH1750 GND
      ├──> INMP441 GND
      └──> MAX98357A GND
```

> ESP32 and the matrix are powered **in parallel** from the same power supply.
> The ESP32 does NOT power the matrix.
>
> For a 16x16 matrix, keep the LED power wires thick and inject `5V` + `GND`
> into both ends of the matrix if your panel exposes both ends. This reduces
> voltage drop at higher brightness.

---

## 6. Capacitor

```text
      long leg (+)  ----> 5V
                   |
                [ 1000uF ]
                   |
      short leg (-) ----> GND
      stripe marks the negative side
```

- Connect **in parallel** across the matrix power input.
- Polarity matters, do not mix up the legs.
- Preferred part in this project: **1000uF 35V**.
- Minimum acceptable: **470uF**, at least **6.3V**.

---

## 7. Resistor 470 Ohm (Data Line Protection)

```text
ESP32 GPIO25 ----[470 Ohm]---- Matrix DIN
```

- Place it in series between `GPIO25` and `DIN`.
- No polarity, either direction works.
- Put it physically close to the matrix `DIN` side if possible.

---

## 8. Touch Button TTP223

| TTP223 pin | Connect to on ESP32 |
|---|---|
| **VCC** | 3V3 |
| **GND** | GND |
| **OUT** | G27 |

> The firmware expects **momentary mode**: `OUT = HIGH` only while the pad is touched.

---

## 9. Optional Modules Wired in the Current Firmware

### BH1750 / GY-30 light sensor

| Sensor pin | Connect to on ESP32 |
|---|---|
| **VCC** | 3V3 |
| **GND** | GND |
| **SDA** | G21 |
| **SCL** | G22 |
| **ADDR** | Leave default / LOW for address `0x23` |

### INMP441 microphone

| Mic pin | Connect to on ESP32 |
|---|---|
| **VDD** | 3V3 |
| **GND** | GND |
| **SCK** | G32 |
| **WS** | G33 |
| **SD** | G34 |
| **L/R** | GND |

### MAX98357A amplifier

| Amp pin | Connect to |
|---|---|
| **VIN** | 5V |
| **GND** | GND |
| **LRC** | G13 |
| **BCLK** | G26 |
| **DIN** | G14 |
| **SD / MODE** | 3V3 |
| **OUT+ / OUT-** | Speaker + / Speaker - |

---

## 10. Pre-Power Checklist

- [ ] Capacitor connected with correct polarity.
- [ ] 470 Ohm resistor placed between `G25` and `DIN`.
- [ ] 5V and GND from the power supply go to both ESP32 and matrix.
- [ ] Matrix `DIN` connected to `G25`, not `G13` and not `G27`.
- [ ] TTP223 powered from `3V3`, not `5V`.
- [ ] TTP223 `OUT` connected to `G27`.
- [ ] BH1750 `SDA/SCL` connected to `G21/G22`.
- [ ] INMP441 connected to `G32/G33/G34` and `L/R` tied to `GND`.
- [ ] MAX98357A connected to `G13/G26/G14` and shares common `GND`.
