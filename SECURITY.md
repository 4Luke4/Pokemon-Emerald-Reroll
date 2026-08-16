# Security Policy

## Supported Versions

Security fixes are provided for the current minor release and the `main` branch.

| Version | Supported |
| --- | --- |
| `main` | Yes |
| `0.1.x` | Yes |
| `< 0.1.0` | No |

## System and Scope

Pokémon Emerald: Reroll is a source patch, local build toolchain, and set of GitHub Actions workflows. It is not a hosted service and does not publish ROM artifacts. Security review covers:

- `patches/reroll.patch` and the C/event-script code it applies;
- local preparation, build, and verification scripts;
- GitHub Actions, Dependabot, issue forms, and release automation; and
- parsing or state transitions that can corrupt memory, escape intended save boundaries, or execute untrusted code during a build.

The pinned `pret/pokeemerald` source, ARM toolchain, GitHub-hosted runners, emulator, flash hardware, and player-supplied game materials are external trust boundaries.

## Threat Model and Trust Boundaries

Treat pull-request content, branch names, commit messages, patch contents, local paths, environment variables, save data, link-battle data, and upstream build inputs as untrusted. Repository maintainers and explicitly approved GitHub workflows are trusted only within their documented permissions.

The primary assets are repository integrity, workflow credentials, maintainer accounts, contributor worktrees, deterministic upstream selection, memory safety on the GBA, and save data outside the challenge's intentional permadeath path.

## Security Invariants

- Preparation must fetch and check out the exact upstream commit documented in `UPSTREAM.md`; a mismatched or dirty worktree fails closed.
- A patch must pass `git apply --check` before mutation and must not write outside the generated `build/pokeemerald` worktree.
- Pull-request workflows that execute contributor code have read-only repository permissions.
- `pull_request_target` workflows must never check out, source, or execute the pull request's head revision. Dependabot auto-merge is restricted to the verified `dependabot[bot]` actor and patch/minor updates, with required checks enforced by GitHub auto-merge.
- Build logs and artifacts must not contain ROMs, saves, tokens, credentials, or personal data. CI may retain a checksum, never the compiled `.gba`.
- Random selection, species/move table traversal, party writes, sprite allocation, and save operations must remain within their declared bounds.
- Save erasure may occur only after a genuine non-link player defeat or an explicit user-initiated erase flow.
- A failed verification or build step must stop the workflow rather than publishing or merging an unverified result.

## Reportable Findings and Severity Context

Please report vulnerabilities that plausibly enable:

- workflow-token theft, unauthorized repository mutation, or execution of untrusted pull-request code with write permissions;
- bypass of the pinned upstream revision or patch-integrity checks;
- path traversal or unintended file mutation outside generated build directories;
- committed or uploaded ROMs, secrets, sensitive save data, or other prohibited artifacts;
- attacker-controlled out-of-bounds access, memory corruption, or arbitrary code execution in the patched game; or
- save erasure or durable save corruption outside the documented permadeath rule.

Severity depends on realistic reachability and impact. Credential compromise, default-branch mutation, or reliably attacker-controlled code execution is high or critical. A crash requiring a deliberately malformed unsupported save with no broader impact is normally low.

## Out of Scope and Accepted Risk

The following are not security findings by themselves:

- gameplay balance, random outcomes, legal-moveset quality, or ordinary emulator incompatibility;
- the documented lack of a hardware entropy source on Game Boy Advance;
- intentional save erasure following a legitimate defeat;
- vulnerabilities solely in unmodified upstream code without a demonstrated Reroll-specific reachable impact;
- Nintendo, Creatures, GAME FREAK, emulator, or hardware issues outside this repository's control; and
- requests for ROMs, copyrighted assets, legal advice, or license exceptions.

Do not use these exclusions to suppress a finding that crosses a listed trust boundary or violates a security invariant.

## Known Limitations and Compensating Controls

- ChaCha20 provides a strong random stream, but seed entropy is limited to timing and device/game state because the GBA has no operating-system CSPRNG. Rejection sampling prevents modulo bias; documentation avoids claiming modern hardware entropy guarantees.
- GitHub Actions use reviewed major-version tags so Dependabot can maintain them. Minimal permissions, actor checks, protected auto-merge, CodeQL, and build verification reduce the risk of mutable action tags.
- ROM header and SHA-256 checks establish build structure and identity, not legal provenance or reproducible equivalence across compiler versions.
- The project is pre-`1.0`; memory- and save-sensitive changes require emulator testing and real-hardware evidence where applicable.

## Reporting a Vulnerability

Use [GitHub private vulnerability reporting](https://github.com/4Luke4/Pokemon-Emerald-Reroll/security/advisories/new). Do not open a public issue for a suspected vulnerability.

Include the affected commit, environment, impact, reproduction steps, and the smallest safe proof of concept. Do not attach a ROM, credentials, unlawful assets, or personal save data. You can expect acknowledgement within seven calendar days and a status update at least every fourteen days while the report is active.

Please allow a reasonable remediation window before public disclosure. Reports may be closed as not applicable when they are not reproducible, affect only an explicitly unsupported environment, or fall entirely outside the scope above; the response will explain that decision.
