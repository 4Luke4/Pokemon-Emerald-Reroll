#include "global.h"
#include "data.h"
#include "decompress.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "reroll/internal.h"
#include "reroll/reroll.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/pokemon.h"

#define FOLLOWER_TILE_TAG 0xF110
#define FOLLOWER_PALETTE_TAG 0xF111
#define FOLLOWER_FRAME_TILES 16
#define FOLLOWER_SHEET_SIZE (6 * 32 * 32 / 2)
#define FOLLOWER_TRAIL_DISTANCE 16
#define FOLLOWER_TRAIL_CAPACITY 64
#define FOLLOWER_EMOTE_LIFETIME 60
#define FOLLOWER_HIDDEN_PLAYER_FLAGS \
    (PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE \
   | PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER \
   | PLAYER_AVATAR_FLAG_FORCED_MOVE)

enum FollowerAnim
{
    FOLLOWER_ANIM_FACE_SOUTH,
    FOLLOWER_ANIM_FACE_NORTH,
    FOLLOWER_ANIM_FACE_WEST,
    FOLLOWER_ANIM_FACE_EAST,
    FOLLOWER_ANIM_WALK_SOUTH,
    FOLLOWER_ANIM_WALK_NORTH,
    FOLLOWER_ANIM_WALK_WEST,
    FOLLOWER_ANIM_WALK_EAST,
};

enum FollowerEmote
{
    FOLLOWER_EMOTE_EXCLAMATION,
    FOLLOWER_EMOTE_QUESTION,
    FOLLOWER_EMOTE_HEART,
};

struct FollowerTrailPoint
{
    s16 x;
    s16 y;
};

static EWRAM_DATA u8 sFollowerSpriteId = MAX_SPRITES;
static EWRAM_DATA u16 sFollowerSpecies = SPECIES_NONE;
static EWRAM_DATA u16 sFollowerGraphicSpecies = SPECIES_NONE;
static EWRAM_DATA bool8 sFollowerShiny = FALSE;
static EWRAM_DATA struct FollowerTrailPoint sFollowerTrail[FOLLOWER_TRAIL_CAPACITY] = {0};
static EWRAM_DATA u8 sFollowerTrailHead = 0;
static EWRAM_DATA u8 sFollowerTrailCount = 0;
static EWRAM_DATA s16 sFollowerLastX = 0;
static EWRAM_DATA s16 sFollowerLastY = 0;
static EWRAM_DATA u8 sFollowerDirection = DIR_SOUTH;
static EWRAM_DATA u8 sFollowerInteractionPartyIndex = PARTY_SIZE;
static EWRAM_DATA bool8 sFollowerInteracting = FALSE;

static void FollowerSpriteCallback(struct Sprite *sprite);
static void FollowerEmoteSpriteCallback(struct Sprite *sprite);

#include "follower_graphics.inc.c"

static const struct OamData sFollowerOam =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 2,
};

