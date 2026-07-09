#!/usr/bin/env python3

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DOMAIN = "play.nevershire.net"


def read_pem(path: Path, marker: str) -> str:
    if not path.is_file():
        raise SystemExit(f"ERROR: Missing PEM file: {path}")

    value = path.read_text(encoding="ascii")

    if marker not in value:
        raise SystemExit(
            f"ERROR: {path} does not contain expected marker: {marker}"
        )

    if not value.endswith("\n"):
        value += "\n"

    return value


def raw_literal(name: str, value: str) -> str:
    delimiter = "LEDARCADE_TLS"

    if f"){delimiter}\"" in value:
        raise SystemExit(
            f"ERROR: PEM content contains raw-string delimiter: {delimiter}"
        )

    return (
        f"static const char {name}[] PROGMEM = "
        f'R"{delimiter}({value}){delimiter}";'
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--domain",
        default=DEFAULT_DOMAIN,
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
            "// Generated locally. Contains a private key.",
            "// Never commit this file.",
            f'static const char TLS_HOSTNAME[] = "{args.domain}";',
            "",
            raw_literal("TLS_CERT_PEM", cert_pem),
            "",
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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
