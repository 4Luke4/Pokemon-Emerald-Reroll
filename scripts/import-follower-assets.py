#!/usr/bin/env python3
"""Reproduce Reroll's HGSS-style follower asset overlay from merrp."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import subprocess


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[1]
MANIFEST_PATH = REPOSITORY_ROOT / "dependencies" / "merrp-followers.json"
EXPECTED_ROWS = 413
SOURCE_REPOSITORY = "PokemonSanFran/merrp"


def git_show(
    repository: pathlib.Path,
    commit: str,
    path: str,
    *,
    binary: bool = False,
) -> str | bytes:
    result = subprocess.run(
        ["git", "show", f"{commit}:{path}"],
        cwd=repository,
        check=True,
        capture_output=True,
        text=not binary,
    )
    return result.stdout


def load_manifest() -> dict[str, object]:
    if not MANIFEST_PATH.is_file():
        raise SystemExit(f"error: dependency manifest is missing: {MANIFEST_PATH}")
    try:
        manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        raise SystemExit(f"error: invalid dependency manifest: {error}") from error
    if manifest.get("repository") != SOURCE_REPOSITORY:
        raise SystemExit("error: dependency manifest identifies an unexpected repository")
    return manifest


def resolve_commit(repository: pathlib.Path, source_ref: str) -> str:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._/-]{0,254}", source_ref):
        raise SystemExit(f"error: unsafe source ref: {source_ref!r}")
    if ".." in source_ref or "//" in source_ref or source_ref.endswith("/"):
        raise SystemExit(f"error: invalid source ref: {source_ref!r}")
    result = subprocess.run(
        ["git", "rev-parse", "--verify", f"{source_ref}^{{commit}}"],
        cwd=repository,
        check=True,
        capture_output=True,
        text=True,
    )
    commit = result.stdout.strip()
    if not re.fullmatch(r"[0-9a-f]{40}", commit):
        raise SystemExit(f"error: source ref did not resolve to a full commit: {source_ref}")
    return commit


def asset_set_digest(asset_directory: pathlib.Path, filenames: list[str]) -> str:
    digest = hashlib.sha256()
    for filename in sorted(filenames):
        digest.update(filename.encode("utf-8"))
        digest.update(b"\0")
        digest.update((asset_directory / filename).read_bytes())
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "merrp_repository",
        type=pathlib.Path,
        help="local PokemonSanFran/merrp git checkout",
    )
    parser.add_argument(
        "--source-ref",
        help="tag, branch, or commit to import (defaults to the manifest lock)",
    )
    parser.add_argument("--release-tag", help="stable GitHub release tag, if any")
    parser.add_argument("--release-url", help="stable GitHub release URL, if any")
    args = parser.parse_args()
    source_repository = args.merrp_repository.resolve()
    if not (source_repository / ".git").exists():
        raise SystemExit(f"error: not a git checkout: {source_repository}")

    manifest = load_manifest()
    source = manifest.get("source")
    if not isinstance(source, dict):
        raise SystemExit("error: dependency manifest has no source object")
    source_ref = args.source_ref or source.get("resolved_commit")
    if not isinstance(source_ref, str):
        raise SystemExit("error: pass --source-ref or record a resolved commit in the manifest")
    if bool(args.release_tag) != bool(args.release_url):
        raise SystemExit("error: --release-tag and --release-url must be supplied together")
    source_commit = resolve_commit(source_repository, source_ref)

    info = git_show(
        source_repository,
        source_commit,
        "src/data/object_events/object_event_graphics_info_followers.h",
    )
    tables = git_show(
        source_repository,
        source_commit,
        "src/data/object_events/object_event_pic_tables.h",
    )
    graphics = git_show(
        source_repository,
        source_commit,
        "src/data/object_events/object_event_graphics.h",
    )
    assert isinstance(info, str) and isinstance(tables, str) and isinstance(graphics, str)

    species_to_table = dict(
        re.findall(
            r"\[(SPECIES_[A-Z0-9_]+)\].*?\b(sPicTable_[A-Za-z0-9_]+)\b",
            info,
        )
    )
    table_to_symbol = dict(
        re.findall(
            r"static const struct SpriteFrameImage (sPicTable_[A-Za-z0-9_]+)\[\] = \{\s*"
            r"overworld_frame\((gObjectEventPic_[A-Za-z0-9_]+),",
            tables,
        )
    )
    symbol_to_path = dict(
        re.findall(
            r"const u32 (gObjectEventPic_[A-Za-z0-9_]+)\[\] = "
            r"(?:INCBIN_COMP|INC(?:BIN|GFX)_U32)\(\"([^\"]+\.(?:4bpp|png))\"",
            graphics,
        )
    )

    rows: list[tuple[str, str, str]] = []
    for species, table in species_to_table.items():
        if species == "SPECIES_NONE":
            continue
        symbol = table_to_symbol[table]
        source_path = symbol_to_path[symbol]
        filename = pathlib.PurePosixPath(source_path).with_suffix(".png").name
        rows.append((species, symbol.removeprefix("gObjectEventPic_"), filename))

    rows.sort()
    if len(rows) != EXPECTED_ROWS:
        raise SystemExit(
            f"error: expected {EXPECTED_ROWS} species/form rows, found {len(rows)}"
        )

    asset_directory = (
        REPOSITORY_ROOT / "overlay" / "graphics" / "reroll" / "followers"
    )
    asset_directory.mkdir(parents=True, exist_ok=True)
    for _, _, filename in rows:
        source_path = f"graphics/object_events/pics/pokemon/{filename}"
        asset = git_show(source_repository, source_commit, source_path, binary=True)
        assert isinstance(asset, bytes)
        (asset_directory / filename).write_bytes(asset)

    expected_filenames = {filename for _, _, filename in rows}
    for existing_asset in asset_directory.glob("*.png"):
        if existing_asset.name not in expected_filenames:
            existing_asset.unlink()

    output = [
        "// Generated from dependencies/merrp-followers.json by",
        "// scripts/import-follower-assets.py; do not edit by hand.",
        "// Keep declarations and the designated species table together.",
    ]
    for _, symbol, filename in rows:
        output.extend(
            [
                f"static const u32 sFollowerGfx_{symbol}[] = INCGFX_U32(",
                f'    "graphics/reroll/followers/{filename}", ".4bpp.lz",',
                '    "-mwidth 4 -mheight 4");',
            ]
        )
    output.extend(
        [
            "",
            "static const u32 *const sFollowerGraphics[SPECIES_UNOWN_QMARK + 1] =",
            "{",
        ]
    )
    for species, symbol, _ in rows:
        output.append(f"    [{species}] = sFollowerGfx_{symbol},")
    output.extend(["};", ""])

    table_path = REPOSITORY_ROOT / "overlay/src/reroll/follower_graphics.inc.c"
    table_path.write_text("\n".join(output), encoding="utf-8")

    release = None
    source_kind = "manual"
    if args.release_tag is not None:
        source_kind = "stable-release"
        release = {"tag": args.release_tag, "url": args.release_url}
    elif source.get("kind") == "bootstrap" and source_commit == source.get("resolved_commit"):
        source_kind = "bootstrap"

    manifest = {
        "schema_version": 1,
        "repository": SOURCE_REPOSITORY,
        "source": {
            "kind": source_kind,
            "resolved_commit": source_commit,
            "release": release,
        },
        "assets": {
            "count": len(rows),
            "format": "indexed PNG, 192x32, six 32x32 frames",
            "sha256": asset_set_digest(asset_directory, list(expected_filenames)),
        },
    }
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"imported {len(rows)} follower sheets from merrp {source_commit}")


if __name__ == "__main__":
    main()
