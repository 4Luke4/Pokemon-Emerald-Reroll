# Third-Party Notices

## `pret/pokeemerald`

Reroll targets the Pokémon Emerald decompilation maintained at <https://github.com/pret/pokeemerald>. The latest commit on its protected default branch is resolved and frozen at build time; the exact revision is recorded in `dist/upstream.sha`. That project retains its own copyright and MIT license terms.

Reroll's proprietary license applies only to original Reroll material. It does not replace or narrow the upstream MIT license for upstream-authored code.

## `pret/pokeheartgold`

The follower's party selection, withdrawal rules, and map-transition behavior
were checked against the Pokémon HeartGold/SoulSilver decompilation at
<https://github.com/pret/pokeheartgold>, revision
`0985e8718df4f25e64d6507d89c0c97c0d288981`. The reference implementation is
partially decompiled: follower creation and species/form selection are in C,
while portions of the movement controller remain in assembly. No Nintendo DS
ROM archive or extracted `pokeheartgold` asset is included by this project.

## `PokemonSanFran/merrp`

The 32×32 six-frame follower sheets under
`overlay/graphics/reroll/followers/` originate from the public
`followers-expanded-id` branch of <https://github.com/PokemonSanFran/merrp>.
Exact bootstrap or stable-release provenance, the resolved immutable commit,
and the complete asset-set digest are recorded in
`dependencies/merrp-followers.json`. The importer reads that manifest or an
explicit release ref; executable code does not embed a permanent merrp commit.

Reroll uses those sheets with an original, isolated sprite lifecycle,
route-trail implementation, and state-based dialogue. Its interaction bubbles
reuse Pokémon Emerald's existing heart, question, and exclamation graphics;
merrp's dialogue, emote, field-move, and map-script subsystems are not imported.

The upstream project's rights and all underlying Pokémon audiovisual rights
remain with their respective owners. Their inclusion does not place the sheets
under Reroll's proprietary license or grant redistribution rights beyond those
the user already possesses.

## Pokémon intellectual property

Pokémon, Pokémon character names, Pokémon Emerald, Nintendo, Game Boy Advance, Creatures, and GAME FREAK names and marks belong to their respective owners. No ownership of those names, marks, game data, audiovisual assets, or other proprietary material is claimed.

Source control does not contain a compiled commercial-game ROM. The manual
release workflow includes a rights attestation because repository automation
cannot determine whether publication is lawful in a maintainer's jurisdiction.
Nothing in this repository grants rights to third-party game content.

Contributors must not commit or upload ROM images, save files containing
personal data, leaked source code, or assets obtained from an unlawful copy.
Only an authorized maintainer may publish a validated build through the
protected manual release workflow.
