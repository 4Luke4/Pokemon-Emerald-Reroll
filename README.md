# Pokémon Emerald: Reroll

[![Build and verify ROM](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/build.yml/badge.svg?branch=main&event=push)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/build.yml?query=branch%3Amain)
[![CodeQL](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/codeql.yml/badge.svg?branch=main&event=push)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/codeql.yml?query=branch%3Amain)
[![Lint](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/super-linter.yml/badge.svg?branch=main&event=push)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/super-linter.yml?query=branch%3Amain)
[![Conventional Commits](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/conventional-commits.yml/badge.svg?branch=main&event=push)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/conventional-commits.yml?query=branch%3Amain)
[![Upstream compatibility](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/upstream-compatibility.yml/badge.svg?branch=main)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/actions/workflows/upstream-compatibility.yml?query=branch%3Amain)
[![GitHub release](https://img.shields.io/github/v/release/4Luke4/Pokemon-Emerald-Reroll?include_prereleases)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases)
[![Total release downloads](https://img.shields.io/github/downloads/4Luke4/Pokemon-Emerald-Reroll/total?label=downloads)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases)
[![Latest release downloads](https://img.shields.io/github/downloads/4Luke4/Pokemon-Emerald-Reroll/latest/total?label=latest%20downloads)](https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases/latest)
[![Version](https://img.shields.io/badge/version-v0.2.1-2ea44f)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-proprietary-red)](LICENSE.md)

Pokémon Emerald: Reroll is a permadeath, progression-scaled challenge mode built as a source patch for the [`pret/pokeemerald`](https://github.com/pret/pokeemerald) decompilation. The original story, maps, battles, and core Generation III mechanics remain in place; roster construction, progression, item access, and selected quality-of-life systems are changed deliberately.

> [!IMPORTANT]
> Source control does not contain a Pokémon Emerald ROM. A maintainer may use
> the manual release workflow only after confirming that the repository is
> authorized to publish its output. Build, download, and distribute game
> material only when you have the necessary rights and comply with applicable
> law.

## Challenge rules

| Area | Reroll behavior |
| --- | --- |
| Player party | Always six unique shiny Pokémon, rebuilt before the first battle and after every Gym Leader, Elite Four, or Champion victory. |
| Player level | All six Pokémon use the floor of the next progression boss's vanilla average level. Experience, EV gain, and Rare Candy leveling are disabled. |
| Species | Any valid species is eligible through level 50. Above level 50, only species with no further evolution are eligible. Legendaries remain eligible. |
| Moves and abilities | Moves come only from the species' level-up, compatible TM/HM, and egg-move pools. A reliable STAB attack is anchored when legally available; useful support, coverage, species stats, and move-pair synergy guide weighted selection for the remaining slots. Abilities are selected only from the species' legal slots. |
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
- The first conscious party member's animated icon follows the player in the overworld.

## Randomness model

Reroll uses a ChaCha20 stream generator and rejection sampling for unbiased bounded selection. Its seed mixes frame and scanline timing, the vanilla generator, trainer identity, and play-time counters. This is substantially stronger than the game's linear congruential generator.

A Game Boy Advance has no operating-system CSPRNG or dedicated hardware entropy source. Accordingly, the generator is cryptographic-grade, but the physical device cannot provide the same entropy guarantees as a modern operating system. The implementation makes that limitation explicit rather than claiming impossible hardware guarantees.

## Build

The build scripts support Linux, Windows through WSL, and macOS. Each platform
needs Git, Python 3.11 or newer, GNU Make, a host C compiler, libpng,
`pkg-config`, and a complete `arm-none-eabi` toolchain with newlib.

### Linux (Debian or Ubuntu)

Install the packages used by CI:

```sh
sudo apt-get update
sudo apt-get install --yes \
  binutils-arm-none-eabi \
  build-essential \
  gcc-arm-none-eabi \
  git \
  libnewlib-arm-none-eabi \
  libpng-dev \
  pkg-config \
  python3
```

Clone and build:

```sh
git clone https://github.com/4Luke4/Pokemon-Emerald-Reroll.git
cd Pokemon-Emerald-Reroll
./scripts/prepare.sh
./scripts/build-rom.sh
```

### Windows 10 or 11 (WSL 2)

Reroll's build entry points are Bash scripts, so the supported Windows path is
[WSL 2 with Ubuntu](https://learn.microsoft.com/windows/wsl/install). Open
PowerShell as Administrator, install WSL, and restart Windows if prompted:

```powershell
wsl --install -d Ubuntu
```

Open the installed Ubuntu terminal and install the Linux dependencies:

```sh
sudo apt-get update
sudo apt-get install --yes \
  binutils-arm-none-eabi \
  build-essential \
  gcc-arm-none-eabi \
  git \
  libnewlib-arm-none-eabi \
  libpng-dev \
  pkg-config \
  python3
```

Keep the checkout in the WSL file system for predictable permissions and build
performance, then build it from the same Ubuntu terminal:

```sh
mkdir -p ~/src
cd ~/src
git clone https://github.com/4Luke4/Pokemon-Emerald-Reroll.git
cd Pokemon-Emerald-Reroll
./scripts/prepare.sh
./scripts/build-rom.sh
```

The result is available to Windows Explorer at
`\\wsl$\Ubuntu\home\<your-wsl-user>\src\Pokemon-Emerald-Reroll\dist`.

### macOS

Install Apple's command-line developer tools, then use
[Homebrew](https://brew.sh/) for the host dependencies:

```sh
xcode-select --install
brew install git libpng pkgconf python
```

The Homebrew cross compiler is intentionally not used because it is built
without target C-library headers. Install the complete devkitARM toolchain from
the official [devkitPro pacman release](https://github.com/devkitPro/pacman/releases/latest),
open the downloaded `.pkg`, and then run:

```sh
sudo dkp-pacman -Syu
sudo dkp-pacman -S gba-dev devkitarm-rules
```

Make devkitARM visible to the current shell and future zsh sessions:

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITARM="${DEVKITPRO}/devkitARM"
printf '%s\n' \
  'export DEVKITPRO=/opt/devkitpro' \
  'export DEVKITARM="${DEVKITPRO}/devkitARM"' >> ~/.zprofile
```

Clone and build:

```sh
git clone https://github.com/4Luke4/Pokemon-Emerald-Reroll.git
cd Pokemon-Emerald-Reroll
./scripts/prepare.sh
./scripts/build-rom.sh
```

### Build output and verification

The local result is `dist/pokemon-emerald-reroll-v0.2.1.gba`. It is intentionally
ignored by Git. Push, pull-request, scheduled, and ordinary manually dispatched
checks retain only its checksum and exact upstream SHA; only the protected,
manual release workflow can attach a verified ROM to a GitHub release.

`pret/pokeemerald` currently publishes no releases or tags. Reroll therefore
defines "latest stable" as the advertised commit of its protected default
branch, whose upstream protection requires the build check. The resolver freezes
that commit for the entire job and records it in `dist/upstream.sha`. To reproduce
a prior build, set `UPSTREAM_REF` to the recorded 40-character commit.

Run source-policy checks without compiling:

```sh
python3 scripts/verify-source.py
```

## Source layout

- `overlay/src/reroll/` contains standalone, commented C modules for each feature.
- `overlay/include/reroll/` contains the public and internal module interfaces.
- `patches/integration/` contains only small, topic-specific hooks into upstream.
- `patches/upstream/` contains isolated upstream host-tool hardening carried by
  Reroll until equivalent corrections land in `pret/pokeemerald`.
- `scripts/resolve-upstream.sh` resolves one immutable upstream SHA per build.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the modular development workflow and
[UPSTREAM.md](UPSTREAM.md) for the upstream stability policy.

## Releases

Maintainers create releases through the manual-only `Create ROM release`
workflow. It accepts either a stable tag matching `v[0-9].[0-9].[0-9]` or a
pre-release tag matching `v[0-9].[0-9].[0-9]-(alpha|beta|pre).[0-9]`. The run
must originate from `main`, requires an explicit rights attestation, builds
against one frozen upstream SHA, validates the ROM, creates an annotated tag,
and publishes the ROM with SHA-256 and upstream-revision manifests.

The download badges count every asset attached to a published release. The
general count spans all releases; the latest-release count follows GitHub's
latest stable release and excludes pre-releases.

See the [workflow responsibility matrix](.github/workflows/README.md) for the
non-overlapping trigger and cache policy.

## Compatibility and scope

- Base: English Pokémon Emerald, revision 0.
- Upstream: the latest build-protected `pret/pokeemerald` default-branch commit, resolved and frozen per build.
- Save files from vanilla Emerald are unsupported. Begin with a fresh save.
- Link, Battle Frontier, e-Reader, Trainer Hill, and Secret Base party generation retain their upstream implementations.
- Version `v0.2.1` hardens upstream host-tool allocation and format handling,
  preserves least-privilege lint automation, and isolates upstream corrections
  from Reroll gameplay hooks. It should be play-tested on both mGBA and real
  hardware before being treated as tournament-stable.

## Project policy

- Bugs and proposals belong in the supplied GitHub issue forms.
- Security concerns must follow [SECURITY.md](SECURITY.md), not a public issue.
- Changes use [Conventional Commits](https://www.conventionalcommits.org/) and update [CHANGELOG.md](CHANGELOG.md) when user-visible.
- Original Reroll work is governed by [LICENSE.md](LICENSE.md). Upstream and third-party rights are described in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Pokémon and Pokémon Emerald are trademarks of Nintendo, Creatures Inc., and GAME FREAK inc. This is an unofficial fan project and is not affiliated with or endorsed by those companies.
