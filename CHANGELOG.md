# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). Commit subjects follow [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

## [Unreleased]

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

[Unreleased]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases/tag/v0.1.0
