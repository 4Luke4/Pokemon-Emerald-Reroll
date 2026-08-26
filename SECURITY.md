# Security Policy

## Supported Versions

Security fixes are provided for the current minor release and the `main` branch.

| Version | Supported |
| --- | --- |
| `main` | Yes |
| `0.3.x` | Yes |
| `< 0.3.0` | No |

## System and Scope

Pokémon Emerald: Reroll is a modular source overlay, integration-patch set,
local build toolchain, and set of GitHub Actions workflows. It is not a hosted
service. Ordinary CI does not publish ROM artifacts; the manually dispatched,
maintainer-authorized release workflow is the sole publication path. Security
review covers:

- standalone feature code under `overlay/`, topic-specific hooks under
  `patches/integration/`, and explicitly carried hardening under
  `patches/upstream/`;
- local preparation, build, and verification scripts;
- GitHub Actions, Dependabot, issue forms, and release automation; and
- parsing or state transitions that can corrupt memory, escape intended save boundaries, or execute untrusted code during a build.

The dynamically resolved `pret/pokeemerald` source, ARM toolchain, GitHub-hosted runners, emulator, flash hardware, and player-supplied game materials are external trust boundaries.

## Threat Model and Trust Boundaries

Treat pull-request content, branch names, commit messages, patch contents, local paths, environment variables, save data, link-battle data, and upstream build inputs as untrusted. Repository maintainers and explicitly approved GitHub workflows are trusted only within their documented permissions.

The primary assets are repository integrity, workflow credentials, maintainer accounts, contributor worktrees, deterministic upstream selection, memory safety on the GBA, and save data outside the challenge's intentional permadeath path.

## Security Invariants

- Preparation must resolve one complete upstream SHA from the protected default branch, freeze it for the job, check out exactly that commit, and record it for reproduction; a mismatched or dirty worktree fails closed.
- Every integration patch must pass `git apply --check` before mutation. Patch and overlay installation must remain inside the selected generated worktree.
- Pull-request workflows that execute contributor code have read-only repository permissions.
- CodeQL issue synchronization may write only the repository-owned tracker after
  successful default-branch Actions and C/C++ analyses; it must never treat a
  failed or cancelled analysis job as a clean result.
- `pull_request_target` workflows must never check out, source, or execute the pull request's head revision. Dependabot auto-merge is restricted to the verified `dependabot[bot]` actor and patch/minor updates, with required checks enforced by GitHub auto-merge.
- Push, pull-request, scheduled, and non-release manual workflow artifacts must
  not contain ROMs, saves, tokens, credentials, or personal data. They may
  retain checksums and upstream metadata, never the compiled `.gba`.
- The manual release workflow may publish a `.gba` only from `main`, after exact
  tag-format validation, explicit rights attestation, source/ROM verification,
  and creation or verification of a tag at the dispatched commit.
- Random selection, species/move table traversal, party writes, sprite allocation, and save operations must remain within their declared bounds.
- Save erasure may occur only after a genuine non-link player defeat or an explicit user-initiated erase flow.
- A failed verification or build step must stop the workflow rather than publishing or merging an unverified result.

## Reportable Findings and Severity Context

Please report vulnerabilities that plausibly enable:

- workflow-token theft, unauthorized repository mutation, or execution of untrusted pull-request code with write permissions;
- bypass of upstream SHA resolution, per-job revision freezing, or integration-patch checks;
- path traversal or unintended file mutation outside generated build directories;
- committed ROMs, unauthorized ROM uploads, secrets, sensitive save data, or
  other prohibited artifacts;
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

The public CodeQL tracker is generated exclusively from repository-owned scans.
It does not replace private vulnerability reporting and must not be used to
publish externally discovered or embargoed vulnerability details.

## Known Limitations and Compensating Controls

- ChaCha20 provides a strong random stream, but seed entropy is limited to timing and device/game state because the GBA has no operating-system CSPRNG. Rejection sampling prevents modulo bias; documentation avoids claiming modern hardware entropy guarantees.
- GitHub Actions use reviewed major-version tags so Dependabot can maintain them. Minimal permissions, actor checks, protected auto-merge, CodeQL, and build verification reduce the risk of mutable action tags.
- Upstream's moving default branch is an accepted dependency boundary.
  Protection requires its build check; Reroll resolves it once per job, records
  the immutable SHA, and runs weekly compatibility builds so breakage fails
  visibly without publishing a ROM.
- Narrow corrections that Reroll must carry for upstream host tools are isolated
  under `patches/upstream/`, applied before gameplay integration, and included
  in exact preparation and cache fingerprints. Equivalent upstream fixes remove
  the local patch rather than creating a permanent fork.
- Prepared build caches use exact keys covering the upstream SHA, feature
  sources, integration hooks, build verification scripts, runner architecture,
  and compiler versions. There are no fallback keys. CodeQL builds are uncached
  so compiler tracing cannot be bypassed by restored object files. Final ROM,
  ELF, map, and save outputs are explicitly excluded from caches.
- ROM header and SHA-256 checks establish build structure and identity, not legal provenance or reproducible equivalence across compiler versions.
- The project is pre-`1.0`; memory- and save-sensitive changes require emulator testing and real-hardware evidence where applicable.

## Reporting a Vulnerability

Use [GitHub private vulnerability reporting](https://github.com/4Luke4/Pokemon-Emerald-Reroll/security/advisories/new). Do not open a public issue for a suspected vulnerability.

Include the affected commit, environment, impact, reproduction steps, and the smallest safe proof of concept. Do not attach a ROM, credentials, unlawful assets, or personal save data. You can expect acknowledgement within seven calendar days and a status update at least every fourteen days while the report is active.

Please allow a reasonable remediation window before public disclosure. Reports may be closed as not applicable when they are not reproducible, affect only an explicitly unsupported environment, or fall entirely outside the scope above; the response will explain that decision.
