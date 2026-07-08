#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="firmware/arduino/color_rally"

# WEMOS LOLIN32 Lite + No OTA (Large APP)
# Required because embedded controller HTML makes the sketch larger than default partition.
FQBN="${1:-esp32:esp32:lolin32-lite:PartitionScheme=no_ota}"

arduino-cli compile \
  --fqbn "$FQBN" \
  "$SKETCH_DIR"
