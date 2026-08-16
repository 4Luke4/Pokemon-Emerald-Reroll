#include "global.h"
#include "main.h"
#include "reroll/reroll.h"
#include "save.h"

void Reroll_GameOver(void)
{
    // Permadeath is intentionally irreversible: both save slots are erased before
    // the boot sequence restarts, so a failed run cannot be resumed.
    ClearSaveData();
    DoSoftReset();
}
