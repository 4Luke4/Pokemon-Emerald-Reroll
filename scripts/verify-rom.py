#!/usr/bin/env python3
"""Validate structural invariants of a locally built Reroll GBA ROM."""

from __future__ import annotations

import hashlib
import pathlib
import sys


MINIMUM_ROM_SIZE = 1 << 20
MAXIMUM_ROM_SIZE = 32 << 20
EXPECTED_TITLE = b"POKEMON EMER"
EXPECTED_GAME_CODE = b"BPEE"


def fail(message: str) -> None:
    raise SystemExit(f"error: {message}")


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: verify-rom.py <rom.gba>")

    rom_path = pathlib.Path(sys.argv[1])
    if not rom_path.is_file():
        fail(f"ROM does not exist: {rom_path}")

    rom = rom_path.read_bytes()
    size = len(rom)
    if not MINIMUM_ROM_SIZE <= size <= MAXIMUM_ROM_SIZE:
        fail(f"unexpected ROM size: {size} bytes")
    if size & (size - 1):
        fail(f"ROM size is not a power of two: {size} bytes")
    if rom[0xA0:0xAC] != EXPECTED_TITLE:
        fail(f"unexpected title field: {rom[0xA0:0xAC]!r}")
    if rom[0xAC:0xB0] != EXPECTED_GAME_CODE:
        fail(f"unexpected game code: {rom[0xAC:0xB0]!r}")
    if rom[0xB2] != 0x96:
        fail(f"invalid fixed header byte: 0x{rom[0xB2]:02x}")

    expected_checksum = (-(sum(rom[0xA0:0xBD]) + 0x19)) & 0xFF
    if rom[0xBD] != expected_checksum:
        fail(
            "invalid header checksum: "
            f"found 0x{rom[0xBD]:02x}, expected 0x{expected_checksum:02x}"
        )

    digest = hashlib.sha256(rom).hexdigest()
    print(f"verified: {rom_path}")
    print(f"size: {size} bytes")
    print(f"sha256: {digest}")


if __name__ == "__main__":
    main()
