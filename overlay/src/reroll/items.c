#include "global.h"
#include "event_data.h"
#include "item.h"
#include "reroll/internal.h"
#include "reroll/reroll.h"
#include "constants/items.h"
#include "constants/vars.h"

static const u16 sPickupItems[] =
{
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_FULL_RESTORE,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_MAX_REVIVE,
    ITEM_ETHER,
    ITEM_MAX_ETHER,
    ITEM_ELIXIR,
    ITEM_MAX_ELIXIR,
    ITEM_GUARD_SPEC,
    ITEM_DIRE_HIT,
    ITEM_X_ATTACK,
    ITEM_X_DEFEND,
    ITEM_X_SPEED,
    ITEM_X_ACCURACY,
    ITEM_X_SPECIAL,
    ITEM_LUM_BERRY,
    ITEM_SITRUS_BERRY,
    ITEM_LEFTOVERS,
    ITEM_SHELL_BELL,
};

// The shop owns the returned pointer for the menu lifetime, so this buffer must
// outlive Reroll_FilterMartInventory's stack frame.
static EWRAM_DATA u16 sFilteredPokemartItems[ITEMS_COUNT] = {0};

bool8 Reroll_CanStoreItem(u16 itemId)
{
    // Capture is disabled at the inventory boundary: balls cannot be checked,
    // granted, purchased, or inserted through ordinary bag APIs.
    return itemId < FIRST_BALL || itemId > LAST_BALL;
}

const u16 *Reroll_FilterMartInventory(const u16 *itemsForSale)
{
    u16 sourceIndex;
    u16 destinationIndex = 0;

    for (sourceIndex = 0;
         itemsForSale[sourceIndex] != ITEM_NONE
      && destinationIndex < ARRAY_COUNT(sFilteredPokemartItems) - 1;
         sourceIndex++)
    {
        if (Reroll_CanStoreItem(itemsForSale[sourceIndex]))
            sFilteredPokemartItems[destinationIndex++] = itemsForSale[sourceIndex];
    }
    sFilteredPokemartItems[destinationIndex] = ITEM_NONE;
    return sFilteredPokemartItems;
}

void Reroll_RandomizePickup(void)
{
    // Story keys and HMs retain their identity so plot and side-quest flags still
    // describe the item the player actually received.
    if (GetItemPocket(gSpecialVar_0x8000) == POCKET_KEY_ITEMS
     || (gSpecialVar_0x8000 >= ITEM_HM01 && gSpecialVar_0x8000 <= ITEM_HM08))
        return;

    RerollRandom_Init();
    gSpecialVar_0x8000 = sPickupItems[RerollRandom_Range(ARRAY_COUNT(sPickupItems))];
    gSpecialVar_0x8001 = 1;
}

void Reroll_ReplaceBallGift(void)
{
    if (!Reroll_CanStoreItem(gSpecialVar_0x8000))
    {
        RerollRandom_Init();
        gSpecialVar_0x8000 = sPickupItems[RerollRandom_Range(ARRAY_COUNT(sPickupItems))];
        gSpecialVar_0x8001 = 1;
    }
}

void Reroll_HasSpareRepel(void)
{
    gSpecialVar_Result = CheckBagHasItem(ITEM_REPEL, 1)
                      || CheckBagHasItem(ITEM_SUPER_REPEL, 1)
                      || CheckBagHasItem(ITEM_MAX_REPEL, 1);
}

void Reroll_UseSpareRepel(void)
{
    u16 item = ITEM_NONE;

    // Prefer the longest remaining Repel so accepting the prompt has best value.
    if (CheckBagHasItem(ITEM_MAX_REPEL, 1))
        item = ITEM_MAX_REPEL;
    else if (CheckBagHasItem(ITEM_SUPER_REPEL, 1))
        item = ITEM_SUPER_REPEL;
    else if (CheckBagHasItem(ITEM_REPEL, 1))
        item = ITEM_REPEL;

    if (item != ITEM_NONE && RemoveBagItem(item, 1))
        VarSet(VAR_REPEL_STEP_COUNT, GetItemHoldEffectParam(item));
}
