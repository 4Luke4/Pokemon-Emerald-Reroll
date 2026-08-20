# Upstream hardening patches

This directory contains narrowly scoped correctness and security hardening for
the generated `pret/pokeemerald` worktree. These changes are deliberately kept
separate from gameplay hooks under `patches/integration/` so ownership and
upstream provenance remain explicit.

| Patch | Scope |
| --- | --- |
| `codeql-host-tools.patch` | Host-tool allocation arithmetic and variadic format safety reported by CodeQL in issue #8 |

Every patch must apply cleanly to the resolved upstream revision before any
Reroll integration patch. Do not add gameplay behavior or standalone Reroll
source here. When upstream incorporates an equivalent correction, remove the
corresponding hunk instead of maintaining a redundant fork.
