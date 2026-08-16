# Upstream Resolution Policy

| Field | Value |
| --- | --- |
| Repository | `https://github.com/pret/pokeemerald.git` |
| Stable channel | Protected default branch |
| Resolver | `scripts/resolve-upstream.sh` |
| Build record | `dist/upstream.sha` |
| Game | English Pokémon Emerald, revision 0 |
| Compiler mode | Modern (`make modern`) |

`pret/pokeemerald` does not publish GitHub releases or tags. Reroll therefore
defines its latest stable source as the commit advertised by upstream's protected
default branch. That branch requires the upstream build status before changes can
merge.

Each build resolves the moving branch once, validates a complete 40-character
SHA, checks out that immutable commit, and records it in `dist/upstream.sha`.
Subsequent build steps never consume the moving branch name. Supplying
`UPSTREAM_REF=<recorded-sha>` reproduces the source selection for an earlier job.

The weekly upstream-compatibility workflow resolves the current stable commit,
applies every integration patch, overlays the standalone feature modules, builds
the modern ROM, and retains only the upstream SHA and ROM digest. A compatibility
failure must:

1. fail closed without uploading a ROM;
2. identify the resolved upstream SHA in the job log;
3. update only the affected standalone module or topic-specific hook;
4. pass source verification, CodeQL, and a clean ROM build;
5. include an mGBA smoke test when behavior, rather than context, changed; and
6. use a `build(upstream): ...` Conventional Commit when remediation is needed.
