# Upstream Pin

| Field | Value |
| --- | --- |
| Repository | `https://github.com/pret/pokeemerald.git` |
| Commit | `9a83a2bbe8e097e62c00f1dbd56849766775d7b6` |
| Tree | `426e47d31e5f02d966772e38265b46cb5f65e81f` |
| Game | English Pokémon Emerald, revision 0 |
| Compiler mode | Modern (`make modern`) |

The build scripts always fetch the exact commit above and apply `patches/reroll.patch`. Updating the pin is a deliberate maintenance operation, not an automatic dependency update. A pin update must:

1. regenerate and review the complete patch;
2. pass source verification, CodeQL, and a clean ROM build;
3. include an mGBA story smoke test through at least the first rival and Gym battles;
4. document conflicts and behavior changes in `CHANGELOG.md`; and
5. use a `build(upstream): ...` Conventional Commit.
