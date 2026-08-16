#!/usr/bin/env bash

set -euo pipefail

readonly UPSTREAM_URL="https://github.com/pret/pokeemerald.git"
readonly UPSTREAM_COMMIT="9a83a2bbe8e097e62c00f1dbd56849766775d7b6"

repository_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
worktree="${repository_root}/build/pokeemerald"
patch_file="${repository_root}/patches/reroll.patch"

if [[ ! -f "${patch_file}" ]]; then
    printf 'error: missing patch: %s\n' "${patch_file}" >&2
    exit 1
fi

if [[ ! -d "${worktree}/.git" ]]; then
    mkdir -p -- "$(dirname -- "${worktree}")"
    git clone --filter=blob:none --no-checkout "${UPSTREAM_URL}" "${worktree}"
    git -C "${worktree}" fetch --depth=1 origin "${UPSTREAM_COMMIT}"
    git -C "${worktree}" checkout --detach "${UPSTREAM_COMMIT}"
fi

actual_commit="$(git -C "${worktree}" rev-parse HEAD)"
if [[ "${actual_commit}" != "${UPSTREAM_COMMIT}" ]]; then
    printf 'error: prepared source is at %s, expected %s\n' "${actual_commit}" "${UPSTREAM_COMMIT}" >&2
    printf 'Remove %s and run this script again.\n' "${worktree}" >&2
    exit 1
fi

if git -C "${worktree}" apply --reverse --check "${patch_file}" >/dev/null 2>&1; then
    printf 'Reroll source is already prepared at %s\n' "${worktree}"
    exit 0
fi

if [[ -n "$(git -C "${worktree}" status --short)" ]]; then
    printf 'error: prepared source has changes that do not match the current patch\n' >&2
    printf 'Preserve your work, remove %s, and run this script again.\n' "${worktree}" >&2
    exit 1
fi

git -C "${worktree}" apply --check "${patch_file}"
git -C "${worktree}" apply --whitespace=error-all "${patch_file}"
printf 'Prepared Reroll source at %s\n' "${worktree}"
