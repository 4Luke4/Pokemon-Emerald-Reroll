#include "global.h"
#include "data.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "reroll/internal.h"
#include "constants/abilities.h"
#include "constants/daycare.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

#define REROLL_MOVE_POOL_CAPACITY 96
#define REROLL_EGG_MOVE_SPECIES_OFFSET 20000
#define REROLL_EGG_MOVE_TERMINATOR 0xFFFF

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const u16 gEggMoves[];

static bool8 IsFinalEvolution(u16 species)
{
    u8 i;

    for (i = 0; i < EVOS_PER_MON; i++)
    {
        if (gEvolutionTable[species][i].method != 0)
            return FALSE;
    }
    return TRUE;
}

static bool8 IsEligibleSpecies(u16 species, u8 level)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;
    if (gSpeciesInfo[species].baseHP == 0)
        return FALSE;

    // Low-level runs may contain any stage or legendary. Once the target level
    // exceeds 50, only species with no further evolution remain eligible.
    if (level > 50 && !IsFinalEvolution(species))
        return FALSE;
    return TRUE;
}

u16 RerollPokemon_ChooseSpecies(u8 level, const u16 *excludedSpecies, u8 excludedCount)
{
    u16 species;
    u16 attempts;
    u8 i;

    for (attempts = 0; attempts < NUM_SPECIES * 2; attempts++)
    {
        bool8 duplicate = FALSE;

        species = RerollRandom_Range(NUM_SPECIES - 1) + 1;
        if (!IsEligibleSpecies(species, level))
            continue;

        for (i = 0; i < excludedCount; i++)
        {
            if (excludedSpecies[i] == species)
            {
                duplicate = TRUE;
                break;
            }
        }
        if (!duplicate)
            return species;
    }

    // A deterministic scan guarantees termination if future data changes make
    // the random candidate pool unexpectedly sparse.
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (IsEligibleSpecies(species, level))
            return species;
    }
    return SPECIES_BULBASAUR;
}

static bool8 AddMoveCandidate(u16 *moves, u8 *moveCount, u16 move)
{
    u8 i;

    if (move == MOVE_NONE || move >= MOVES_COUNT)
        return FALSE;
    for (i = 0; i < *moveCount; i++)
    {
        if (moves[i] == move)
            return FALSE;
    }
    if (*moveCount >= REROLL_MOVE_POOL_CAPACITY)
        return FALSE;
    moves[(*moveCount)++] = move;
    return TRUE;
}

static void AddEggMoves(u16 species, u16 *moves, u8 *moveCount)
{
    u16 i;

    for (i = 0; gEggMoves[i] != REROLL_EGG_MOVE_TERMINATOR; i++)
    {
        if (gEggMoves[i] == species + REROLL_EGG_MOVE_SPECIES_OFFSET)
        {
            for (i++; gEggMoves[i] < REROLL_EGG_MOVE_SPECIES_OFFSET; i++)
                AddMoveCandidate(moves, moveCount, gEggMoves[i]);
            return;
        }
    }
}

static u16 ScoreMove(u16 species, u16 move)
{
    const struct BattleMove *battleMove = &gBattleMoves[move];
    u16 score;

    // Status moves get a useful baseline; damaging moves scale with power.
    score = battleMove->power == 0 ? 55 : 80 + battleMove->power * 2;
    if (battleMove->type == gSpeciesInfo[species].types[0]
     || battleMove->type == gSpeciesInfo[species].types[1])
        score += 60;
    if (battleMove->accuracy != 0)
        score += battleMove->accuracy / 4;

    // Small stream-derived variation avoids identical legal sets while power,
    // accuracy, and same-type utility remain the dominant factors.
    score += RerollRandom_Range(24);
    return score;
}

void RerollPokemon_BuildMoveset(struct Pokemon *mon, u16 species, u8 level)
{
    u16 candidates[REROLL_MOVE_POOL_CAPACITY];
    u16 scores[REROLL_MOVE_POOL_CAPACITY];
    u8 candidateCount = 0;
    u8 i;
    u8 slot;

    // Level-up moves are legal only when learned at or below the assigned level.
    for (i = 0; i < MAX_LEVEL_UP_MOVES; i++)
    {
        u16 packedMove = gLevelUpLearnsets[species][i];

        if (packedMove == LEVEL_UP_END)
            break;
        if ((packedMove & LEVEL_UP_MOVE_LV) <= (level << 9))
            AddMoveCandidate(candidates, &candidateCount, packedMove & LEVEL_UP_MOVE_ID);
    }

    // Compatible TMs, HMs, and egg moves are all valid challenge moves.
    for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
    {
        if (CanSpeciesLearnTMHM(species, i))
            AddMoveCandidate(candidates, &candidateCount, ItemIdToBattleMoveId(ITEM_TM01 + i));
    }
    AddEggMoves(species, candidates, &candidateCount);

    for (i = 0; i < candidateCount; i++)
        scores[i] = ScoreMove(species, candidates[i]);

    // Repeated maximum selection yields four distinct moves whenever the legal
    // pool contains at least four candidates.
    for (slot = 0; slot < MAX_MON_MOVES; slot++)
    {
        u16 selectedMove = MOVE_NONE;
        u16 bestScore = 0;
        u8 selected = 0;

        for (i = 0; i < candidateCount; i++)
        {
            if (scores[i] > bestScore)
            {
                bestScore = scores[i];
                selectedMove = candidates[i];
                selected = i;
            }
        }

        SetMonData(mon, MON_DATA_MOVE1 + slot, &selectedMove);
        if (selectedMove != MOVE_NONE)
        {
            u8 pp = gBattleMoves[selectedMove].pp;
            SetMonData(mon, MON_DATA_PP1 + slot, &pp);
            scores[selected] = 0;
        }
    }
}

void RerollPokemon_ChooseAbility(struct Pokemon *mon, u16 species)
{
    u8 abilityNum = 0;

    // Only legal ability slots from the species table can be selected.
    if (gSpeciesInfo[species].abilities[1] != ABILITY_NONE)
        abilityNum = RerollRandom_Range(2);
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
}

u32 RerollPokemon_CreateShinyPersonality(u32 otId)
{
    u16 upper = RerollRandom_Range(1 << 16);
    u16 lower = upper ^ HIHALF(otId) ^ LOHALF(otId);

    // The constructed shiny value is zero, safely below Gen III's threshold.
    return ((u32)upper << 16) | lower;
}
