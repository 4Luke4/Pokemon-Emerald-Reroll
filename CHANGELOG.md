# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). Commit subjects follow [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

## [Unreleased]

## [0.3.0] - 2026-08-26

### Added

- Added credited, species-indexed 32×32 HGSS-style overworld sheets for every
  valid Generation I–III species and all 28 Unown forms.
- Added normal and shiny follower palette selection from Emerald's canonical
  species palette tables.
- Added A-button follower conversations driven by the lead Pokémon's current
  status, HP, and friendship, with native heart, question, and exclamation
  emotes.
- Added a dependency manifest and a daily, manually dispatchable workflow that
  detects merrp's latest stable GitHub Release, verifies and builds regenerated
  assets in a read-only job, then opens a maintainer-reviewed update pull
  request from a separately permissioned publishing job.

### Changed

- Replaced the party-menu icon follower with a six-frame directional
  overworld sprite that walks the player's recorded route one tile behind.
- Followers now withdraw during biking, surfing, underwater travel, and forced
  movement, matching the relevant HeartGold/SoulSilver visibility behavior.
- Reworked the merrp importer to resolve a supplied release tag or the manifest
  lock instead of embedding a fixed source commit in executable code.

### Fixed

- Reinitialize follower path state after warps and sprite recreation so the
  follower cannot cut across the screen or retain a stale Unown form.

## [0.2.2] - 2026-08-20

### Security

- Changed Super-Linter to deny all workflow-level token permissions and grant
  only `contents: read`, `packages: read`, and `statuses: write` to the lint job.
- Added an independent GitHub Actions CodeQL analysis so workflow findings are
  reevaluated on every scan and the generated tracker can close them reliably.
- Added source-policy checks that prevent either least-privilege configuration
  or the Actions analysis category from being removed accidentally.

## [0.2.1] - 2026-08-20

### Changed

- Added a dedicated `patches/upstream/` layer so carried `pret/pokeemerald`
  corrections remain separate from Reroll's gameplay integration hooks and are
  included in preparation manifests and exact CI cache keys.
- Extended source verification and contributor documentation to enforce the
  upstream-patch ownership boundary.

### Security

- Fixed the five C/C++ CodeQL findings reported in issue #8 by promoting host
  image-allocation arithmetic before multiplication and matching `mid2agb`'s
  variadic format argument to its conversion specifier.
- Declared Super-Linter's repository token permissions explicitly as
  `contents: read`, `packages: read`, and `statuses: write`.

## [0.2.0] - 2026-08-20

### Added

- Linux, Windows 10/11 through WSL 2, and macOS build instructions, backed by a
  cross-platform source-fingerprint helper used during preparation and caching.
- A repository `VERSION` source of truth validated against build output,
  documentation, security support policy, and release tags.
- A dedicated post-analysis workflow that maintains one issue containing every
  open default-branch CodeQL finding, reopens it when findings recur, and closes
  it only after a successful clean analysis.
- A weekly compatibility workflow that builds against the latest protected upstream default-branch commit and records the resolved SHA.
- A manual-only release workflow that validates stable and pre-release tags,
  creates an annotated tag, and publishes the verified ROM with checksum and
  upstream-revision manifests.
- A workflow responsibility matrix documenting every trigger and exclusive
  assurance.
- README badges for default-branch build, CodeQL, lint, Conventional Commit,
  and upstream-compatibility status, plus all-release and latest stable-release
  asset download counts.

### Changed

- Reworked moveset construction around a dependable legal STAB attack, one
  useful support preference, and weighted random coverage/support choices based
  on species stats, types, drawbacks, and compatible move pairs.
- Updated the overworld follower to use the first conscious party member and to
  share the player object's camera-coordinate behavior.
- Replaced the fixed upstream commit with a per-build stable-revision resolver and immutable build record.
- Split the monolithic patch into commented feature modules and small, topic-specific integration patches.
- Removed redundant manual build and CodeQL triggers and moved CodeQL's recurring
  analysis away from the weekly upstream-compatibility schedule.
- Centralized exact-key caches for ordinary, compatibility, and release builds;
  CodeQL remains deliberately uncached to preserve compiler tracing.

### Fixed

- Excluded Emerald's reserved `SPECIES_OLD_UNOWN_*` placeholder range from
  randomized rosters, preventing question-mark opponents with invalid names.
- Prevented the follower icon from drifting away from the player as the camera
  moves, and removed it cleanly when every party member is fainted.

## [0.1.0] - 2026-08-16

### Added

- Initial project scaffolding pinned to a reproducible `pret/pokeemerald` revision.
- Six-member shiny player-party rerolls tied to Gym and Pokémon League progression.
- Level locking, legal scored movesets, legal abilities, and final-evolution filtering above level 50.
- Randomized story-trainer parties with vanilla levels, six-member important-trainer teams, useful held items, trainer items, and scaled AI policy.
- Battle-useful field pickup randomization and complete player Poké Ball exclusion.
- Save-erasing permadeath, true Set battle style, indoor running, modern Repel chaining, virtual HMs, and an overworld lead-Pokémon follower.
- Reproducible build and ROM verification scripts.
- CI, CodeQL, linting, Dependabot, issue forms, pull-request guidance, ownership rules, and repository policy documents.

[Unreleased]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.2.2...v0.3.0
[0.2.2]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases/tag/v0.1.0
