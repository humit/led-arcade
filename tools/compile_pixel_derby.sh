#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
SKETCH_DIR="firmware/arduino/pixel_derby"
FQBN="${1:-esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200}"

echo "Compiling Pixel Derby"
echo "  Sketch: $SKETCH_DIR"
echo "  FQBN:   $FQBN"
arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"
