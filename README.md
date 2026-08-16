# Pokémon Emerald: Reroll

[![Build and verify ROM](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/build.yml/badge.svg)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/build.yml)
[![CodeQL](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/codeql.yml/badge.svg)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/codeql.yml)
[![Lint](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/super-linter.yml/badge.svg)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/super-linter.yml)
[![Version](https://img.shields.io/badge/version-v0.1.0-2ea44f)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-proprietary-red)](LICENSE.md)

Pokémon Emerald: Reroll is a permadeath, progression-scaled challenge mode built as a source patch for the [`pret/pokeemerald`](https://github.com/pret/pokeemerald) decompilation. The original story, maps, battles, and core Generation III mechanics remain in place; roster construction, progression, item access, and selected quality-of-life systems are changed deliberately.

> [!IMPORTANT]
> This project does not include a Pokémon Emerald ROM and does not distribute built ROM artifacts. Build only from lawfully obtained materials and comply with the laws that apply where you live.

## Challenge rules

| Area | Reroll behavior |
| --- | --- |
| Player party | Always six unique shiny Pokémon, rebuilt before the first battle and after every Gym Leader, Elite Four, or Champion victory. |
| Player level | All six Pokémon use the floor of the next progression boss's vanilla average level. Experience, EV gain, and Rare Candy leveling are disabled. |
| Species | Any valid species is eligible through level 50. Above level 50, only species with no further evolution are eligible. Legendaries remain eligible. |
| Moves and abilities | Moves come only from the species' level-up, compatible TM/HM, and egg-move pools. Four scored moves are preferred. Abilities are selected only from the species' legal slots. |
| Trainer parties | Vanilla slot levels are preserved. Important trainers receive six Pokémon; added slots use that trainer's vanilla average level. |
| Trainer difficulty | Ordinary trainers use bad-move and viability checks. Important trainers add KO planning, first-turn setup, HP awareness, six useful held items, and four usable trainer items. |
| Items | Ordinary field pickups are rerolled from a battle-useful allowlist. Plot keys and HMs retain their identities. Ball gifts become battle items; balls cannot be stored or bought. |
| Battle style | True Set mode. The Shift prompt is never offered, and the option is locked to Set. |
| Defeat | A whiteout erases both save slots and resets the game. A new run receives a new random stream. |

## Quality-of-life changes

- Running is allowed indoors wherever the destination tile is safe.
- Repel expiration offers to consume another available Repel, preferring the longest duration.
- Story field moves are virtual capabilities after their normal badge checks and do not consume move slots.
- Interacting with Cut, Rock Smash, Strength, Surf, Waterfall, and Dive obstacles executes the action without a confirmation prompt.
- The lead party member's animated icon follows the player in the overworld.

## Randomness model

Reroll uses a ChaCha20 stream generator and rejection sampling for unbiased bounded selection. Its seed mixes frame and scanline timing, the vanilla generator, trainer identity, and play-time counters. This is substantially stronger than the game's linear congruential generator.

A Game Boy Advance has no operating-system CSPRNG or dedicated hardware entropy source. Accordingly, the generator is cryptographic-grade, but the physical device cannot provide the same entropy guarantees as a modern operating system. The implementation makes that limitation explicit rather than claiming impossible hardware guarantees.

## Build

### Prerequisites

- Git
- Python 3.11 or newer
- GNU Make and a C build toolchain
- `gcc-arm-none-eabi` and `binutils-arm-none-eabi`
- `libpng` development headers and `pkg-config`

On Debian or Ubuntu:

```sh
sudo apt-get update
sudo apt-get install --yes build-essential gcc-arm-none-eabi binutils-arm-none-eabi libpng-dev pkg-config python3 git
```

Prepare the pinned upstream source, apply the Reroll patch, build with the modern toolchain, and validate the GBA header:

```sh
./scripts/prepare.sh
./scripts/build-rom.sh
```

The local result is `dist/pokemon-emerald-reroll-v0.1.0.gba`. It is intentionally ignored by Git and never uploaded by CI.

Run source-policy checks without compiling:

```sh
python3 scripts/verify-source.py
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for the patch-development workflow and upstream-pin maintenance requirements.

## Compatibility and scope

- Base: English Pokémon Emerald, revision 0.
- Upstream pin: `pret/pokeemerald` commit `9a83a2bbe8e097e62c00f1dbd56849766775d7b6`.
- Save files from vanilla Emerald are unsupported. Begin with a fresh save.
- Link, Battle Frontier, e-Reader, Trainer Hill, and Secret Base party generation retain their upstream implementations.
- Version `v0.1.0` is the initial scaffolding release and should be play-tested on both mGBA and real hardware before being treated as tournament-stable.

## Project policy

- Bugs and proposals belong in the supplied GitHub issue forms.
- Security concerns must follow [SECURITY.md](SECURITY.md), not a public issue.
- Changes use [Conventional Commits](https://www.conventionalcommits.org/) and update [CHANGELOG.md](CHANGELOG.md) when user-visible.
- Original Reroll work is governed by [LICENSE.md](LICENSE.md). Upstream and third-party rights are described in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Pokémon and Pokémon Emerald are trademarks of Nintendo, Creatures Inc., and GAME FREAK inc. This is an unofficial fan project and is not affiliated with or endorsed by those companies.
