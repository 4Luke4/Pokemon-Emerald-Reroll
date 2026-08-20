#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "reroll/internal.h"
#include "reroll/reroll.h"
#include "sprite.h"
#include "constants/pokemon.h"

static EWRAM_DATA u8 sFollowerSpriteId = MAX_SPRITES;
static EWRAM_DATA u16 sFollowerSpecies = SPECIES_NONE;

static void FollowerSpriteCallback(struct Sprite *sprite);

static void ResetFollowerState(void)
{
    sFollowerSpriteId = MAX_SPRITES;
    sFollowerSpecies = SPECIES_NONE;
}

static void DestroyFollower(void)
{
    // Sprite IDs are recycled after map and menu transitions. Only destroy the
    // recorded slot when it still belongs to this module.
    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback)
        FreeAndDestroyMonIconSprite(&gSprites[sFollowerSpriteId]);
    ResetFollowerState();
}

static u8 FindFirstConsciousPartyMon(void)
{
    u8 i;

    for (i = 0; i < gPlayerPartyCount && i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        u8 level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);

        if (GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && RerollPokemon_IsEligibleSpecies(species, level))
            return i;
    }
    return PARTY_SIZE;
}

static void FollowerSpriteCallback(struct Sprite *sprite)
{
    struct Sprite *playerSprite;
    s16 xOffset = 0;
    s16 yOffset = 0;

    UpdateMonIconFrame(sprite);
    if (gPlayerAvatar.spriteId >= MAX_SPRITES || !gSprites[gPlayerAvatar.spriteId].inUse)
    {
        sprite->invisible = TRUE;
        return;
    }

    playerSprite = &gSprites[gPlayerAvatar.spriteId];
    // Object-event coordinates are camera-relative. The follower must opt in
    // to the same global offset or it drifts across the map as the camera pans.
    sprite->coordOffsetEnabled = playerSprite->coordOffsetEnabled;
    switch (gObjectEvents[gPlayerAvatar.objectEventId].facingDirection)
    {
    case DIR_NORTH:
        yOffset = 18;
        break;
    case DIR_SOUTH:
        yOffset = -18;
        break;
    case DIR_WEST:
        xOffset = 18;
        break;
    case DIR_EAST:
        xOffset = -18;
        break;
    }
    sprite->x = playerSprite->x + xOffset;
    sprite->y = playerSprite->y + yOffset;
    sprite->subpriority = playerSprite->subpriority + (yOffset > 0);
    sprite->invisible = playerSprite->invisible;
}

void Reroll_UpdateFollower(void)
{
    u16 species;
    u32 personality;
    u8 partyIndex;

    if (gMain.inBattle || gPlayerAvatar.spriteId >= MAX_SPRITES)
        return;

    partyIndex = FindFirstConsciousPartyMon();
    if (partyIndex == PARTY_SIZE)
    {
        DestroyFollower();
        return;
    }
    species = GetMonData(&gPlayerParty[partyIndex], MON_DATA_SPECIES);

    // Reuse the icon while the lead species is unchanged. This prevents a new
    // sprite allocation on every overworld frame.
    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback
     && sFollowerSpecies == species)
        return;

    DestroyFollower();

    LoadMonIconPalette(species);
    personality = GetMonData(&gPlayerParty[partyIndex], MON_DATA_PERSONALITY);
    sFollowerSpriteId = CreateMonIcon(species, FollowerSpriteCallback, 0, 0, 1,
                                      personality, TRUE);
    if (sFollowerSpriteId < MAX_SPRITES)
    {
        // Match field-object camera semantics immediately; the callback keeps
        // this synchronized if the player sprite changes during a transition.
        gSprites[sFollowerSpriteId].coordOffsetEnabled = TRUE;
        sFollowerSpecies = species;
    }
    else
    {
        ResetFollowerState();
    }
}
