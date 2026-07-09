# LED Arcade Platform

ESP32-based physical LED arcade platform.

Current MVP game: **Color Rally**.

## Current hardware baseline

- Board: WEMOS LOLIN32 Lite / ESP32
- LED DIN: GPIO23
- Audio output: GPIO22 → PAM8403 L-IN
- LED count: 200
- Network: open AP captive portal
- AP SSID: `LED-Arcade`
- AP IP: `10.10.10.1`

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
2. Opening WiFi settings and tapping `LED-Arcade`.

QR payload:

```text
WIFI:T:nopass;S:LED-Arcade;;
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

