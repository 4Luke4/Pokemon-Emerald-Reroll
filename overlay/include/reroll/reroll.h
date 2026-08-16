#ifndef GUARD_REROLL_REROLL_H
#define GUARD_REROLL_REROLL_H

#include "global.h"

struct Pokemon;

// Party lifecycle and progression hooks.
void Reroll_EnsurePlayerParty(void);
void Reroll_OnBattleEnd(void);

// Trainer generation and trainer-tier battle policy.
u8 Reroll_CreateTrainerParty(struct Pokemon *party, u16 trainerId, bool8 firstTrainer);
u32 Reroll_GetTrainerAiFlags(u16 trainerId);
u16 Reroll_GetTrainerItem(u16 trainerId, u8 itemSlot);

// Capture prevention, mart filtering, pickups, and modern Repel behavior.
bool8 Reroll_CanStoreItem(u16 itemId);
const u16 *Reroll_FilterMartInventory(const u16 *itemsForSale);
void Reroll_RandomizePickup(void);
void Reroll_ReplaceBallGift(void);
void Reroll_HasSpareRepel(void);
void Reroll_UseSpareRepel(void);

// Virtual HM helpers keep story traversal independent of party movesets.
u8 Reroll_FindVirtualHmUser(u16 move);
bool8 Reroll_HasVirtualSurfUser(void);

// Overworld follower and irreversible run failure.
void Reroll_UpdateFollower(void);
void Reroll_GameOver(void);

#endif // GUARD_REROLL_REROLL_H
