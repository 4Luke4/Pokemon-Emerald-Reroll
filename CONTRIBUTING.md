# Contributing to Pokémon Emerald: Reroll

Thank you for improving Reroll. Contributions should preserve the vanilla story, keep challenge rules deterministic at their boundaries, and remain reproducible from the pinned upstream commit.

## Before opening a pull request

1. Search existing issues and pull requests.
2. Discuss large gameplay, save-format, licensing, or architecture changes in a feature request first.
3. Do not disclose vulnerabilities publicly; follow `SECURITY.md`.
4. Confirm that every file you submit is yours to license. Never commit ROMs, save files, extracted assets from an unlawful copy, credentials, or personal data.

## Development setup

Install the prerequisites in `README.md`, then run:

```sh
git clone https://github.com/4Luke4/Pokemon-Emerald-Reroll.git
cd Pokemon-Emerald-Reroll
git config commit.template .gitmessage
./scripts/prepare.sh
```

The prepared, patched decompilation lives at `build/pokeemerald`. Edit and test there. When the source change is ready, regenerate the repository patch:

```sh
./scripts/update-patch.sh
python3 scripts/verify-source.py
./scripts/build-rom.sh
```

`update-patch.sh` includes only differences from the pinned upstream commit. Review `patches/reroll.patch` before committing; generated noise usually indicates an accidental tool or asset change.

## Design requirements

- Preserve story flags, scripts, maps, and vanilla encounter sequencing unless an approved issue explicitly changes them.
- Keep normal story-trainer levels equal to their vanilla slots. Six-member expansion slots use the trainer's vanilla average.
- Generate moves only from legal level-up, compatible TM/HM, or egg-move sources.
- Keep species filtering consistent at the level-50 boundary.
- Keep player Pokémon shiny, level-locked, and exactly six after every invariant check.
- Do not weaken save erasure on defeat or re-enable Shift prompts and Poké Balls.
- Use bounded, unbiased selection through the Reroll random stream; do not call the vanilla RNG for gameplay selection.
- Document hardware or emulator assumptions explicitly.

## Tests and evidence

Every pull request should include the smallest relevant evidence:

- `python3 scripts/verify-source.py`
- `./scripts/build-rom.sh`
- mGBA smoke test for the changed flow
- real-hardware result when timing, flash save, audio, DMA, or sprite limits are affected

For gameplay changes, list the maps or trainers exercised and whether a fresh save was used. Never upload the resulting `.gba` file; attach logs or the SHA-256 digest instead.

## Commits

Use Conventional Commits:

```text
feat(roster): reroll the party after league victories
fix(items): exclude ball gifts from challenge runs
balance(ai): improve ordinary trainer viability checks
```

Keep commits focused and buildable. Rebase fixups before requesting review. Update `CHANGELOG.md` under `Unreleased` for user-visible behavior.

## Pull requests

- Complete the pull-request template.
- Link the issue that explains the change.
- Keep unrelated formatting and generated changes out of the patch.
- Explain save compatibility and balancing implications.
- Resolve review conversations with code, tests, or concrete reasoning.
- Do not merge your own change unless repository policy explicitly permits it.

## Contribution license

By submitting a contribution, you agree to the contribution grant in `LICENSE.md` and certify that you have the right to submit the work. If you cannot make that grant, do not submit the contribution; open a discussion with the maintainer instead.
