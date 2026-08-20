#include "global.h"
#include "battle.h"
#include "data.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "reroll/internal.h"
#include "constants/abilities.h"
#include "constants/battle_move_effects.h"
#include "constants/daycare.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

#define REROLL_MOVE_POOL_CAPACITY 96
#define REROLL_EGG_MOVE_SPECIES_OFFSET 20000
#define REROLL_EGG_MOVE_TERMINATOR 0xFFFF
#define REROLL_MIN_SUPPORT_SCORE 60

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

bool8 RerollPokemon_IsEligibleSpecies(u16 species, u8 level)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    // Emerald keeps obsolete Unown-form slots in the species table with
    // deliberately extreme stats and question-mark graphics. They are data
    // placeholders, not obtainable Pokémon, so a nonzero base HP is not enough
    // to establish that a species is valid for a challenge roster.
    if (species >= SPECIES_OLD_UNOWN_B && species <= SPECIES_OLD_UNOWN_Z)
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
        if (!RerollPokemon_IsEligibleSpecies(species, level))
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
        if (RerollPokemon_IsEligibleSpecies(species, level))
        {
            bool8 duplicate = FALSE;

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

static bool8 IsStabMove(u16 species, u16 move)
{
    return gBattleMoves[move].type == gSpeciesInfo[species].types[0]
        || gBattleMoves[move].type == gSpeciesInfo[species].types[1];
}

static bool8 IsDamagingMove(u16 move)
{
    return gBattleMoves[move].power != 0;
}

static u16 GetMoveAccuracy(u16 move)
{
    // Zero is the data-table representation for moves that bypass accuracy.
    return gBattleMoves[move].accuracy == 0 ? 100 : gBattleMoves[move].accuracy;
}

static u16 GetRelevantOffense(u16 species, u16 move)
{
    if (IS_TYPE_PHYSICAL(gBattleMoves[move].type))
        return gSpeciesInfo[species].baseAttack;
    return gSpeciesInfo[species].baseSpAttack;
}

static u16 GetDamagingMoveScore(u16 species, u16 move)
{
    const struct BattleMove *battleMove = &gBattleMoves[move];
    u16 penalty = 0;
    u16 score;

    if (!IsDamagingMove(move))
        return 0;

    score = battleMove->power * 3;
    score += GetMoveAccuracy(move);
    score += GetRelevantOffense(species, move);
    if (IsStabMove(species, move))
        score += 100;

    // A high displayed power does not by itself make a dependable anchor.
    // Keep drawbacks meaningful while still allowing these moves when the
    // species has no safer legal alternative.
    switch (battleMove->effect)
    {
    case EFFECT_EXPLOSION:
        penalty = 240;
        break;
    case EFFECT_RECHARGE:
        penalty = 100;
        break;
    case EFFECT_RAZOR_WIND:
    case EFFECT_SKY_ATTACK:
    case EFFECT_SOLAR_BEAM:
    case EFFECT_SEMI_INVULNERABLE:
        penalty = 60;
        break;
    case EFFECT_RECOIL:
    case EFFECT_DOUBLE_EDGE:
    case EFFECT_RECOIL_IF_MISS:
    case EFFECT_FOCUS_PUNCH:
        penalty = 40;
        break;
    }

    score = score > penalty ? score - penalty : 1;
    // Similar legal attacks may trade places between runs; power, reliability,
    // STAB, and the species' stronger offensive stat remain dominant.
    return score + RerollRandom_Range(32);
}

static bool8 SpeciesHasType(u16 species, u8 type)
{
    return gSpeciesInfo[species].types[0] == type
        || gSpeciesInfo[species].types[1] == type;
}

static bool8 IsSetupEffect(u8 effect)
{
    switch (effect)
    {
    case EFFECT_ATTACK_UP:
    case EFFECT_DEFENSE_UP:
    case EFFECT_SPEED_UP:
    case EFFECT_SPECIAL_ATTACK_UP:
    case EFFECT_SPECIAL_DEFENSE_UP:
    case EFFECT_ACCURACY_UP:
    case EFFECT_EVASION_UP:
    case EFFECT_ATTACK_UP_2:
    case EFFECT_DEFENSE_UP_2:
    case EFFECT_SPEED_UP_2:
    case EFFECT_SPECIAL_ATTACK_UP_2:
    case EFFECT_SPECIAL_DEFENSE_UP_2:
    case EFFECT_ACCURACY_UP_2:
    case EFFECT_EVASION_UP_2:
    case EFFECT_FOCUS_ENERGY:
    case EFFECT_DEFENSE_CURL:
    case EFFECT_BELLY_DRUM:
    case EFFECT_COSMIC_POWER:
    case EFFECT_BULK_UP:
    case EFFECT_CALM_MIND:
    case EFFECT_DRAGON_DANCE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool8 IsResidualEffect(u8 effect)
{
    switch (effect)
    {
    case EFFECT_TOXIC:
    case EFFECT_LEECH_SEED:
    case EFFECT_WILL_O_WISP:
    case EFFECT_SANDSTORM:
    case EFFECT_HAIL:
        return TRUE;
    default:
        return FALSE;
    }
}

static u16 GetSupportMoveScore(u16 species, u16 move)
{
    const struct BattleMove *battleMove = &gBattleMoves[move];
    u16 score;

    if (IsDamagingMove(move))
        return 0;

    switch (battleMove->effect)
    {
    case EFFECT_ATTACK_UP:
    case EFFECT_ATTACK_UP_2:
    case EFFECT_BELLY_DRUM:
    case EFFECT_BULK_UP:
        score = 60 + gSpeciesInfo[species].baseAttack / 2;
        break;
    case EFFECT_SPECIAL_ATTACK_UP:
    case EFFECT_SPECIAL_ATTACK_UP_2:
    case EFFECT_CALM_MIND:
        score = 60 + gSpeciesInfo[species].baseSpAttack / 2;
        break;
    case EFFECT_DRAGON_DANCE:
        score = 70 + gSpeciesInfo[species].baseAttack / 2;
        break;
    case EFFECT_DEFENSE_UP:
    case EFFECT_SPECIAL_DEFENSE_UP:
    case EFFECT_DEFENSE_UP_2:
    case EFFECT_SPECIAL_DEFENSE_UP_2:
    case EFFECT_COSMIC_POWER:
        score = 80 + gSpeciesInfo[species].baseHP / 3;
        break;
    case EFFECT_RESTORE_HP:
    case EFFECT_MORNING_SUN:
    case EFFECT_SYNTHESIS:
    case EFFECT_MOONLIGHT:
    case EFFECT_SOFTBOILED:
    case EFFECT_WISH:
    case EFFECT_INGRAIN:
        score = 115;
        break;
    case EFFECT_SLEEP:
    case EFFECT_TOXIC:
    case EFFECT_PARALYZE:
    case EFFECT_LEECH_SEED:
    case EFFECT_WILL_O_WISP:
    case EFFECT_YAWN:
        score = 100;
        break;
    case EFFECT_PROTECT:
    case EFFECT_SUBSTITUTE:
    case EFFECT_LIGHT_SCREEN:
    case EFFECT_REFLECT:
    case EFFECT_SPIKES:
    case EFFECT_SAFEGUARD:
        score = 90;
        break;
    case EFFECT_REST:
        score = 85;
        break;
    case EFFECT_HEAL_BELL:
    case EFFECT_REFRESH:
    case EFFECT_HAZE:
    case EFFECT_MIST:
        score = 80;
        break;
    case EFFECT_ATTACK_DOWN:
    case EFFECT_DEFENSE_DOWN:
    case EFFECT_SPEED_DOWN:
    case EFFECT_SPECIAL_ATTACK_DOWN:
    case EFFECT_SPECIAL_DEFENSE_DOWN:
    case EFFECT_ACCURACY_DOWN:
    case EFFECT_EVASION_DOWN:
    case EFFECT_ATTACK_DOWN_2:
    case EFFECT_DEFENSE_DOWN_2:
    case EFFECT_SPEED_DOWN_2:
    case EFFECT_SPECIAL_ATTACK_DOWN_2:
    case EFFECT_SPECIAL_DEFENSE_DOWN_2:
    case EFFECT_ACCURACY_DOWN_2:
    case EFFECT_EVASION_DOWN_2:
    case EFFECT_CONFUSE:
    case EFFECT_DISABLE:
    case EFFECT_ENCORE:
    case EFFECT_MEAN_LOOK:
    case EFFECT_TAUNT:
    case EFFECT_TORMENT:
        score = 70;
        break;
    case EFFECT_RAIN_DANCE:
        score = SpeciesHasType(species, TYPE_WATER)
             || SpeciesHasType(species, TYPE_ELECTRIC) ? 95 : 55;
        break;
    case EFFECT_SUNNY_DAY:
        score = SpeciesHasType(species, TYPE_FIRE)
             || SpeciesHasType(species, TYPE_GRASS) ? 95 : 55;
        break;
    case EFFECT_SANDSTORM:
        score = SpeciesHasType(species, TYPE_ROCK)
             || SpeciesHasType(species, TYPE_GROUND)
             || SpeciesHasType(species, TYPE_STEEL) ? 90 : 50;
        break;
    case EFFECT_HAIL:
        score = SpeciesHasType(species, TYPE_ICE) ? 90 : 50;
        break;
    case EFFECT_SPEED_UP:
    case EFFECT_SPEED_UP_2:
    case EFFECT_ACCURACY_UP:
    case EFFECT_EVASION_UP:
    case EFFECT_ACCURACY_UP_2:
    case EFFECT_EVASION_UP_2:
    case EFFECT_FOCUS_ENERGY:
    case EFFECT_DEFENSE_CURL:
    case EFFECT_DESTINY_BOND:
    case EFFECT_PERISH_SONG:
        score = 75;
        break;
    case EFFECT_BATON_PASS:
    case EFFECT_SLEEP_TALK:
        score = 40;
        break;
    case EFFECT_SPLASH:
    case EFFECT_TELEPORT:
        return 0;
    default:
        score = 35;
        break;
    }

    if (battleMove->accuracy != 0)
        score += battleMove->accuracy / 5;
    return score;
}

static u16 GetPairSynergy(u16 firstMove, u16 secondMove)
{
    const struct BattleMove *first = &gBattleMoves[firstMove];
    const struct BattleMove *second = &gBattleMoves[secondMove];

    if ((firstMove == MOVE_REST && secondMove == MOVE_SLEEP_TALK)
     || (firstMove == MOVE_SLEEP_TALK && secondMove == MOVE_REST))
        return 120;
    if ((first->effect == EFFECT_BATON_PASS && IsSetupEffect(second->effect))
     || (second->effect == EFFECT_BATON_PASS && IsSetupEffect(first->effect)))
        return 80;
    if ((first->effect == EFFECT_PROTECT && IsResidualEffect(second->effect))
     || (second->effect == EFFECT_PROTECT && IsResidualEffect(first->effect)))
        return 70;
    if ((firstMove == MOVE_STOCKPILE
      && (secondMove == MOVE_SPIT_UP || secondMove == MOVE_SWALLOW))
     || (secondMove == MOVE_STOCKPILE
      && (firstMove == MOVE_SPIT_UP || firstMove == MOVE_SWALLOW)))
        return 100;
    if ((firstMove == MOVE_ENDURE
      && (secondMove == MOVE_FLAIL || secondMove == MOVE_REVERSAL))
     || (secondMove == MOVE_ENDURE
      && (firstMove == MOVE_FLAIL || firstMove == MOVE_REVERSAL)))
        return 90;
    if ((firstMove == MOVE_DEFENSE_CURL && secondMove == MOVE_ROLLOUT)
     || (secondMove == MOVE_DEFENSE_CURL && firstMove == MOVE_ROLLOUT))
        return 80;
    if ((firstMove == MOVE_SUNNY_DAY
      && (second->type == TYPE_FIRE || secondMove == MOVE_SOLAR_BEAM))
     || (secondMove == MOVE_SUNNY_DAY
      && (first->type == TYPE_FIRE || firstMove == MOVE_SOLAR_BEAM)))
        return 60;
    if ((firstMove == MOVE_RAIN_DANCE
      && (second->type == TYPE_WATER || secondMove == MOVE_THUNDER))
     || (secondMove == MOVE_RAIN_DANCE
      && (first->type == TYPE_WATER || firstMove == MOVE_THUNDER)))
        return 60;
    if ((firstMove == MOVE_CHARGE && second->type == TYPE_ELECTRIC)
     || (secondMove == MOVE_CHARGE && first->type == TYPE_ELECTRIC))
        return 50;
    return 0;
}

static u16 ScoreCandidate(u16 species, u16 move, const u16 *chosenMoves, u8 chosenCount)
{
    const struct BattleMove *battleMove = &gBattleMoves[move];
    u16 score;
    u8 supportCount = 0;
    u8 i;

    if (IsDamagingMove(move))
    {
        score = 25 + battleMove->power + GetMoveAccuracy(move) / 2;
        score += GetRelevantOffense(species, move) / 3;
        if (IsStabMove(species, move))
            score += 30;
    }
    else
    {
        score = GetSupportMoveScore(species, move);
    }

    for (i = 0; i < chosenCount; i++)
    {
        const struct BattleMove *chosen = &gBattleMoves[chosenMoves[i]];

        score += GetPairSynergy(move, chosenMoves[i]);
        if (!IsDamagingMove(chosenMoves[i]))
            supportCount++;
        else if (IsDamagingMove(move) && chosen->type == battleMove->type)
            score = score * 2 / 3;
    }

    // One support move is deliberately encouraged. Further support remains
    // possible, but the weighting keeps a legal way to deal damage prominent.
    if (!IsDamagingMove(move) && supportCount >= 2)
        score = score * 2 / 3;
    return score;
}

static s16 ChooseBestDamagingMove(u16 species, const u16 *candidates,
                                  u8 candidateCount, bool8 requireStab)
{
    u16 bestScore = 0;
    s16 best = -1;
    u8 i;

    for (i = 0; i < candidateCount; i++)
    {
        u16 score;

        if (!IsDamagingMove(candidates[i]))
            continue;
        if (requireStab && !IsStabMove(species, candidates[i]))
            continue;
        score = GetDamagingMoveScore(species, candidates[i]);
        if (score > bestScore)
        {
            bestScore = score;
            best = i;
        }
    }
    return best;
}

static s16 ChooseWeightedMove(u16 species, const u16 *candidates, u8 candidateCount,
                              const bool8 *selected, const u16 *chosenMoves,
                              u8 chosenCount, bool8 supportOnly)
{
    u32 totalWeight = 0;
    u32 choice;
    u8 i;

    for (i = 0; i < candidateCount; i++)
    {
        u16 score;

        if (selected[i])
            continue;
        score = ScoreCandidate(species, candidates[i], chosenMoves, chosenCount);
        if (supportOnly && (IsDamagingMove(candidates[i]) || score < REROLL_MIN_SUPPORT_SCORE))
            continue;
        totalWeight += score;
    }
    if (totalWeight == 0)
        return -1;

    choice = RerollRandom_Range(totalWeight);
    for (i = 0; i < candidateCount; i++)
    {
        u16 score;

        if (selected[i])
            continue;
        score = ScoreCandidate(species, candidates[i], chosenMoves, chosenCount);
        if (supportOnly && (IsDamagingMove(candidates[i]) || score < REROLL_MIN_SUPPORT_SCORE))
            continue;
        if (choice < score)
            return i;
        choice -= score;
    }
    return -1;
}

static void AddSelectedMove(const u16 *candidates, bool8 *selected, s16 selectedIndex,
                            u16 *chosenMoves, u8 *chosenCount)
{
    if (selectedIndex < 0)
        return;
    selected[selectedIndex] = TRUE;
    chosenMoves[(*chosenCount)++] = candidates[selectedIndex];
}

static void WriteMoveset(struct Pokemon *mon, const u16 *moves, u8 moveCount)
{
    u8 slot;

    for (slot = 0; slot < MAX_MON_MOVES; slot++)
    {
        u16 move = slot < moveCount ? moves[slot] : MOVE_NONE;

        SetMonData(mon, MON_DATA_MOVE1 + slot, &move);
        if (move != MOVE_NONE)
        {
            u8 pp = gBattleMoves[move].pp;
            SetMonData(mon, MON_DATA_PP1 + slot, &pp);
        }
    }
}

void RerollPokemon_BuildMoveset(struct Pokemon *mon, u16 species, u8 level)
{
    u16 candidates[REROLL_MOVE_POOL_CAPACITY];
    u16 chosenMoves[MAX_MON_MOVES];
    bool8 selected[REROLL_MOVE_POOL_CAPACITY] = {FALSE};
    u8 candidateCount = 0;
    u8 chosenCount = 0;
    u8 i;
    s16 selectedIndex;

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

    // Slot one is a dependable STAB attack whenever the legal pool provides
    // one. A fallback damaging move prevents unusual species from being left
    // helpless when no damaging move matches either of their types.
    selectedIndex = ChooseBestDamagingMove(species, candidates, candidateCount, TRUE);
    if (selectedIndex < 0)
        selectedIndex = ChooseBestDamagingMove(species, candidates, candidateCount, FALSE);
    AddSelectedMove(candidates, selected, selectedIndex, chosenMoves, &chosenCount);

    // Prefer one genuinely useful support option. Its weight is derived from
    // the species' actual stats and type, then pairwise bonuses reward coherent
    // combinations such as Rest/Sleep Talk and weather-compatible attacks.
    if (chosenCount < MAX_MON_MOVES)
    {
        selectedIndex = ChooseWeightedMove(species, candidates, candidateCount,
                                           selected, chosenMoves, chosenCount, TRUE);
        AddSelectedMove(candidates, selected, selectedIndex, chosenMoves, &chosenCount);
    }

    // Remaining slots use weighted random selection instead of repeated maxima.
    // Coverage, reliability, species strategy, and already-selected moves shape
    // the weights without forcing every roster into four damaging attacks.
    while (chosenCount < MAX_MON_MOVES)
    {
        selectedIndex = ChooseWeightedMove(species, candidates, candidateCount,
                                           selected, chosenMoves, chosenCount, FALSE);
        if (selectedIndex < 0)
            break;
        AddSelectedMove(candidates, selected, selectedIndex, chosenMoves, &chosenCount);
    }
    WriteMoveset(mon, chosenMoves, chosenCount);
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
