#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

./tools/compile_color_rally.sh
./tools/upload_color_rally.sh

sleep 1

exec ./tools/monitor_color_rally.sh
