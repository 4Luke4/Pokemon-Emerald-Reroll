#include "global.h"
#include "battle.h"
#include "data.h"
#include "pokemon.h"
#include "reroll/internal.h"
#include "reroll/reroll.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/trainers.h"

static const u16 sUsefulHeldItems[] =
{
    ITEM_LEFTOVERS,
    ITEM_LUM_BERRY,
    ITEM_SITRUS_BERRY,
    ITEM_SHELL_BELL,
    ITEM_BRIGHT_POWDER,
    ITEM_QUICK_CLAW,
    ITEM_FOCUS_BAND,
};

static const u16 sImportantTrainerItems[MAX_TRAINER_ITEMS] =
{
    ITEM_FULL_RESTORE,
    ITEM_MAX_POTION,
    ITEM_FULL_HEAL,
    ITEM_GUARD_SPEC,
};

static u8 GetTrainerMonLevel(u16 trainerId, u8 slot)
{
    const struct Trainer *trainer = &gTrainers[trainerId];
    u16 total;
    u8 i;

    if (trainer->partySize == 0)
        return 1;

    // Added slots on important six-member teams inherit the vanilla party's
    // floored average. Existing slots retain their exact vanilla level.
    if (slot >= trainer->partySize)
    {
        total = 0;
        for (i = 0; i < trainer->partySize; i++)
            total += GetTrainerMonLevel(trainerId, i);
        return total / trainer->partySize;
    }

    switch (trainer->partyFlags)
    {
    case 0:
        return trainer->party.NoItemDefaultMoves[slot].lvl;
    case F_TRAINER_PARTY_CUSTOM_MOVESET:
        return trainer->party.NoItemCustomMoves[slot].lvl;
    case F_TRAINER_PARTY_HELD_ITEM:
        return trainer->party.ItemDefaultMoves[slot].lvl;
    default:
        return trainer->party.ItemCustomMoves[slot].lvl;
    }
}

bool8 RerollTrainer_IsImportant(u16 trainerId)
{
    u8 trainerClass = gTrainers[trainerId].trainerClass;

    switch (trainerClass)
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_SALON_MAIDEN:
    case TRAINER_CLASS_DOME_ACE:
    case TRAINER_CLASS_PALACE_MAVEN:
    case TRAINER_CLASS_ARENA_TYCOON:
    case TRAINER_CLASS_FACTORY_HEAD:
    case TRAINER_CLASS_PIKE_QUEEN:
    case TRAINER_CLASS_PYRAMID_KING:
        return TRUE;
    default:
        return FALSE;
    }
}

u8 RerollTrainer_GetAverageLevel(u16 trainerId)
{
    u16 total = 0;
    u8 i;

    if (gTrainers[trainerId].partySize == 0)
        return 1;
    for (i = 0; i < gTrainers[trainerId].partySize; i++)
        total += GetTrainerMonLevel(trainerId, i);
    return total / gTrainers[trainerId].partySize;
}

u8 Reroll_CreateTrainerParty(struct Pokemon *party, u16 trainerId, bool8 firstTrainer)
{
    u16 selectedSpecies[PARTY_SIZE];
    u8 monCount;
    u8 i;

    if (trainerId == TRAINER_SECRET_BASE)
        return 0;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     || (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_TRAINER_HILL)))
        return gTrainers[trainerId].partySize;

    RerollRandom_Init();
    if (firstTrainer)
        ZeroEnemyPartyMons();

    monCount = RerollTrainer_IsImportant(trainerId) ? PARTY_SIZE : gTrainers[trainerId].partySize;
    if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
        monCount = min(monCount, PARTY_SIZE / 2);

    for (i = 0; i < monCount; i++)
    {
        u8 level = GetTrainerMonLevel(trainerId, i);
        u16 heldItem = ITEM_NONE;

        selectedSpecies[i] = RerollPokemon_ChooseSpecies(level, selectedSpecies, i);
        CreateMon(&party[i], selectedSpecies[i], level,
                  RerollTrainer_IsImportant(trainerId) ? MAX_PER_STAT_IVS : 18,
                  FALSE, 0, OT_ID_RANDOM_NO_SHINY, 0);
        RerollPokemon_ChooseAbility(&party[i], selectedSpecies[i]);
        RerollPokemon_BuildMoveset(&party[i], selectedSpecies[i], level);

        if (RerollTrainer_IsImportant(trainerId))
        {
            heldItem = sUsefulHeldItems[RerollRandom_Range(ARRAY_COUNT(sUsefulHeldItems))];
            SetMonData(&party[i], MON_DATA_HELD_ITEM, &heldItem);
        }
    }

    if (gTrainers[trainerId].doubleBattle)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
    return monCount;
}

u32 Reroll_GetTrainerAiFlags(u16 trainerId)
{
    u32 flags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_CHECK_VIABILITY;

    // Important trainers receive the full tactical policy; ordinary trainers
    // keep a smaller viability policy and their vanilla fainting intent.
    if (RerollTrainer_IsImportant(trainerId))
    {
        flags |= AI_SCRIPT_TRY_TO_FAINT;
        flags |= AI_SCRIPT_SETUP_FIRST_TURN;
        flags |= AI_SCRIPT_HP_AWARE;
    }
    else if (gTrainers[trainerId].aiFlags & AI_SCRIPT_TRY_TO_FAINT)
    {
        flags |= AI_SCRIPT_TRY_TO_FAINT;
    }
    return flags;
}

u16 Reroll_GetTrainerItem(u16 trainerId, u8 itemSlot)
{
    if (RerollTrainer_IsImportant(trainerId))
        return sImportantTrainerItems[itemSlot];
    return gTrainers[trainerId].items[itemSlot];
}
