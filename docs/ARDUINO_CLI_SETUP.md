# Arduino CLI Development Setup

This guide prepares a macOS development machine for the LED Arcade firmware workflow.

## Pinned toolchain

```text
Board: WEMOS LOLIN32 Lite
Arduino CLI: 1.5.1
ESP32 core: 3.3.10
FastLED: 3.10.5
Async TCP: 3.4.10
ESP Async WebServer: 3.6.0
Sketch: firmware/arduino/led_arcade
FQBN: esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200
```

`PartitionScheme=no_ota` is required because the embedded controller page does not fit in the default application partition.

## One-command setup

From the repository root:

```bash
./tools/setup_arduino_cli_macos.sh
./tools/arcade doctor
./tools/arcade compile --clean
```

The setup script installs the exact versions listed above. `arcade doctor` fails with corrective commands if a required core or library is absent or has a different version.

## Manual setup

Install Arduino CLI with Homebrew:

```bash
brew install arduino-cli
arduino-cli config init --overwrite
arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
```

Install the pinned core and libraries:

```bash
arduino-cli core install "esp32:esp32@3.3.10"
arduino-cli lib install "FastLED@3.10.5"
arduino-cli lib install "Async TCP@3.4.10"
arduino-cli lib install "ESP Async WebServer@3.6.0"
```

WiFi, Networking, DNSServer, FS, SPI, AsyncUDP, and Hash are provided by the ESP32 core.

## Compile, deploy, and monitor

Compile without uploading:

```bash
./tools/arcade compile --clean
```

Connect the ESP32 and run the normal compile, upload, and monitor workflow:

```bash
./tools/arcade deploy --clean
```

The tool auto-detects a single supported serial port. If more than one is present:

```bash
./tools/arcade ports
./tools/arcade deploy --clean --port /dev/cu.usbserial-XXXX
```

Use `Ctrl+C` to leave the serial monitor.

## Custom board target

Override the default FQBN when necessary:

```bash
FQBN="esp32:esp32:esp32:PartitionScheme=no_ota,UploadSpeed=115200" \
  ./tools/arcade compile --clean
```

Avoid `esp32-bluepad32:*` targets unless explicitly testing Bluepad32 controller support.

## Expected partition size

A successful build should report a maximum program storage size close to:

```text
Maximum is 2097152 bytes
```

If it reports approximately 1,310,720 bytes, the large application partition is not active.

## macOS notes

Homebrew may warn that Xcode Command Line Tools are outdated. If Arduino CLI installs and the firmware compiles, that warning is not immediately blocking.

To reinstall Command Line Tools later:

```bash
sudo rm -rf /Library/Developer/CommandLineTools
sudo xcode-select --install
```
