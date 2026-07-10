#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
./tools/compile_pixel_derby.sh
./tools/upload_pixel_derby.sh "${1:-}"
