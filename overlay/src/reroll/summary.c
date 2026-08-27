#include "global.h"
#include "pokemon.h"
#include "reroll/reroll.h"
#include "constants/pokemon.h"

static const u8 sNatureStatColorNeutral[] = _("{COLOR WHITE}");
static const u8 sNatureStatColorIncreased[] = _("{COLOR LIGHT_RED}");
static const u8 sNatureStatColorDecreased[] = _("{COLOR BLUE}");

const u8 *Reroll_GetNatureStatColor(u8 nature, u8 stat)
{
    s8 modifier;

    // HP is never nature-affected. Invalid identifiers are rendered neutrally
    // instead of indexing beyond Emerald's canonical modifier table.
    if (nature >= NUM_NATURES || stat == STAT_HP || stat >= NUM_STATS)
        return sNatureStatColorNeutral;

    modifier = gNatureStatTable[nature][stat - 1];
    if (modifier > 0)
        return sNatureStatColorIncreased;
    if (modifier < 0)
        return sNatureStatColorDecreased;
    return sNatureStatColorNeutral;
}
