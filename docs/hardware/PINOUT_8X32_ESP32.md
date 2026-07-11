# ESP32 pinout reference

This is the current as-built and reserved pin map for the LED Arcade prototype.

## GPIO map

| Function | ESP32 pin | Destination | Status |
|---|---:|---|---|
| Matrix data | `GPIO23` | 8x32 WS2812 matrix `DIN` | Validated |
| 1D strip data | `GPIO19` | 1D WS2812 strip `DIN` | Reserved; simultaneous operation not yet validated |
| Audio output | `GPIO22` | PAM8403 `L-IN` | Validated |
| 5 V input | `5V` / JST `+` | PSU `5V+` | Regulated 5 V only |
| Ground | `GND` / JST `-` | PSU `5V-` | Common ground for all modules |

## Data-line components

Each display data line requires its own components.

| Data path | Series resistor | Pulldown |
|---|---:|---:|
| `GPIO23` -> matrix `DIN` | `330-340 ohm` | `10k ohm` from `DIN` to `GND` |
| `GPIO19` -> 1D strip `DIN` | `330-340 ohm` | `10k ohm` from `DIN` to `GND` |

## Firmware symbols

| Display | Firmware symbol | Value |
|---|---|---:|
| 8x32 matrix | `MATRIX_DATA_PIN` | `23` |
| 1D strip | `STRIP_1D_DATA_PIN` | `19` |
| Audio | `AUDIO_PIN` | `22` |

## Important limitation

The dedicated pins are prepared for a future dual-display build, but the following are not yet validated:

- simultaneous FastLED output on both pins,
- timing interaction between both LED buses,
- combined current draw,
- independent current limiting and brightness profiles.
