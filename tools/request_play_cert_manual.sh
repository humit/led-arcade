#!/usr/bin/env bash
set -euo pipefail

DOMAIN="${DOMAIN:-play.nevershire.net}"
EMAIL="${EMAIL:-}"
CERT_NAME="${CERT_NAME:-play.nevershire.net}"

ROOT="$(
  cd "$(dirname "${BASH_SOURCE[0]}")/.." &&
  pwd
)"

DEST="$ROOT/secrets/tls/$DOMAIN"
LIVE_DIR="/etc/letsencrypt/live/$CERT_NAME"

if ! command -v certbot >/dev/null 2>&1; then
  echo "ERROR: certbot is not installed."
  echo
  echo "Install it on macOS with:"
  echo "  brew install certbot"
  exit 1
fi

if [[ -z "$EMAIL" ]]; then
  cat >&2 <<EOM
ERROR: EMAIL is required for the ACME account.

Example:
  EMAIL=you@example.com ./tools/request_play_cert_manual.sh
EOM
  exit 1
fi

echo "Requesting certificate"
echo "  Domain: $DOMAIN"
echo "  Method: manual DNS-01"
echo
echo "Certbot will ask you to create a TXT record under:"
echo "  _acme-challenge.$DOMAIN"
echo

sudo certbot certonly \
  --manual \
  --preferred-challenges dns \
  --cert-name "$CERT_NAME" \
  --domain "$DOMAIN" \
  --email "$EMAIL" \
  --agree-tos \
  --no-eff-email

if ! sudo test -s "$LIVE_DIR/fullchain.pem"; then
  echo "ERROR: certificate not found: $LIVE_DIR/fullchain.pem" >&2
  exit 1
fi

if ! sudo test -s "$LIVE_DIR/privkey.pem"; then
  echo "ERROR: private key not found: $LIVE_DIR/privkey.pem" >&2
  exit 1
fi

mkdir -p "$DEST"

sudo install \
  -o "$(id -u)" \
  -g "$(id -g)" \
  -m 0644 \
  "$LIVE_DIR/fullchain.pem" \
  "$DEST/fullchain.pem"

sudo install \
  -o "$(id -u)" \
  -g "$(id -g)" \
  -m 0600 \
  "$LIVE_DIR/privkey.pem" \
  "$DEST/privkey.pem"

echo
echo "Certificate copied to:"
echo "  $DEST/fullchain.pem"
echo "  $DEST/privkey.pem"
echo
echo "Next:"
echo "  ./tools/generate_tls_header.py"
