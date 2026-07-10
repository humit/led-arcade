#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
SKETCH_DIR="firmware/arduino/pixel_derby"
FQBN="${FQBN:-esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200}"
PORT="${1:-}"
if [[ -z "$PORT" ]]; then
  PORT="$(find /dev -maxdepth 1 -name 'cu.usbserial*' -print 2>/dev/null | head -n 1)"
fi
if [[ -z "$PORT" ]]; then
  echo "ERROR: serial port not found; pass it explicitly." >&2
  exit 1
fi
arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"
