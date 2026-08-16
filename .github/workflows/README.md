# Workflow responsibilities

Each workflow owns one responsibility. Multiple checks may run for the same pull
request, but no two workflows produce the same assurance or publish the same
artifact.

| Workflow | Trigger | Exclusive responsibility |
| --- | --- | --- |
| `build.yml` | Push or pull request to `main` | Compile and validate the ROM; retain only its checksum and upstream SHA. |
| `codeql.yml` | Push or pull request to `main`; monthly schedule | Run an uncached, instrumented C/C++ security analysis. |
| `super-linter.yml` | Push or pull request to `main`; manual | Lint changed source, scripts, documentation, and workflow definitions. |
| `conventional-commits.yml` | Push to `main`; pull-request lifecycle | Validate Conventional Commit subjects. |
| `upstream-compatibility.yml` | Weekly schedule; manual | Test the latest protected upstream revision independently of repository changes. |
| `release.yml` | Manual only | Validate a version, create an annotated tag, and publish the verified ROM release. |
| `dependabot-automerge.yml` | Dependabot pull-request lifecycle | Enable auto-merge only for eligible dependency updates. |
| `label.yml` | Pull-request lifecycle | Apply path-based labels without executing pull-request code. |
| `stale.yml` | Monthly schedule; manual | Manage inactive issues and pull requests. |

## Build cache policy

The normal build, upstream compatibility, and release workflows share the local
`setup-build` action. Its cache uses an exact key composed from the runner OS and
architecture, compiler versions, immutable upstream SHA, overlay and integration
source, and build/verification scripts. It has no prefix fallback, so an object
built by another compiler or for another source revision is never reused.
Final ROM, ELF, map, and save outputs are excluded from cache storage; a cache
hit still performs the final link and ROM validation.

CodeQL intentionally bypasses this cache. Its compiler tracing must observe a
clean instrumented build; restoring object files could reduce analysis coverage.

## Release policy

Only `release.yml` can publish a `.gba`, and only when manually dispatched from
`main`. Stable tags must match `v[0-9].[0-9].[0-9]`. Pre-release tags must match
`v[0-9].[0-9].[0-9]-(alpha|beta|pre).[0-9]`. The workflow requires an explicit
rights attestation, validates the ROM, creates an annotated tag, and attaches the
ROM, SHA-256 manifest, and exact upstream revision to the GitHub release.
