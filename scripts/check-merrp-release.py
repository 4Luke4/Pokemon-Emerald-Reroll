#!/usr/bin/env python3
"""Resolve merrp's latest stable GitHub Release for dependency automation."""

from __future__ import annotations

import argparse
import json
import os
import pathlib
import re
import urllib.error
import urllib.request


REPOSITORY = "PokemonSanFran/merrp"
LATEST_RELEASE_URL = f"https://api.github.com/repos/{REPOSITORY}/releases/latest"
SAFE_TAG = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]{0,127}")


def write_outputs(path: pathlib.Path, values: dict[str, str]) -> None:
    with path.open("a", encoding="utf-8") as output:
        for name, value in values.items():
            if "\n" in value or "\r" in value:
                raise SystemExit(f"error: unsafe multiline workflow output: {name}")
            output.write(f"{name}={value}\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    parser.add_argument("--github-output", type=pathlib.Path, required=True)
    args = parser.parse_args()

    try:
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise SystemExit(f"error: cannot read dependency manifest: {error}") from error
    if manifest.get("repository") != REPOSITORY:
        raise SystemExit("error: dependency manifest identifies an unexpected repository")

    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "Pokemon-Emerald-Reroll dependency updater",
        "X-GitHub-Api-Version": "2022-11-28",
    }
    if token := os.environ.get("GITHUB_TOKEN"):
        headers["Authorization"] = f"Bearer {token}"
    request = urllib.request.Request(LATEST_RELEASE_URL, headers=headers)
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            release = json.load(response)
    except urllib.error.HTTPError as error:
        if error.code == 404:
            write_outputs(
                args.github_output,
                {"available": "false", "update_required": "false"},
            )
            print(f"{REPOSITORY} has no stable GitHub Release")
            return
        raise SystemExit(f"error: GitHub Releases API returned HTTP {error.code}") from error
    except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as error:
        raise SystemExit(f"error: cannot query GitHub Releases API: {error}") from error

    if not isinstance(release, dict):
        raise SystemExit("error: GitHub Releases API returned an unexpected payload")
    tag = release.get("tag_name")
    release_id = release.get("id")
    release_url = release.get("html_url")
    if release.get("draft") is not False or release.get("prerelease") is not False:
        raise SystemExit("error: GitHub's latest stable endpoint returned an unstable release")
    if not isinstance(tag, str) or SAFE_TAG.fullmatch(tag) is None:
        raise SystemExit("error: stable release has an unsafe or unsupported tag")
    if not isinstance(release_id, int) or release_id < 1:
        raise SystemExit("error: stable release has an invalid numeric ID")
    expected_url_prefix = f"https://github.com/{REPOSITORY}/releases/tag/"
    if release_url != expected_url_prefix + tag:
        raise SystemExit("error: stable release has an unexpected URL")

    source = manifest.get("source")
    recorded_release = source.get("release") if isinstance(source, dict) else None
    recorded_tag = (
        recorded_release.get("tag") if isinstance(recorded_release, dict) else None
    )
    update_required = tag != recorded_tag
    write_outputs(
        args.github_output,
        {
            "available": "true",
            "update_required": str(update_required).lower(),
            "release_id": str(release_id),
            "tag": tag,
            "url": release_url,
        },
    )
    print(f"latest stable merrp release: {tag}; update required: {update_required}")


if __name__ == "__main__":
    main()
