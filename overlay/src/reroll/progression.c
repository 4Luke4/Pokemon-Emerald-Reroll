#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "reroll/internal.h"
#include "reroll/reroll.h"
#include "save.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/pokemon.h"
#include "constants/trainers.h"

// Ordered vanilla story opponents define both reroll triggers and the level of
// the next generated player party.
static const u16 sProgressionBosses[] =
{
    TRAINER_ROXANNE_1,
    TRAINER_BRAWLY_1,
    TRAINER_WATTSON_1,
    TRAINER_FLANNERY_1,
    TRAINER_NORMAN_1,
    TRAINER_WINONA_1,
    TRAINER_TATE_AND_LIZA_1,
    TRAINER_JUAN_1,
    TRAINER_SIDNEY,
    TRAINER_PHOEBE,
    TRAINER_GLACIA,
    TRAINER_DRAKE,
    TRAINER_WALLACE,
};

static u16 GetNextProgressionBoss(void)
{
    static const u16 badgeFlags[] =
    {
        FLAG_BADGE01_GET,
        FLAG_BADGE02_GET,
        FLAG_BADGE03_GET,
        FLAG_BADGE04_GET,
        FLAG_BADGE05_GET,
        FLAG_BADGE06_GET,
        FLAG_BADGE07_GET,
        FLAG_BADGE08_GET,
    };
    static const u16 eliteFlags[] =
    {
        FLAG_DEFEATED_ELITE_4_SIDNEY,
        FLAG_DEFEATED_ELITE_4_PHOEBE,
        FLAG_DEFEATED_ELITE_4_GLACIA,
        FLAG_DEFEATED_ELITE_4_DRAKE,
    };
    u8 i;

    for (i = 0; i < ARRAY_COUNT(badgeFlags); i++)
    {
        if (!FlagGet(badgeFlags[i]))
            return sProgressionBosses[i];
    }
    for (i = 0; i < ARRAY_COUNT(eliteFlags); i++)
    {
        if (!FlagGet(eliteFlags[i]))
            return sProgressionBosses[ARRAY_COUNT(badgeFlags) + i];
    }
    return TRAINER_WALLACE;
}

static void CreatePlayerParty(u8 level)
{
    u16 species[PARTY_SIZE];
    u32 otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    u8 i;

    ZeroPlayerPartyMons();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 personality;
        u32 experience;

        species[i] = RerollPokemon_ChooseSpecies(level, species, i);
        personality = RerollPokemon_CreateShinyPersonality(otId);
        CreateMon(&gPlayerParty[i], species[i], level, MAX_PER_STAT_IVS, TRUE,
                  personality, OT_ID_PRESET, otId);
        RerollPokemon_ChooseAbility(&gPlayerParty[i], species[i]);
        RerollPokemon_BuildMoveset(&gPlayerParty[i], species[i], level);

        // Pin experience to the exact floor for the assigned level. Battle EXP,
        // EVs, and Rare Candy are blocked by small integration hooks.
        experience = gExperienceTables[gSpeciesInfo[species[i]].growthRate][level];
        SetMonData(&gPlayerParty[i], MON_DATA_EXP, &experience);
        CalculateMonStats(&gPlayerParty[i]);
    }
    gPlayerPartyCount = PARTY_SIZE;
}

static bool8 PlayerPartyMatchesLevel(u8 level)
{
    u32 otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        u32 personality = GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY);

        if (species == SPECIES_NONE)
            return FALSE;
        if (GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) != level)
            return FALSE;
        if (GET_SHINY_VALUE(otId, personality) >= SHINY_ODDS)
            return FALSE;
    }
    return TRUE;
}

void Reroll_EnsurePlayerParty(void)
{
    u8 targetLevel;

    RerollRandom_Init();
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
    ClearItemSlots(gSaveBlock1Ptr->bagPocket_PokeBalls, BAG_POKEBALLS_COUNT);

    targetLevel = RerollTrainer_GetAverageLevel(GetNextProgressionBoss());
    if (!PlayerPartyMatchesLevel(targetLevel))
        CreatePlayerParty(targetLevel);
}

void Reroll_OnBattleEnd(void)
{
    u8 i;

    if (gBattleOutcome == B_OUTCOME_LOST
     && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_WALLY_TUTORIAL)))
    {
        Reroll_GameOver();
        return;
    }

    if (gBattleOutcome != B_OUTCOME_WON || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return;

    for (i = 0; i < ARRAY_COUNT(sProgressionBosses); i++)
    {
        if (gTrainerBattleOpponent_A == sProgressionBosses[i])
        {
            u8 next = i + 1;

            if (next >= ARRAY_COUNT(sProgressionBosses))
                next = ARRAY_COUNT(sProgressionBosses) - 1;
            CreatePlayerParty(RerollTrainer_GetAverageLevel(sProgressionBosses[next]));
            return;
        }
    }

    // Leader rematches also reroll. Progression flags select the next undefeated
    // League opponent for the party's floored average level.
    if (RerollTrainer_IsImportant(gTrainerBattleOpponent_A)
     && (gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_LEADER
      || gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_ELITE_FOUR
      || gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_CHAMPION))
        CreatePlayerParty(RerollTrainer_GetAverageLevel(GetNextProgressionBoss()));
}
