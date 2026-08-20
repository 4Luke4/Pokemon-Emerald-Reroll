#!/usr/bin/env bash

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
worktree="${REROLL_WORKTREE:-${repository_root}/build/pokeemerald}"
patch_directory="${repository_root}/patches/integration"

if [[ ! -d "${worktree}/.git" ]]; then
    printf 'error: run scripts/prepare.sh before updating integration patches\n' >&2
    exit 1
fi

mkdir -p "${patch_directory}"

# Each patch contains only the minimal hooks for one review domain. New feature
# logic belongs in overlay/src/reroll, never in these generated patch files.
git -C "${worktree}" diff --full-index --no-ext-diff HEAD -- \
    src/battle_ai_script_commands.c src/battle_main.c \
    src/battle_script_commands.c > "${patch_directory}/battle.patch"
git -C "${worktree}" diff --full-index --no-ext-diff HEAD -- \
    src/bike.c src/new_game.c src/option_menu.c src/overworld.c src/pokemon.c \
    > "${patch_directory}/challenge.patch"
git -C "${worktree}" diff --full-index --no-ext-diff HEAD -- \
    src/scrcmd.c src/field_player_avatar.c > "${patch_directory}/hms.patch"
git -C "${worktree}" diff --full-index --no-ext-diff HEAD -- \
    src/item.c src/shop.c > "${patch_directory}/items.patch"
git -C "${worktree}" diff --full-index --no-ext-diff HEAD -- \
    asm/macros/event.inc data/scripts/field_move_scripts.inc \
    data/scripts/repel.inc data/scripts/surf.inc data/specials.inc \
    > "${patch_directory}/scripts.patch"

printf 'Updated topic-specific patches in %s\n' "${patch_directory}"
