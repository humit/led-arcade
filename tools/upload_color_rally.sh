#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="firmware/arduino/color_rally"
FQBN="${FQBN:-esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200}"

PORT="${1:-}"

if [[ -z "$PORT" ]]; then
  mapfile_cmd_available=false

  # macOS default bash can be old, so avoid relying on bash 4 mapfile.
  PORTS="$(ls /dev/cu.usbserial* 2>/dev/null || true)"
  COUNT="$(printf "%s\n" "$PORTS" | sed '/^$/d' | wc -l | tr -d ' ')"

  if [[ "$COUNT" == "0" ]]; then
    echo "ERROR: No /dev/cu.usbserial* port found."
    echo
    echo "Available Arduino boards/ports:"
    arduino-cli board list || true
    echo
    echo "Other likely serial ports:"
    ls /dev/cu.* 2>/dev/null | grep -i "usb\|wch\|serial\|slab" || true
    exit 1
  fi

  if [[ "$COUNT" != "1" ]]; then
    echo "ERROR: Multiple /dev/cu.usbserial* ports found:"
    printf "%s\n" "$PORTS"
    echo
    echo "Pass one explicitly, for example:"
    echo "  $0 /dev/cu.usbserial-XXXX"
    exit 1
  fi

  PORT="$(printf "%s\n" "$PORTS" | sed '/^$/d' | head -n 1)"
fi

echo "Uploading Color Rally"
echo "  Port: $PORT"
echo "  FQBN: $FQBN"
echo

arduino-cli upload \
  -p "$PORT" \
  --fqbn "$FQBN" \
  "$SKETCH_DIR"
