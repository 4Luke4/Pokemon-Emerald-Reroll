#!/usr/bin/env python3
"""Fail fast when modular Reroll sources lose a challenge invariant."""

from __future__ import annotations

import pathlib
import re
import subprocess
import sys


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[1]
OVERLAY_ROOT = REPOSITORY_ROOT / "overlay"
PATCH_ROOT = REPOSITORY_ROOT / "patches" / "integration"
UPSTREAM_PATCH_ROOT = REPOSITORY_ROOT / "patches" / "upstream"

REQUIRED_MARKERS = {
    "src/reroll/rng.c": (
        "#define REROLL_CHACHA_ROUNDS 20",
        "Rejection sampling prevents modulo bias",
    ),
    "src/reroll/pokemon.c": (
        "if (level > 50 && !IsFinalEvolution(species))",
        "SPECIES_OLD_UNOWN_B",
        "ChooseWeightedMove",
        "CanSpeciesLearnTMHM",
        "AddEggMoves",
    ),
    "src/reroll/progression.c": (
        "gPlayerPartyCount = PARTY_SIZE",
        "RerollPokemon_CreateShinyPersonality",
        "Reroll_OnBattleEnd",
    ),
    "src/reroll/trainers.c": (
        "RerollTrainer_IsImportant(trainerId) ? PARTY_SIZE",
        "Reroll_GetTrainerAiFlags",
    ),
    "src/reroll/items.c": (
        "sPickupItems",
        "Reroll_UseSpareRepel",
        "Reroll_CanStoreItem",
    ),
    "src/reroll/hms.c": ("Reroll_FindVirtualHmUser",),
    "src/reroll/follower.c": (
        "FollowerSpriteCallback",
        "FindFirstConsciousPartyMon",
        "coordOffsetEnabled",
    ),
    "src/reroll/permadeath.c": ("ClearSaveData();",),
}


def fail(message: str) -> None:
    raise SystemExit(f"error: {message}")


def main() -> None:
    missing: list[str] = []
    for relative_path, markers in REQUIRED_MARKERS.items():
        source_path = OVERLAY_ROOT / relative_path
        if not source_path.is_file():
            missing.append(relative_path)
            continue
        source = source_path.read_text(encoding="utf-8")
        missing.extend(
            f"{relative_path}: {marker}" for marker in markers if marker not in source
        )
    if missing:
        fail("modular source is missing invariants: " + ", ".join(missing))

    patches = sorted(PATCH_ROOT.glob("*.patch"))
    if len(patches) < 5:
        fail("expected at least five topic-specific integration patches")
    for patch_path in patches:
        patch = patch_path.read_text(encoding="utf-8")
        if "/dev/null" in patch:
            fail(f"feature source must not be embedded in {patch_path.name}")
        if len(patch.splitlines()) > 250:
            fail(f"integration patch is too large to audit: {patch_path.name}")

    upstream_patches = sorted(UPSTREAM_PATCH_ROOT.glob("*.patch"))
    if not upstream_patches:
        fail("expected at least one isolated upstream hardening patch")
    expected_upstream_markers = {
        "tools/gbagfx/gfx.c",
        "tools/gbagfx/convert_png.c",
        "tools/rsfont/convert_png.c",
        "tools/rsfont/font.c",
        "tools/mid2agb/agb.cpp",
        "static_cast<unsigned long>(event.param2)",
    }
    upstream_patch_text = "\n".join(
        patch.read_text(encoding="utf-8") for patch in upstream_patches
    )
    missing_upstream_markers = sorted(
        marker for marker in expected_upstream_markers if marker not in upstream_patch_text
    )
    if missing_upstream_markers:
        fail(
            "upstream hardening is missing CodeQL corrections: "
            + ", ".join(missing_upstream_markers)
        )

    if (REPOSITORY_ROOT / "patches" / "reroll.patch").exists():
        fail("legacy monolithic patch is still present")

    prepare = (REPOSITORY_ROOT / "scripts" / "prepare.sh").read_text(encoding="utf-8")
    if "resolve-upstream.sh" not in prepare or "UPSTREAM_COMMIT=" in prepare:
        fail("prepare.sh must dynamically resolve upstream without a fixed commit")
    if "scripts/fingerprint.py" not in prepare:
        fail("prepare.sh must use the cross-platform build-input fingerprint")
    if "patches/upstream" not in prepare or "scripts/fingerprint.py patches overlay" not in prepare:
        fail("prepare.sh must apply and fingerprint isolated upstream patches")

    super_linter = (
        REPOSITORY_ROOT / ".github" / "workflows" / "super-linter.yml"
    ).read_text(encoding="utf-8")
    super_linter_fragments = (
        "permissions: {}\n",
        "    permissions:\n"
        "      contents: read\n"
        "      packages: read\n"
        "      # Super-Linter publishes the lint result as a GitHub commit status.\n"
        "      statuses: write\n",
    )
    if missing_permissions := [
        fragment for fragment in super_linter_fragments if fragment not in super_linter
    ]:
        fail(f"Super-Linter is missing {len(missing_permissions)} permission block(s)")

    codeql = (REPOSITORY_ROOT / ".github" / "workflows" / "codeql.yml").read_text(
        encoding="utf-8"
    )
    actions_analysis_markers = (
        "analyze-actions:",
        "languages: actions",
        "category: /language:actions",
    )
    if missing_actions_analysis := [
        marker for marker in actions_analysis_markers if marker not in codeql
    ]:
        fail(
            "CodeQL is missing GitHub Actions analysis markers: "
            + ", ".join(missing_actions_analysis)
        )

    version = (REPOSITORY_ROOT / "VERSION").read_text(encoding="utf-8").strip()
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        fail("VERSION must contain one stable semantic version")
    version_markers = {
        "README.md": (f"version-v{version}", f"pokemon-emerald-reroll-v{version}.gba"),
        "CHANGELOG.md": (f"## [{version}]",),
        "SECURITY.md": (f"`{version.rsplit('.', 1)[0]}.x`",),
    }
    for relative_path, markers in version_markers.items():
        contents = (REPOSITORY_ROOT / relative_path).read_text(encoding="utf-8")
        if missing_markers := [marker for marker in markers if marker not in contents]:
            fail(f"{relative_path} is missing version markers: {', '.join(missing_markers)}")

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

    marker_count = sum(len(markers) for markers in REQUIRED_MARKERS.values())
    print(f"verified {marker_count} invariants across {len(REQUIRED_MARKERS)} modules")


if __name__ == "__main__":
    try:
        main()
    except UnicodeDecodeError as error:
        print(f"error: could not decode project metadata: {error}", file=sys.stderr)
        raise SystemExit(1) from error
