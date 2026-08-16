#!/usr/bin/env bash

set -euo pipefail

readonly UPSTREAM_COMMIT="9a83a2bbe8e097e62c00f1dbd56849766775d7b6"

repository_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
worktree="${repository_root}/build/pokeemerald"
patch_file="${repository_root}/patches/reroll.patch"
mkdir -p -- "${repository_root}/build"
temporary_directory="$(mktemp -d "${repository_root}/build/reroll-patch.XXXXXX")"
temporary_patch="${temporary_directory}/reroll.patch"

cleanup() {
    git -C "${worktree}" reset --quiet HEAD -- >/dev/null 2>&1 || true
    rm -rf -- "${temporary_directory}"
}
trap cleanup EXIT

if [[ ! -d "${worktree}/.git" ]]; then
    printf 'error: run scripts/prepare.sh before updating the patch\n' >&2
    exit 1
fi

actual_commit="$(git -C "${worktree}" rev-parse HEAD)"
if [[ "${actual_commit}" != "${UPSTREAM_COMMIT}" ]]; then
    printf 'error: refusing to diff unexpected upstream commit %s\n' "${actual_commit}" >&2
    exit 1
fi

# Intent-to-add exposes new source files to git diff without staging content.
git -C "${worktree}" add --intent-to-add --all
git -C "${worktree}" diff --binary --full-index --no-ext-diff HEAD -- > "${temporary_patch}"

if [[ ! -s "${temporary_patch}" ]]; then
    printf 'error: no Reroll changes were found in %s\n' "${worktree}" >&2
    exit 1
fi

mkdir -p -- "$(dirname -- "${patch_file}")"
mv -- "${temporary_patch}" "${patch_file}"
printf 'Updated %s\n' "${patch_file}"
