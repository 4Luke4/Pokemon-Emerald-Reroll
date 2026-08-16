#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "reroll/reroll.h"
#include "sprite.h"
#include "constants/pokemon.h"

static EWRAM_DATA u8 sFollowerSpriteId = MAX_SPRITES;
static EWRAM_DATA u16 sFollowerSpecies = SPECIES_NONE;

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

    if (gMain.inBattle || gPlayerAvatar.spriteId >= MAX_SPRITES)
        return;
    species = GetMonData(&gPlayerParty[0], MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return;

    // Reuse the icon while the lead species is unchanged. This prevents a new
    // sprite allocation on every overworld frame.
    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback
     && sFollowerSpecies == species)
        return;

    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback)
        FreeAndDestroyMonIconSprite(&gSprites[sFollowerSpriteId]);

    LoadMonIconPalette(species);
    personality = GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY);
    sFollowerSpriteId = CreateMonIcon(species, FollowerSpriteCallback, 0, 0, 1,
                                      personality, TRUE);
    if (sFollowerSpriteId < MAX_SPRITES)
        sFollowerSpecies = species;
}
