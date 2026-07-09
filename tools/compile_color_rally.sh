#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="firmware/arduino/color_rally"

# WEMOS LOLIN32 Lite + No OTA (Large APP) + safe upload speed.
# No OTA is required because embedded controller HTML makes the sketch larger
# than the default app partition.
FQBN="${1:-esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200}"

echo "Compiling Color Rally"
echo "  Sketch: $SKETCH_DIR"
echo "  FQBN:   $FQBN"
echo

arduino-cli compile \
  --fqbn "$FQBN" \
  "$SKETCH_DIR"
