# Arduino CLI Development Setup

This document explains how to prepare a new macOS development machine for the LED Arcade Platform firmware workflow.

Current firmware target:

```text
Board: WEMOS LOLIN32 Lite
Arduino core: esp32:esp32
Sketch: firmware/arduino/color_rally
Default FQBN: esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200
1. Install Arduino CLI

Using Homebrew:

brew install arduino-cli

Verify:

arduino-cli version
2. Initialize Arduino CLI config
arduino-cli config init --overwrite

Add the Espressif ESP32 board package index:

arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json

Update indexes:

arduino-cli core update-index
3. Install ESP32 board core
arduino-cli core install esp32:esp32

Verify that the LOLIN32 Lite board is available:

arduino-cli board listall | grep -i "lolin32\|wemos\|esp32 dev"

Expected relevant entry:

WEMOS LOLIN32 Lite    esp32:esp32:lolin32-lite

Avoid esp32-bluepad32:* targets for the current captive portal / FastLED firmware unless explicitly working on Bluepad32 controller experiments.

4. Install required libraries
arduino-cli lib install FastLED

The following libraries are provided by the ESP32 core and do not need separate installation:

WiFi
WebServer
DNSServer
5. Compile Color Rally

From repo root:

./tools/compile_color_rally.sh

The script defaults to:

esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200

PartitionScheme=no_ota is required because the embedded controller HTML makes the firmware too large for the default partition.

Expected successful compile should show a maximum program storage around:

Maximum is 2097152 bytes

If the output instead shows:

Maximum is 1310720 bytes

then the large app partition is not being used.

6. Upload Color Rally

Connect the ESP32 by USB.

The upload script auto-detects a single /dev/cu.usbserial* port:

./tools/upload_color_rally.sh

If multiple matching ports exist, pass the port explicitly:

./tools/upload_color_rally.sh /dev/cu.usbserial-XXXX

To list connected boards/ports:

arduino-cli board list

or:

ls /dev/cu.* | grep -i "usb\|wch\|serial\|slab"
7. Override board target if needed

Compile with a custom FQBN:

./tools/compile_color_rally.sh "esp32:esp32:esp32:PartitionScheme=no_ota,UploadSpeed=115200"

Upload with a custom FQBN:

FQBN="esp32:esp32:esp32:PartitionScheme=no_ota,UploadSpeed=115200" \
  ./tools/upload_color_rally.sh /dev/cu.usbserial-XXXX
8. Upload speed note

The board package default upload speed may be 921600.

If uploads fail or are unstable, use:

UploadSpeed=115200

This is the current default in our scripts.

9. One-command setup

For a new macOS machine, run:

./tools/setup_arduino_cli_macos.sh

Then compile:

./tools/compile_color_rally.sh

Then connect ESP32 and upload:

./tools/upload_color_rally.sh
10. Known macOS notes

Homebrew may warn that Xcode Command Line Tools are outdated. If Arduino CLI installs and compiles successfully, this is not immediately blocking.

To reinstall Command Line Tools later:

sudo rm -rf /Library/Developer/CommandLineTools
sudo xcode-select --install

