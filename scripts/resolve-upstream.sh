#!/usr/bin/env bash

set -euo pipefail

readonly upstream_repository="${UPSTREAM_REPOSITORY:-pret/pokeemerald}"
readonly upstream_url="${UPSTREAM_URL:-https://github.com/${upstream_repository}.git}"
readonly requested_ref="${UPSTREAM_REF:-}"

if [[ -n "${requested_ref}" ]]; then
    # Explicit immutable SHAs are useful for reproducing an older CI build.
    if [[ "${requested_ref}" =~ ^[0-9a-fA-F]{40}$ ]]; then
        resolved_commit="${requested_ref,,}"
    else
        # Prefer an annotated tag's peeled commit, then an exact branch/tag ref.
        remote_refs="$(git ls-remote "${upstream_url}" \
            "${requested_ref}" \
            "refs/heads/${requested_ref}" \
            "refs/tags/${requested_ref}" \
            "refs/tags/${requested_ref}^{}")"
        resolved_commit="$(awk '$2 ~ /\^\{\}$/ {print $1; found=1} END {if (!found) exit 1}' <<<"${remote_refs}" 2>/dev/null || true)"
        if [[ -z "${resolved_commit}" ]]; then
            resolved_commit="$(awk 'NR == 1 {print $1}' <<<"${remote_refs}")"
        fi
    fi
else
    # pokeemerald currently has no releases or tags. Its protected default branch
    # requires the upstream build check, so the advertised HEAD is our documented
    # definition of the latest stable upstream revision.
    remote_head="$(git ls-remote --symref "${upstream_url}" HEAD)"
    resolved_commit="$(awk '$2 == "HEAD" {print $1}' <<<"${remote_head}")"
fi

if [[ ! "${resolved_commit}" =~ ^[0-9a-f]{40}$ ]]; then
    printf 'error: could not resolve a complete upstream commit from %s\n' "${upstream_url}" >&2
    exit 1
fi

printf '%s\n' "${resolved_commit}"
