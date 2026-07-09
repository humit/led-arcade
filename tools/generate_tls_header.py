#!/usr/bin/env python3

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DOMAIN = "play.nevershire.net"


def raw_literal(name: str, value: str) -> str:
    delimiter = "LEDARCADE_TLS"

    if f"){delimiter}\"" in value:
        raise ValueError(
            f"PEM content unexpectedly contains raw-string delimiter: {delimiter}"
        )

    return (
        f"static const char {name}[] PROGMEM = "
        f'R"{delimiter}({value}){delimiter}";'
    )


def read_pem(path: Path, expected_marker: str) -> str:
    if not path.is_file():
        raise FileNotFoundError(f"Missing PEM file: {path}")

    value = path.read_text(encoding="ascii")

    if expected_marker not in value:
        raise ValueError(
            f"{path} does not contain expected marker: {expected_marker}"
        )

    if not value.endswith("\n"):
        value += "\n"

    return value


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate the local TLS credential header for Color Rally."
    )
    parser.add_argument(
        "--domain",
        default=DEFAULT_DOMAIN,
        help=f"Certificate domain; default: {DEFAULT_DOMAIN}",
    )
    args = parser.parse_args()

    secret_dir = ROOT / "secrets" / "tls" / args.domain
    cert_path = secret_dir / "fullchain.pem"
    key_path = secret_dir / "privkey.pem"

    cert_pem = read_pem(cert_path, "BEGIN CERTIFICATE")
    key_pem = read_pem(key_path, "BEGIN PRIVATE KEY")

    output = (
        ROOT
        / "firmware"
        / "arduino"
        / "color_rally"
        / "src"
        / "generated"
        / "TlsCredentials.h"
    )
    output.parent.mkdir(parents=True, exist_ok=True)

    content = "\n".join(
        [
            "#pragma once",
            "",
            "#include <Arduino.h>",
            "",
            "// Generated locally by tools/generate_tls_header.py.",
            "// Contains a private key. Never commit this file.",
            f'static const char TLS_HOSTNAME[] = "{args.domain}";',
            raw_literal("TLS_CERT_PEM", cert_pem),
            raw_literal("TLS_PRIVATE_KEY_PEM", key_pem),
            "",
            "static const size_t TLS_CERT_PEM_LEN = sizeof(TLS_CERT_PEM);",
            (
                "static const size_t TLS_PRIVATE_KEY_PEM_LEN = "
                "sizeof(TLS_PRIVATE_KEY_PEM);"
            ),
            "",
        ]
    )

    output.write_text(content, encoding="utf-8")
    output.chmod(0o600)

    print(f"Generated: {output}")
