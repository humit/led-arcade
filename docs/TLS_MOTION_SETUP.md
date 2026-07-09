# TLS setup for motion controllers

The browser motion APIs require a secure context. The HTTP captive portal remains
available for onboarding, while the motion controller will be served from:

```text
https://play.nevershire.net/
```

Realtime traffic will use secure WebSocket on the same origin:

```text
wss://play.nevershire.net/ws
```

## 1. Install Certbot on macOS

```bash
brew install certbot
```

## 2. Request the certificate

The initial workflow uses a manual DNS-01 challenge, so it works with any DNS
provider:

```bash
./tools/request_play_cert_manual.sh
```

Certbot will display a TXT record similar to:

```text
_acme-challenge.play.nevershire.net
```

Create the requested TXT record in the public DNS zone, wait until it is visible,
then continue the Certbot prompt.

Manual DNS certificates do not renew automatically. Once the DNS provider is
known, replace this workflow with the corresponding Certbot DNS plugin.

## 3. Copy the issued files into the local secret directory

The request script copies:

```text
fullchain.pem
privkey.pem
```

into:

```text
secrets/tls/play.nevershire.net/
```

This directory must never be committed.

## 4. Generate the firmware header

```bash
./tools/generate_tls_header.py
```

Generated output:

```text
firmware/arduino/color_rally/src/generated/TlsCredentials.h
```

The generated header contains the certificate chain and private key as C string
literals. It must not be committed.

## 5. Validate the generated files

```bash
test -s secrets/tls/play.nevershire.net/fullchain.pem
test -s secrets/tls/play.nevershire.net/privkey.pem
test -s firmware/arduino/color_rally/src/generated/TlsCredentials.h
```

Inspect certificate dates:

```bash
openssl x509 \
  -in secrets/tls/play.nevershire.net/fullchain.pem \
  -noout -subject -issuer -dates
```

Verify the private key matches the certificate:

```bash
CERT_PUB="$(
  openssl x509 \
    -in secrets/tls/play.nevershire.net/fullchain.pem \
    -pubkey -noout |
  openssl pkey -pubin -outform der |
  shasum -a 256
)"