static const union AnimCmd sAnimFollowerFaceSouth[] =
{
    ANIMCMD_FRAME(0 * FOLLOWER_FRAME_TILES, 16),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerFaceNorth[] =
{
    ANIMCMD_FRAME(2 * FOLLOWER_FRAME_TILES, 16),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerFaceWest[] =
{
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 16),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerFaceEast[] =
{
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 16, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerWalkSouth[] =
{
    ANIMCMD_FRAME(0 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_FRAME(1 * FOLLOWER_FRAME_TILES, 12),
    ANIMCMD_FRAME(0 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerWalkNorth[] =
{
    ANIMCMD_FRAME(2 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_FRAME(3 * FOLLOWER_FRAME_TILES, 12),
    ANIMCMD_FRAME(2 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerWalkWest[] =
{
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_FRAME(5 * FOLLOWER_FRAME_TILES, 12),
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 6),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnimFollowerWalkEast[] =
{
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 6, .hFlip = TRUE),
    ANIMCMD_FRAME(5 * FOLLOWER_FRAME_TILES, 12, .hFlip = TRUE),
    ANIMCMD_FRAME(4 * FOLLOWER_FRAME_TILES, 6, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sFollowerAnimTable[] =
{
    [FOLLOWER_ANIM_FACE_SOUTH] = sAnimFollowerFaceSouth,
    [FOLLOWER_ANIM_FACE_NORTH] = sAnimFollowerFaceNorth,
    [FOLLOWER_ANIM_FACE_WEST] = sAnimFollowerFaceWest,
    [FOLLOWER_ANIM_FACE_EAST] = sAnimFollowerFaceEast,
    [FOLLOWER_ANIM_WALK_SOUTH] = sAnimFollowerWalkSouth,
    [FOLLOWER_ANIM_WALK_NORTH] = sAnimFollowerWalkNorth,
    [FOLLOWER_ANIM_WALK_WEST] = sAnimFollowerWalkWest,
    [FOLLOWER_ANIM_WALK_EAST] = sAnimFollowerWalkEast,
};

static const struct SpriteTemplate sFollowerSpriteTemplate =
{
    .tileTag = FOLLOWER_TILE_TAG,
    .paletteTag = FOLLOWER_PALETTE_TAG,
    .oam = &sFollowerOam,
    .anims = sFollowerAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = FollowerSpriteCallback,
};

static const u8 sFollowerExclamationGfx[] = INCGFX_U8(
    "graphics/field_effects/pics/emotion_exclamation.png", ".4bpp");
static const u8 sFollowerQuestionGfx[] = INCGFX_U8(
    "graphics/field_effects/pics/emotion_question.png", ".4bpp");
static const u8 sFollowerHeartGfx[] = INCGFX_U8(
    "graphics/field_effects/pics/emotion_heart.png", ".4bpp");

static const struct SpriteFrameImage sFollowerEmoteImages[] =
{
    {.data = sFollowerExclamationGfx, .size = sizeof(sFollowerExclamationGfx)},
    {.data = sFollowerQuestionGfx, .size = sizeof(sFollowerQuestionGfx)},
    {.data = sFollowerHeartGfx, .size = sizeof(sFollowerHeartGfx)},
};

static const union AnimCmd sAnimFollowerEmoteExclamation[] =
{
    ANIMCMD_FRAME(FOLLOWER_EMOTE_EXCLAMATION, FOLLOWER_EMOTE_LIFETIME),
    ANIMCMD_END,
};

static const union AnimCmd sAnimFollowerEmoteQuestion[] =
{
    ANIMCMD_FRAME(FOLLOWER_EMOTE_QUESTION, FOLLOWER_EMOTE_LIFETIME),
    ANIMCMD_END,
};

static const union AnimCmd sAnimFollowerEmoteHeart[] =
{
    ANIMCMD_FRAME(FOLLOWER_EMOTE_HEART, FOLLOWER_EMOTE_LIFETIME),
    ANIMCMD_END,
};

static const union AnimCmd *const sFollowerEmoteAnimTable[] =
{
    [FOLLOWER_EMOTE_EXCLAMATION] = sAnimFollowerEmoteExclamation,
    [FOLLOWER_EMOTE_QUESTION] = sAnimFollowerEmoteQuestion,
    [FOLLOWER_EMOTE_HEART] = sAnimFollowerEmoteHeart,
};

static const struct OamData sFollowerEmoteOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const struct SpriteTemplate sFollowerEmoteSpriteTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sFollowerEmoteOam,
    .anims = sFollowerEmoteAnimTable,
    .images = sFollowerEmoteImages,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = FollowerEmoteSpriteCallback,
};

static const u8 sTextFollowerAsleep[] = _("{STR_VAR_1} is dozing as it walks.");
static const u8 sTextFollowerPoisoned[] = _("{STR_VAR_1} looks unwell from\npoison.");
static const u8 sTextFollowerBurned[] = _("{STR_VAR_1} is enduring its burn.");
static const u8 sTextFollowerFrozen[] = _("{STR_VAR_1} is trembling from the\ncold.");
static const u8 sTextFollowerParalyzed[] = _("{STR_VAR_1} is moving stiffly.");
static const u8 sTextFollowerTired[] = _("{STR_VAR_1} looks worn out.\nA rest would help.");
static const u8 sTextFollowerHappy[] = _("{STR_VAR_1} is delighted to be\ntraveling with you!");
static const u8 sTextFollowerNeutral[] = _("{STR_VAR_1} is watching its\nsurroundings carefully.");
static const u8 sTextFollowerUnavailable[] = _("Your follower is no longer nearby.");

static s16 AbsDelta(s16 value)
{
    return value < 0 ? -value : value;
}

static void GetDirectionDelta(u8 direction, s16 *dx, s16 *dy)
{
    *dx = 0;
    *dy = 0;
    switch (direction)
    {
    case DIR_NORTH:
        *dy = -1;
        break;
    case DIR_SOUTH:
        *dy = 1;
        break;
    case DIR_WEST:
        *dx = -1;
        break;
    case DIR_EAST:
        *dx = 1;
        break;
    }
}

static u8 GetDirectionFromDelta(s16 dx, s16 dy, u8 fallback)
{
    if (AbsDelta(dx) > AbsDelta(dy))
        return dx < 0 ? DIR_WEST : DIR_EAST;
    if (dy != 0)
        return dy < 0 ? DIR_NORTH : DIR_SOUTH;
    return fallback;
}

static u8 GetFollowerAnim(u8 direction, bool8 moving)
{
    u8 base = moving ? FOLLOWER_ANIM_WALK_SOUTH : FOLLOWER_ANIM_FACE_SOUTH;

    switch (direction)
    {
    case DIR_NORTH:
        return base + 1;
    case DIR_WEST:
        return base + 2;
    case DIR_EAST:
        return base + 3;
    default:
        return base;
    }
}

static void ResetFollowerTrail(const struct Sprite *playerSprite)
{
    s16 dx;
    s16 dy;

    sFollowerDirection = gObjectEvents[gPlayerAvatar.objectEventId].facingDirection;
    GetDirectionDelta(sFollowerDirection, &dx, &dy);

    // Seed one full tile behind the player. Subsequent samples preserve the
    // exact route around corners instead of cutting diagonally through walls.
    sFollowerTrail[0].x = playerSprite->x - dx * FOLLOWER_TRAIL_DISTANCE;
    sFollowerTrail[0].y = playerSprite->y - dy * FOLLOWER_TRAIL_DISTANCE;
    sFollowerTrail[1].x = playerSprite->x;
    sFollowerTrail[1].y = playerSprite->y;
    sFollowerTrailHead = 1;
    sFollowerTrailCount = 2;
    sFollowerLastX = sFollowerTrail[0].x;
    sFollowerLastY = sFollowerTrail[0].y;
}

static void RecordPlayerPosition(const struct Sprite *playerSprite)
{
    struct FollowerTrailPoint *latest = &sFollowerTrail[sFollowerTrailHead];
    s16 dx = playerSprite->x - latest->x;
    s16 dy = playerSprite->y - latest->y;

    if (dx == 0 && dy == 0)
        return;

    // A discontinuity means a warp, map reload, or sprite recreation. Starting
    // a fresh trail prevents the follower from visibly crossing the screen.
    if (AbsDelta(dx) + AbsDelta(dy) > FOLLOWER_TRAIL_DISTANCE / 2)
    {
        ResetFollowerTrail(playerSprite);
        return;
    }

    sFollowerTrailHead = (sFollowerTrailHead + 1) % FOLLOWER_TRAIL_CAPACITY;
    sFollowerTrail[sFollowerTrailHead].x = playerSprite->x;
    sFollowerTrail[sFollowerTrailHead].y = playerSprite->y;
    if (sFollowerTrailCount < FOLLOWER_TRAIL_CAPACITY)
        sFollowerTrailCount++;
}

static struct FollowerTrailPoint GetFollowerTrailPosition(void)
{
    struct FollowerTrailPoint newer = sFollowerTrail[sFollowerTrailHead];
    struct FollowerTrailPoint result = newer;
    s16 distance = 0;
    u8 i;
    u8 index = sFollowerTrailHead;

    for (i = 1; i < sFollowerTrailCount; i++)
    {
        struct FollowerTrailPoint older;
        s16 dx;
        s16 dy;
        s16 segment;

        index = (index + FOLLOWER_TRAIL_CAPACITY - 1) % FOLLOWER_TRAIL_CAPACITY;
        older = sFollowerTrail[index];
        dx = older.x - newer.x;
        dy = older.y - newer.y;
        segment = AbsDelta(dx) + AbsDelta(dy);
        if (distance + segment >= FOLLOWER_TRAIL_DISTANCE && segment != 0)
        {
            s16 remainder = FOLLOWER_TRAIL_DISTANCE - distance;

            result.x = newer.x + dx * remainder / segment;
            result.y = newer.y + dy * remainder / segment;
            if (!sFollowerInteracting)
                sFollowerDirection = GetDirectionFromDelta(-dx, -dy, sFollowerDirection);
            return result;
        }
        distance += segment;
        newer = older;
        result = older;
    }
    return result;
}

static void ResetFollowerState(void)
{
    sFollowerSpriteId = MAX_SPRITES;
    sFollowerSpecies = SPECIES_NONE;
    sFollowerGraphicSpecies = SPECIES_NONE;
    sFollowerShiny = FALSE;
    sFollowerTrailCount = 0;
    sFollowerInteractionPartyIndex = PARTY_SIZE;
    sFollowerInteracting = FALSE;
}

static void DestroyFollower(void)
{
    // Sprite IDs are recycled after map and menu transitions. Only destroy the
    // recorded slot when it still belongs to this module.
    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback)
        DestroySprite(&gSprites[sFollowerSpriteId]);

    FreeSpriteTilesByTag(FOLLOWER_TILE_TAG);
    FreeSpritePaletteByTag(FOLLOWER_PALETTE_TAG);
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

static u16 GetFollowerGraphicSpecies(u16 species, u32 personality)
{
    if (species == SPECIES_UNOWN)
    {
        u16 letter = GetUnownLetterByPersonality(personality);

        return letter == 0 ? SPECIES_UNOWN : letter + SPECIES_UNOWN_B - 1;
    }
    return species;
}

static void FollowerSpriteCallback(struct Sprite *sprite)
{
    struct Sprite *playerSprite;
    struct FollowerTrailPoint position;
    bool8 moving;
    u8 anim;

    if (gPlayerAvatar.spriteId >= MAX_SPRITES
     || !gSprites[gPlayerAvatar.spriteId].inUse
     || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
    {
        sprite->invisible = TRUE;
        return;
    }

    playerSprite = &gSprites[gPlayerAvatar.spriteId];
    if (sFollowerTrailCount == 0)
        ResetFollowerTrail(playerSprite);
    RecordPlayerPosition(playerSprite);
    position = GetFollowerTrailPosition();
    moving = position.x != sFollowerLastX || position.y != sFollowerLastY;
    sFollowerLastX = position.x;
    sFollowerLastY = position.y;

    // Match object-event camera and priority semantics while retaining an
    // independent 32x32 HGSS sheet and animation state.
    sprite->coordOffsetEnabled = playerSprite->coordOffsetEnabled;
    sprite->x = position.x;
    sprite->y = position.y;
    sprite->oam.priority = playerSprite->oam.priority;
    sprite->subpriority = playerSprite->subpriority;
    if (position.y < playerSprite->y && sprite->subpriority != 0)
        sprite->subpriority--;
    else if (position.y > playerSprite->y && sprite->subpriority != 0xFF)
        sprite->subpriority++;

    anim = GetFollowerAnim(sFollowerDirection, moving);
    StartSpriteAnimIfDifferent(sprite, anim);
    sprite->invisible = playerSprite->invisible
                     || (gPlayerAvatar.flags & FOLLOWER_HIDDEN_PLAYER_FLAGS);
}

static void FollowerEmoteSpriteCallback(struct Sprite *sprite)
{
    if (sFollowerSpriteId >= MAX_SPRITES
     || !gSprites[sFollowerSpriteId].inUse
     || gSprites[sFollowerSpriteId].callback != FollowerSpriteCallback
     || ++sprite->data[0] >= FOLLOWER_EMOTE_LIFETIME)
    {
        DestroySprite(sprite);
        return;
    }

    sprite->coordOffsetEnabled = gSprites[sFollowerSpriteId].coordOffsetEnabled;
    sprite->x = gSprites[sFollowerSpriteId].x;
    sprite->y = gSprites[sFollowerSpriteId].y - 24;
    sprite->subpriority = gSprites[sFollowerSpriteId].subpriority;
    if (sprite->subpriority != 0xFF)
        sprite->subpriority++;
    sprite->invisible = gSprites[sFollowerSpriteId].invisible;
}

static void ShowFollowerEmote(u8 emote)
{
    u8 spriteId = CreateSpriteAtEnd(&sFollowerEmoteSpriteTemplate, 0, 0, 0x52);

    if (spriteId >= MAX_SPRITES)
        return;
    StartSpriteAnim(&gSprites[spriteId], emote);
    if (emote == FOLLOWER_EMOTE_HEART)
        gSprites[spriteId].oam.paletteNum = 2;
}

bool8 Reroll_TryInteractWithFollower(s16 x, s16 y, u8 elevation)
{
    struct Sprite *playerSprite;
    struct Sprite *followerSprite;
    s16 playerX;
    s16 playerY;
    s16 dx;
    s16 dy;
    u8 partyIndex;

    if (sFollowerSpriteId >= MAX_SPRITES
     || !gSprites[sFollowerSpriteId].inUse
     || gSprites[sFollowerSpriteId].callback != FollowerSpriteCallback
     || gSprites[sFollowerSpriteId].invisible
     || gPlayerAvatar.spriteId >= MAX_SPRITES
     || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT
     || elevation != PlayerGetElevation())
        return FALSE;

    playerSprite = &gSprites[gPlayerAvatar.spriteId];
    followerSprite = &gSprites[sFollowerSpriteId];
    if (AbsDelta(playerSprite->x - followerSprite->x)
      + AbsDelta(playerSprite->y - followerSprite->y) != FOLLOWER_TRAIL_DISTANCE)
        return FALSE;

    PlayerGetDestCoords(&playerX, &playerY);
    GetDirectionDelta(sFollowerDirection, &dx, &dy);
    if (x != playerX - dx || y != playerY - dy)
        return FALSE;

    partyIndex = FindFirstConsciousPartyMon();
    if (partyIndex == PARTY_SIZE)
        return FALSE;

    sFollowerInteractionPartyIndex = partyIndex;
    sFollowerInteracting = TRUE;
    sFollowerDirection = GetOppositeDirection(GetPlayerFacingDirection());
    ScriptContext_SetupScript(EventScript_RerollFollowerInteraction);
    return TRUE;
}

void Reroll_ShowFollowerMessage(void)
{
    const u8 *message = sTextFollowerUnavailable;
    u8 emote = FOLLOWER_EMOTE_QUESTION;

    if (sFollowerInteractionPartyIndex < PARTY_SIZE)
    {
        struct Pokemon *mon = &gPlayerParty[sFollowerInteractionPartyIndex];
        u32 status = GetMonData(mon, MON_DATA_STATUS);
        u16 hp = GetMonData(mon, MON_DATA_HP);
        u16 maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        u8 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);

        GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
        message = sTextFollowerNeutral;
        if (status & STATUS1_SLEEP)
            message = sTextFollowerAsleep;
        else if (status & STATUS1_PSN_ANY)
            message = sTextFollowerPoisoned;
        else if (status & STATUS1_BURN)
            message = sTextFollowerBurned;
        else if (status & STATUS1_FREEZE)
            message = sTextFollowerFrozen;
        else if (status & STATUS1_PARALYSIS)
            message = sTextFollowerParalyzed;
        else if (maxHp != 0 && hp * 4 <= maxHp)
            message = sTextFollowerTired;
        else if (friendship >= 200)
        {
            message = sTextFollowerHappy;
            emote = FOLLOWER_EMOTE_HEART;
        }

        if (status != 0 || (maxHp != 0 && hp * 4 <= maxHp))
            emote = FOLLOWER_EMOTE_EXCLAMATION;
    }

    ShowFollowerEmote(emote);
    ShowFieldMessage(message);
}

void Reroll_EndFollowerInteraction(void)
{
    sFollowerInteractionPartyIndex = PARTY_SIZE;
    sFollowerInteracting = FALSE;
}

void Reroll_UpdateFollower(void)
{
    struct CompressedSpriteSheet sheet;
    struct CompressedSpritePalette palette;
    u16 species;
    u16 graphicSpecies;
    u32 personality;
    bool8 shiny;
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
    personality = GetMonData(&gPlayerParty[partyIndex], MON_DATA_PERSONALITY);
    shiny = IsMonShiny(&gPlayerParty[partyIndex]);
    graphicSpecies = GetFollowerGraphicSpecies(species, personality);

    // Keep a valid follower allocation across frames, but recreate it when the
    // first conscious party member or its shiny palette changes.
    if (sFollowerSpriteId < MAX_SPRITES
     && gSprites[sFollowerSpriteId].inUse
     && gSprites[sFollowerSpriteId].callback == FollowerSpriteCallback
     && sFollowerSpecies == species
     && sFollowerGraphicSpecies == graphicSpecies
     && sFollowerShiny == shiny)
        return;

    DestroyFollower();
    if (graphicSpecies >= ARRAY_COUNT(sFollowerGraphics)
     || sFollowerGraphics[graphicSpecies] == NULL)
        return;

    sheet.data = sFollowerGraphics[graphicSpecies];
    sheet.size = FOLLOWER_SHEET_SIZE;
    sheet.tag = FOLLOWER_TILE_TAG;
    LoadCompressedSpriteSheet(&sheet);
    if (GetSpriteTileStartByTag(FOLLOWER_TILE_TAG) == 0xFFFF)
        return;

    palette.data = shiny ? gMonShinyPaletteTable[species].data
                         : gMonPaletteTable[species].data;
    palette.tag = FOLLOWER_PALETTE_TAG;
    LoadCompressedSpritePalette(&palette);
    if (IndexOfSpritePaletteTag(FOLLOWER_PALETTE_TAG) == 0xFF)
    {
        DestroyFollower();
        return;
    }

    sFollowerSpriteId = CreateSprite(&sFollowerSpriteTemplate, 0, 0, 0);
    if (sFollowerSpriteId >= MAX_SPRITES)
    {
        DestroyFollower();
        return;
    }

    ResetFollowerTrail(&gSprites[gPlayerAvatar.spriteId]);
    sFollowerSpecies = species;
    sFollowerGraphicSpecies = graphicSpecies;
    sFollowerShiny = shiny;
}
