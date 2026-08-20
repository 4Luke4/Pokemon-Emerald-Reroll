#!/usr/bin/env bash

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
upstream_url="${UPSTREAM_URL:-https://github.com/pret/pokeemerald.git}"
resolved_commit="$("${repository_root}/scripts/resolve-upstream.sh")"
worktree="${REROLL_WORKTREE:-${repository_root}/build/pokeemerald}"
upstream_patch_directory="${repository_root}/patches/upstream"
integration_patch_directory="${repository_root}/patches/integration"
overlay_directory="${repository_root}/overlay"
manifest="${worktree}.reroll-prepared"

if [[ ! -d "${upstream_patch_directory}"
   || ! -d "${integration_patch_directory}"
   || ! -d "${overlay_directory}" ]]; then
    printf 'error: missing upstream patches, integration patches, or source overlay\n' >&2
    exit 1
fi

# Python keeps the input fingerprint identical on GNU/Linux and macOS, whose
# bundled sort and checksum utilities expose different command-line options.
fingerprint="$(
    cd "${repository_root}"
    python3 scripts/fingerprint.py patches overlay
)"
expected_manifest="${resolved_commit} ${fingerprint}"

if [[ -d "${worktree}/.git"
   && -f "${manifest}"
   && "$(<"${manifest}")" == "${expected_manifest}"
   && "$(git -C "${worktree}" rev-parse HEAD)" == "${resolved_commit}" ]]; then
    printf 'Reroll source is already prepared at %s (%s)\n' "${worktree}" "${resolved_commit}"
    exit 0
fi

if [[ ! -d "${worktree}/.git" ]]; then
    mkdir -p "$(dirname "${worktree}")"
    git clone --filter=blob:none --no-checkout "${upstream_url}" "${worktree}"
else
    if [[ -n "$(git -C "${worktree}" status --short)" ]]; then
        printf 'error: prepared source differs from the current overlay or upstream revision\n' >&2
        printf 'Preserve any work, remove %s, and run this script again.\n' "${worktree}" >&2
        exit 1
    fi
fi

git -C "${worktree}" fetch --depth=1 origin "${resolved_commit}"
git -C "${worktree}" checkout --detach "${resolved_commit}"
actual_commit="$(git -C "${worktree}" rev-parse HEAD)"
if [[ "${actual_commit}" != "${resolved_commit}" ]]; then
    printf 'error: prepared source is at %s, expected %s\n' "${actual_commit}" "${resolved_commit}" >&2
    exit 1
fi

# Apply isolated upstream hardening before gameplay integration so a failing
# hunk identifies the ownership boundary that needs maintenance.
for patch_file in \
    "${upstream_patch_directory}"/*.patch \
    "${integration_patch_directory}"/*.patch; do
    git -C "${worktree}" apply --check "${patch_file}"
    git -C "${worktree}" apply --whitespace=error-all "${patch_file}"
done

mkdir -p "${worktree}/include/reroll" "${worktree}/src/reroll"
cp -R "${overlay_directory}/include/." "${worktree}/include/"
cp -R "${overlay_directory}/src/." "${worktree}/src/"
printf '%s\n' "${expected_manifest}" > "${manifest}"
printf 'Prepared Reroll source at %s (%s)\n' "${worktree}" "${resolved_commit}"
