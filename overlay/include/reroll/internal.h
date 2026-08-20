#ifndef GUARD_REROLL_INTERNAL_H
#define GUARD_REROLL_INTERNAL_H

#include "global.h"

struct Pokemon;

// ChaCha20-backed random stream shared by every randomized subsystem.
void RerollRandom_Init(void);
u32 RerollRandom_Range(u32 upperBound);

// Legal Pokémon construction primitives shared by player and trainer parties.
bool8 RerollPokemon_IsEligibleSpecies(u16 species, u8 level);
u16 RerollPokemon_ChooseSpecies(u8 level, const u16 *excludedSpecies, u8 excludedCount);
void RerollPokemon_BuildMoveset(struct Pokemon *mon, u16 species, u8 level);
void RerollPokemon_ChooseAbility(struct Pokemon *mon, u16 species);
u32 RerollPokemon_CreateShinyPersonality(u32 otId);

// Trainer metadata used by both opponent generation and progression scaling.
bool8 RerollTrainer_IsImportant(u16 trainerId);
u8 RerollTrainer_GetAverageLevel(u16 trainerId);

#endif // GUARD_REROLL_INTERNAL_H
