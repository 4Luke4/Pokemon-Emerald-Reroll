#!/usr/bin/env python3
"""Fail fast when the generated Reroll patch loses a core challenge invariant."""

from __future__ import annotations

import pathlib
import subprocess
import sys


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[1]
PATCH_PATH = REPOSITORY_ROOT / "patches" / "reroll.patch"
UPSTREAM_COMMIT = "9a83a2bbe8e097e62c00f1dbd56849766775d7b6"

REQUIRED_MARKERS = {
    "ChaCha20 stream": "#define REROLL_CHACHA_ROUNDS 20",
    "unbiased bounded selection": "Rejection sampling prevents modulo bias",
    "six-member parties": "gPlayerPartyCount = PARTY_SIZE",
    "shiny personality construction": "CreateShinyPersonality",
    "level-50 evolution boundary": "if (level > 50 && !IsFinalEvolution(species))",
    "legal TM/HM moves": "CanSpeciesLearnTMHM",
    "legal egg moves": "AddEggMoves",
    "Set battle style": "OPTIONS_BATTLE_STYLE_SET",
    "experience suppression": "gBattleScripting.getexpState = 6",
    "ball exclusion": "itemId >= FIRST_BALL && itemId <= LAST_BALL",
    "permadeath save erase": "ClearSaveData();",
    "progression reroll": "Reroll_OnBattleEnd",
    "important trainer expansion": "IsImportantTrainer(trainerId) ? PARTY_SIZE",
    "pickup allowlist": "sPickupItems",
    "modern Repel": "Reroll_UseSpareRepel",
    "lead follower": "FollowerSpriteCallback",
}


def fail(message: str) -> None:
    raise SystemExit(f"error: {message}")


def main() -> None:
    if not PATCH_PATH.is_file():
        fail(f"missing generated patch: {PATCH_PATH}")

    patch = PATCH_PATH.read_text(encoding="utf-8")
    missing = [name for name, marker in REQUIRED_MARKERS.items() if marker not in patch]
    if missing:
        fail("patch is missing invariants: " + ", ".join(missing))

    for metadata_path in (
        REPOSITORY_ROOT / "README.md",
        REPOSITORY_ROOT / "UPSTREAM.md",
        REPOSITORY_ROOT / "scripts" / "prepare.sh",
        REPOSITORY_ROOT / "scripts" / "update-patch.sh",
    ):
        if UPSTREAM_COMMIT not in metadata_path.read_text(encoding="utf-8"):
            fail(f"upstream pin mismatch in {metadata_path.relative_to(REPOSITORY_ROOT)}")

    tracked_paths = subprocess.run(
        ["git", "-C", str(REPOSITORY_ROOT), "ls-files", "-z"],
        check=True,
        capture_output=True,
    ).stdout.decode("utf-8").split("\0")
    forbidden_suffixes = {".gba", ".sav"}
    committed_binaries = [
        path
        for path in tracked_paths
        if path and pathlib.PurePosixPath(path).suffix.lower() in forbidden_suffixes
    ]
    if committed_binaries:
        fail("forbidden game binary present: " + ", ".join(map(str, committed_binaries)))

    print(f"verified {len(REQUIRED_MARKERS)} source invariants")


if __name__ == "__main__":
    try:
        main()
    except UnicodeDecodeError as error:
        print(f"error: could not decode project metadata: {error}", file=sys.stderr)
        raise SystemExit(1) from error
