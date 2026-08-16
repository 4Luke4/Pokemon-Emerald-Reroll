#include "global.h"
#include "pokemon.h"
#include "reroll/reroll.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

static bool8 IsFieldHm(u16 move)
{
    return move == MOVE_CUT
        || move == MOVE_FLY
        || move == MOVE_SURF
        || move == MOVE_STRENGTH
        || move == MOVE_FLASH
        || move == MOVE_ROCK_SMASH
        || move == MOVE_WATERFALL
        || move == MOVE_DIVE;
}

u8 Reroll_FindVirtualHmUser(u16 move)
{
    u8 i;

    if (!IsFieldHm(move))
        return PARTY_SIZE;

    // Field scripts still receive a real, non-Egg party member for nicknames and
    // field effects, but the HM no longer consumes one of that Pokémon's moves.
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            return i;
    }
    return PARTY_SIZE;
}

bool8 Reroll_HasVirtualSurfUser(void)
{
    return Reroll_FindVirtualHmUser(MOVE_SURF) != PARTY_SIZE;
}
