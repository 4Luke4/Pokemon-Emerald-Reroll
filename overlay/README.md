# Reroll source overlay

The overlay contains original Reroll source files copied into a clean upstream
worktree by `scripts/prepare.sh`. Keeping feature logic here makes it readable,
lintable, and reviewable without decoding a generated patch.

| Module | Responsibility |
| --- | --- |
| `src/reroll/rng.c` | ChaCha20 stream, entropy mixing, and unbiased ranges |
| `src/reroll/pokemon.c` | Species eligibility, legal strategy-aware moves, abilities, shininess |
| `src/reroll/progression.c` | Six-member player party, levels, boss rerolls |
| `src/reroll/trainers.c` | Trainer rosters, levels, items, and AI tiers |
| `src/reroll/items.c` | Ball exclusion, marts, pickups, and Repel chaining |
| `src/reroll/hms.c` | Virtual field-move users |
| `src/reroll/follower.c` | First-conscious-Pokémon HGSS follower lifecycle, path trail, animation, interaction dialogue, and emotes |
| `src/reroll/follower_graphics.inc.c` | Species-indexed HGSS follower sheet declarations |
| `data/scripts/reroll_follower.inc` | Lock-safe field-message lifecycle for follower interactions |
| `graphics/reroll/followers/` | 32×32 six-frame Gen I–III overworld sheets and Unown forms |
| `src/reroll/permadeath.c` | Save erasure and reset after defeat |

`include/reroll/reroll.h` is the narrow public interface consumed by upstream
hooks. `include/reroll/internal.h` is shared only between overlay modules.

New gameplay behavior should be implemented in the smallest relevant module.
Integration patches should contain only the call site needed to enter that API.
