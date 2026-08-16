# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). Commit subjects follow [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

## [Unreleased]

### Added

- A weekly compatibility workflow that builds against the latest protected upstream default-branch commit and records the resolved SHA.

### Changed

- Replaced the fixed upstream commit with a per-build stable-revision resolver and immutable build record.
- Split the monolithic patch into commented feature modules and small, topic-specific integration patches.

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

[Unreleased]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/4Luke4/Pokemon-Emerald-Reroll/releases/tag/v0.1.0
