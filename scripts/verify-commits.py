#!/usr/bin/env python3
"""Validate Conventional Commit subjects and an optional pull-request title."""

from __future__ import annotations

import re
import subprocess
import sys


SUBJECT_PATTERN = re.compile(
    r"^(feat|fix|balance|build|ci|docs|refactor|perf|test|chore)"
    r"(?:\([a-z0-9][a-z0-9._/-]*\))?!?: [a-z0-9].*$"
)
MAXIMUM_SUBJECT_LENGTH = 72


def validate(label: str, subject: str) -> list[str]:
    errors: list[str] = []
    if len(subject) > MAXIMUM_SUBJECT_LENGTH:
        errors.append(
            f"{label} is {len(subject)} characters; maximum is {MAXIMUM_SUBJECT_LENGTH}"
        )
    if not SUBJECT_PATTERN.fullmatch(subject):
        errors.append(f"{label} is not a Conventional Commit: {subject!r}")
    return errors


def main() -> None:
    if len(sys.argv) not in (3, 4):
        raise SystemExit("usage: verify-commits.py <base> <head> [pull-request-title]")

    base, head = sys.argv[1:3]
    output = subprocess.run(
        ["git", "log", "--format=%H%x00%s", "--no-merges", f"{base}..{head}"],
        check=True,
        capture_output=True,
        text=True,
    ).stdout

    errors: list[str] = []
    for line in output.splitlines():
        commit, subject = line.split("\0", 1)
        errors.extend(validate(f"commit {commit[:12]}", subject))

    if len(sys.argv) == 4:
        errors.extend(validate("pull-request title", sys.argv[3]))

    if errors:
        print("Conventional Commit validation failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        raise SystemExit(1)

    print("Conventional Commit validation passed")


if __name__ == "__main__":
    main()
