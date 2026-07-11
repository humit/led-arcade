# 8x32 matrix wiring

This document describes the validated 8x32 WS2812 matrix build using an ESP32, PAM8403 amplifier, speaker, and regulated 5 V PSU.

## Components

- ESP32 development board
- 8x32 WS2812B matrix
- PAM8403 amplifier
- Speaker
- Regulated 5 V PSU
- `330-340 ohm` series resistor on matrix data
- `10k ohm` pulldown resistor from matrix `DIN` to ground

## Connections

| Source | Destination |
|---|---|
| ESP32 `GPIO23` | `330-340 ohm` resistor -> matrix `DIN` |
| Matrix `DIN` | `10k ohm` resistor -> common `GND` |
| ESP32 `GPIO22` | PAM8403 `L-IN` |
| PSU `5V+` | ESP32 `5V`, matrix `5V`, PAM8403 `5V` |
| PSU `5V-` | ESP32 `GND`, matrix `GND`, PAM8403 `GND` |
| PAM8403 left speaker output | Speaker |

## Matrix connector orientation

| Matrix location | Pins | Purpose |
|---|---|---|
| Left | `DIN / GND / 5V` | Data input and power |
| Center | `GND / 5V` | Power injection |
| Right | `DOUT / GND / 5V` | Data output and power |

Data must enter through `DIN`. Connecting the ESP32 data line to `DOUT` will not drive the matrix.

## Firmware configuration

```cpp
#define MATRIX_DATA_PIN 23
#define STRIP_1D_DATA_PIN 19
#define AUDIO_PIN 22
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
```

Source: `firmware/arduino/pixel_derby/src/Config.h`

The matrix renderer uses `MATRIX_DATA_PIN`. `STRIP_1D_DATA_PIN` is reserved for a future simultaneous 1D display.
