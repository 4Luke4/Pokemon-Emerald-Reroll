#!/usr/bin/env python3
"""Print a deterministic SHA-256 fingerprint for repository build inputs."""

from __future__ import annotations

import argparse
import hashlib
import pathlib


def iter_files(paths: list[pathlib.Path]) -> list[pathlib.Path]:
    """Expand files and directories into one stable, duplicate-free file list."""

    files: set[pathlib.Path] = set()
    for path in paths:
        if path.is_file():
            files.add(path.resolve())
        elif path.is_dir():
            files.update(
                candidate.resolve() for candidate in path.rglob("*") if candidate.is_file()
            )
        else:
            raise FileNotFoundError(f"build input does not exist: {path}")
    return sorted(files, key=lambda path: path.as_posix())


def fingerprint(paths: list[pathlib.Path], root: pathlib.Path) -> str:
    """Hash both relative names and bytes so renames invalidate prepared state."""

    digest = hashlib.sha256()
    resolved_root = root.resolve()
    for path in iter_files(paths):
        try:
            relative_path = path.relative_to(resolved_root)
        except ValueError as error:
            raise ValueError(f"build input is outside the repository: {path}") from error
        digest.update(relative_path.as_posix().encode("utf-8"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="+", type=pathlib.Path)
    parser.add_argument(
        "--root",
        type=pathlib.Path,
        default=pathlib.Path.cwd(),
        help="repository root used to normalize input names (default: current directory)",
    )
    arguments = parser.parse_args()
    print(fingerprint(arguments.paths, arguments.root))


if __name__ == "__main__":
    main()
