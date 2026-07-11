# LED Arcade Platform

ESP32-based physical LED arcade platform.

Current MVP game: **Color Rally**.

## Current hardware baseline

- Board: WEMOS LOLIN32 Lite / ESP32
- 8x32 matrix data: `GPIO23`
- 1D strip data: `GPIO19` (reserved; simultaneous operation is not yet validated)
- Audio output: `GPIO22` -> PAM8403 `L-IN`
- Power: regulated 5 V PSU with common ground
- LED data conditioning: `330-340 ohm` series resistor and `10k ohm` pulldown
- Access point: `! JOIN GAME !` at `10.10.10.10`

The matrix and 1D strip use separate reserved data pins. The dual-display electrical load, FastLED timing, and simultaneous rendering path are not yet validated.

Hardware documentation:

- [`docs/hardware/HARDWARE_WIRING.md`](docs/hardware/HARDWARE_WIRING.md)
- [`docs/hardware/WIRING_8X32_ESP32.md`](docs/hardware/WIRING_8X32_ESP32.md)
- [`docs/hardware/PINOUT_8X32_ESP32.md`](docs/hardware/PINOUT_8X32_ESP32.md)
- [`docs/hardware/TROUBLESHOOTING_LED_AUDIO.md`](docs/hardware/TROUBLESHOOTING_LED_AUDIO.md)

Repository language policy:

- [`docs/DEVELOPMENT_LANGUAGE.md`](docs/DEVELOPMENT_LANGUAGE.md)

## Build

```bash
cd /Users/uezerce/src/led-arcade-platform
./tools/compile_color_rally.sh
```

## Upload

```bash
./tools/upload_color_rally.sh
```

## Current UX target

User can join by either:

1. Scanning the WiFi QR code, or
2. Opening Wi-Fi settings and selecting `! JOIN GAME !`.

QR payload:

```text
WIFI:T:nopass;S:! JOIN GAME !;;
```

## Firmware layout

```text
firmware/arduino/color_rally/
  color_rally.ino
  src/
    Config.h
    Types.h
    SystemState.h
    hardware/
      AudioOut.h
      LedRenderer.h
    net/
      CaptivePortal.h
    controller/
      ControllerPage.h
    session/
      SessionManager.h
    games/color_rally/
      ColorRallyGame.h
```
## Arduino CLI workflow

Development compile/upload workflow is documented in:

```text
docs/ARDUINO_CLI_SETUP.md
```

Common commands:

```bash
./tools/setup_arduino_cli_macos.sh
./tools/compile_color_rally.sh
./tools/upload_color_rally.sh
```


## 8×32 Pixel Derby

New multiplayer demo:

```bash
./tools/compile_pixel_derby.sh
./tools/upload_pixel_derby.sh
```

Dependencies:

- FastLED
- WebSockets by Markus Sattler (`arduinoWebSockets`)

Flow: platform selector → 8×32 game selector → lobby → 2–8 player Pixel Derby.
Realtime controller traffic uses WebSocket port 81; HTTP remains only for the captive portal page.
