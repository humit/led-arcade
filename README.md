# LED Arcade Platform

ESP32-based physical LED arcade platform.

This repo was generated from the latest uploaded monolithic Color Rally sketch.

## Arduino projects

```text
firmware/arduino/color_rally/
  Current dev sketch with:
  - open AP (`LED-Arcade`)
  - captive portal
  - Color Rally gameplay
  - badminton-style ball head/tail rendering
  - center white marker removed
  - global pause/resume
  - global mute/unmute

firmware/arduino/color_rally_baseline_uploaded/
  Exact uploaded monolithic baseline, preserved for rollback.
```

## Hardware baseline

- ESP32: WEMOS LOLIN32 Lite
- LED DIN: GPIO23
- Audio output: GPIO22 → PAM8403 L-IN
- LED count: 200
- AP IP: `10.10.10.1`

## WiFi QR payload

```text
WIFI:T:nopass;S:LED-Arcade;;
```

## Arduino IDE

Open:

```text
firmware/arduino/color_rally/color_rally.ino
```

Board notes:

- Use regular ESP32 board package, not Bluepad32 core.
- Board: WEMOS LOLIN32 Lite or ESP32 Dev Module.
- Partition: Huge APP is recommended if sketch size grows.
