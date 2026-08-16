#!/usr/bin/env bash

set -euo pipefail

readonly VERSION="0.1.0"

repository_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
worktree="${repository_root}/build/pokeemerald"
output_directory="${repository_root}/dist"
output_rom="${output_directory}/pokemon-emerald-reroll-v${VERSION}.gba"
job_count="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '2')}"

"${repository_root}/scripts/prepare.sh"
python3 "${repository_root}/scripts/verify-source.py"
make -C "${worktree}" --jobs="${job_count}" modern

mkdir -p -- "${output_directory}"
cp -- "${worktree}/pokeemerald_modern.gba" "${output_rom}"
python3 "${repository_root}/scripts/verify-rom.py" "${output_rom}"
