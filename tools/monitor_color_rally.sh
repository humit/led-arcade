#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-}"

if [[ -z "$PORT" ]]; then
  PORTS="$(find /dev -maxdepth 1 -name 'cu.usbserial*' -print 2>/dev/null || true)"
  COUNT="$(printf "%s\n" "$PORTS" | sed '/^$/d' | wc -l | tr -d ' ')"

  if [[ "$COUNT" == "0" ]]; then
    echo "ERROR: No /dev/cu.usbserial* port found."
    echo
    echo "Arduino CLI ports:"
    arduino-cli board list || true
    echo
    echo "Other likely serial ports:"
    find /dev -maxdepth 1 \
      \( -name 'cu.usb*' -o -name 'cu.wch*' -o -name 'cu.SLAB*' \) \
      -print 2>/dev/null || true
    exit 1
  fi

  if [[ "$COUNT" != "1" ]]; then
    echo "ERROR: Multiple /dev/cu.usbserial* ports found:"
    printf "%s\n" "$PORTS"
    echo
    echo "Pass one explicitly:"
    echo "  $0 /dev/cu.usbserial-XXXX"
    exit 1
  fi

  PORT="$(printf "%s\n" "$PORTS" | sed '/^$/d' | head -n 1)"
fi

echo "Monitoring Color Rally"
echo "  Port: $PORT"
echo "  Baud: 115200"
echo
echo "Exit with Ctrl+C"
echo

arduino-cli monitor \
  --port "$PORT" \
  --config baudrate=115200
