# LED Arcade Platform

ESP32-based physical LED arcade platform using phone controllers over a captive-portal Wi-Fi AP and WebSocket input.

## Current arena baseline

- **8×32 matrix:** Pixel Derby, Tron Arena, Pixel Raider, Color Clash, Pixel Pong
- **1D strip:** Reflex Rally, Power Push
- Board: WEMOS LOLIN32 Lite / ESP32
- Matrix DIN: GPIO23
- 1D strip DIN: GPIO22
- Audio: GPIO25 → PAM8403 L-IN
- AP SSID: `! OYUNA KATIL !`
- AP IP: `10.10.10.10`

## Build

```bash
cd /Users/uezerce/src/led-arcade
./tools/arcade compile
```

Default FQBN:

```text
esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200
```

## Upload and monitor

```bash
./tools/arcade deploy /dev/cu.usbserial-XXXX
```

## Runtime flow

```text
platform selector → arena-specific game selector → lobby → countdown → game → result
```

A single human player receives an automatic CPU opponent in multiplayer games that support solo entry. Realtime controller traffic uses WebSocket port 81; HTTP serves the captive portal only.

## Pixel Pong

Pixel Pong is the first 8×32 game added after the 1D arena baseline. It supports one human vs CPU or two humans, uses upper/lower phone controls, accelerates during rallies, and ends at five points.

Implementation and hardware test plan:

```text
docs/GAME_PIXEL_PONG.md
```

## Arduino CLI setup

```bash
./tools/setup_arduino_cli_macos.sh
```
