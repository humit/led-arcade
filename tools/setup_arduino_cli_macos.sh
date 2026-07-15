#!/usr/bin/env bash
set -euo pipefail

if ! command -v brew >/dev/null 2>&1; then
  echo "ERROR: Homebrew not found. Install Homebrew first:"
  echo "  https://brew.sh/"
  exit 1
fi

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "Installing arduino-cli..."
  brew install arduino-cli
else
  echo "arduino-cli already installed:"
  arduino-cli version
fi

echo "Initializing Arduino CLI config..."
arduino-cli config init --overwrite

echo "Adding Espressif ESP32 board manager URL..."
arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json

echo "Updating board indexes..."
arduino-cli core update-index

echo "Installing ESP32 core..."
arduino-cli core install esp32:esp32

echo "Installing required libraries..."
arduino-cli lib install FastLED
arduino-cli lib install "WebSockets"
arduino-cli lib install "AsyncTCP"
arduino-cli lib install "ESP Async WebServer"

echo
echo "Installed Arduino CLI:"
arduino-cli version

echo
echo "Available relevant ESP32 boards:"
arduino-cli board listall | grep -i "lolin32\|wemos\|esp32 dev" || true

echo
echo "Setup complete."
echo "Next:"
echo "  ./tools/compile_color_rally.sh"
