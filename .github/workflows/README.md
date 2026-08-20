# Workflow responsibilities

Each workflow owns one responsibility. Multiple checks may run for the same pull
request, but no two workflows produce the same assurance or publish the same
artifact.

| Workflow | Trigger | Exclusive responsibility |
| --- | --- | --- |
| `build.yml` | Push or pull request to `main` | Compile and validate the ROM; retain only its checksum and upstream SHA. |
| `codeql.yml` | Push or pull request to `main`; monthly schedule | Analyze GitHub Actions and run an uncached, instrumented C/C++ security analysis. |
| `codeql-issue.yml` | Successful default-branch CodeQL completion; manual | Synchronize one issue with every open default-branch CodeQL finding and close it after a clean analysis. |
| `super-linter.yml` | Push or pull request to `main`; manual | Lint changed source, scripts, documentation, and workflow definitions. |
| `conventional-commits.yml` | Push to `main`; pull-request lifecycle | Validate Conventional Commit subjects. |
| `upstream-compatibility.yml` | Weekly schedule; manual | Test the latest protected upstream revision independently of repository changes. |
| `release.yml` | Manual only | Validate a version, create an annotated tag, and publish the verified ROM release. |
| `dependabot-automerge.yml` | Dependabot pull-request lifecycle | Enable auto-merge only for eligible dependency updates. |
| `label.yml` | Pull-request lifecycle | Apply path-based labels without executing pull-request code. |
| `stale.yml` | Monthly schedule; manual | Manage inactive issues and pull requests. |

## README status and release badges

README workflow badges are scoped to `main`; push-driven checks also select the
`push` event so feature-branch or pull-request runs cannot replace the reported
default-branch status. Build, CodeQL, lint, Conventional Commits, and scheduled
upstream compatibility each retain a distinct badge matching the responsibility
matrix above.

Release telemetry uses Shields.io's GitHub release-asset counters. The total
badge sums all assets across all published releases. The latest badge sums all
assets attached to GitHub's latest stable release, excluding pre-releases. Both
badges link directly to the matching GitHub Releases view.

## Build cache policy

The normal build, upstream compatibility, and release workflows share the local
`setup-build` action. Its cache uses an exact key composed from the runner OS and
architecture, compiler versions, immutable upstream SHA, overlay, both patch
layers, and build/verification scripts. It has no prefix fallback, so an object
built by another compiler or for another source revision is never reused.
Final ROM, ELF, map, and save outputs are excluded from cache storage; a cache
hit still performs the final link and ROM validation.

CodeQL intentionally bypasses this cache. Its compiler tracing must observe a
clean instrumented build; restoring object files could reduce analysis coverage.

## CodeQL issue synchronization

`codeql-issue.yml` runs only after both the Actions and C/C++ jobs complete
successfully on `main`, so a failed or cancelled scan can never produce a false
clean result. The reporting
script discovers the repository's default branch through GitHub's API, paginates
all open CodeQL alerts, and owns one issue identified by a private Markdown
marker. The issue is reopened and refreshed when findings exist, and it is
closed with the `completed` reason only after a successful analysis returns no
open findings. The workflow checks out only trusted default-branch code and has
the minimum read/write permissions required for alerts and issues.

## Release policy

Only `release.yml` can publish a `.gba`, and only when manually dispatched from
`main`. Stable tags must match `v[0-9].[0-9].[0-9]`. Pre-release tags must match
`v[0-9].[0-9].[0-9]-(alpha|beta|pre).[0-9]`. The workflow requires an explicit
rights attestation, requires the tag's base version to match `VERSION`, validates
the ROM, creates an annotated tag, and attaches the ROM, SHA-256 manifest, and
exact upstream revision to the GitHub release.
