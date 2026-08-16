#include "global.h"
#include "main.h"
#include "random.h"
#include "reroll/internal.h"
#include "save.h"

#define REROLL_CHACHA_ROUNDS 20

// ChaCha20 is used as a deterministic random stream after seeding. The GBA has
// no hardware entropy source, so timing, save identity, and the vanilla RNG are
// mixed together. This provides strong diffusion but cannot make an entropy
// claim against an attacker who controls every sampled input.
struct RerollRngState
{
    u32 input[16];
    u32 output[16];
    u8 outputIndex;
    bool8 initialized;
};

static EWRAM_DATA struct RerollRngState sRerollRng = {0};

static u32 RotateLeft(u32 value, u8 shift)
{
    return (value << shift) | (value >> (32 - shift));
}

#define CHACHA_QUARTER_ROUND(a, b, c, d) \
    do                                      \
    {                                       \
        (a) += (b);                         \
        (d) ^= (a);                         \
        (d) = RotateLeft((d), 16);          \
        (c) += (d);                         \
        (b) ^= (c);                         \
        (b) = RotateLeft((b), 12);          \
        (a) += (b);                         \
        (d) ^= (a);                         \
        (d) = RotateLeft((d), 8);           \
        (c) += (d);                         \
        (b) ^= (c);                         \
        (b) = RotateLeft((b), 7);           \
    } while (0)

static u32 MixEntropyWord(u32 value)
{
    // MurmurHash3's finalizer diffuses low-entropy hardware timing samples.
    value ^= value >> 16;
    value *= 0x85EBCA6B;
    value ^= value >> 13;
    value *= 0xC2B2AE35;
    value ^= value >> 16;
    return value;
}

static void RefillRandomBlock(void)
{
    u32 working[16];
    u8 i;

    for (i = 0; i < ARRAY_COUNT(working); i++)
        working[i] = sRerollRng.input[i];

    for (i = 0; i < REROLL_CHACHA_ROUNDS; i += 2)
    {
        CHACHA_QUARTER_ROUND(working[0], working[4], working[8], working[12]);
        CHACHA_QUARTER_ROUND(working[1], working[5], working[9], working[13]);
        CHACHA_QUARTER_ROUND(working[2], working[6], working[10], working[14]);
        CHACHA_QUARTER_ROUND(working[3], working[7], working[11], working[15]);
        CHACHA_QUARTER_ROUND(working[0], working[5], working[10], working[15]);
        CHACHA_QUARTER_ROUND(working[1], working[6], working[11], working[12]);
        CHACHA_QUARTER_ROUND(working[2], working[7], working[8], working[13]);
        CHACHA_QUARTER_ROUND(working[3], working[4], working[9], working[14]);
    }

    for (i = 0; i < ARRAY_COUNT(working); i++)
        sRerollRng.output[i] = working[i] + sRerollRng.input[i];

    // Words 12 and 13 form a 64-bit block counter.
    if (++sRerollRng.input[12] == 0)
        sRerollRng.input[13]++;
    sRerollRng.outputIndex = 0;
}

void RerollRandom_Init(void)
{
    u32 entropy;
    u8 i;

    if (sRerollRng.initialized)
        return;

    entropy = Random32();
    entropy ^= (u32)REG_VCOUNT << 24;
    entropy ^= gMain.vblankCounter1;
    entropy ^= gMain.vblankCounter2 << 16;
    entropy ^= T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    entropy ^= gSaveBlock2Ptr->playTimeVBlanks << 8;
    entropy ^= gSaveBlock2Ptr->playTimeSeconds << 16;
    entropy ^= gSaveBlock2Ptr->playTimeMinutes << 24;

    // The first four words are the standard ChaCha "expand 32-byte k" constant.
    sRerollRng.input[0] = 0x61707865;
    sRerollRng.input[1] = 0x3320646E;
    sRerollRng.input[2] = 0x79622D32;
    sRerollRng.input[3] = 0x6B206574;
    for (i = 4; i < 12; i++)
    {
        entropy = MixEntropyWord(entropy + 0x9E3779B9 + i);
        sRerollRng.input[i] = entropy;
    }
    sRerollRng.input[12] = 0;
    sRerollRng.input[13] = 0;
    sRerollRng.input[14] = MixEntropyWord(entropy ^ Random32());
    sRerollRng.input[15] = MixEntropyWord(entropy ^ gMain.vblankCounter1);
    sRerollRng.outputIndex = ARRAY_COUNT(sRerollRng.output);
    sRerollRng.initialized = TRUE;
}

static u32 NextRandomWord(void)
{
    RerollRandom_Init();
    if (sRerollRng.outputIndex >= ARRAY_COUNT(sRerollRng.output))
        RefillRandomBlock();
    return sRerollRng.output[sRerollRng.outputIndex++];
}

u32 RerollRandom_Range(u32 upperBound)
{
    u32 value;
    u32 limit;

    if (upperBound <= 1)
        return 0;

    // Rejection sampling prevents modulo bias for non-power-of-two ranges.
    limit = UINT32_MAX - (UINT32_MAX % upperBound);
    do
        value = NextRandomWord();
    while (value >= limit);
    return value % upperBound;
}
