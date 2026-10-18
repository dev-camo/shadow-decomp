/* Voices of Sega's GameCube AX sound library: AX voice setup from bank regions, envelopes, pitch
 * and mix, and voice allocation. The original file name is unknown. Definitions are in reverse
 * address order (deferred inlining). */
#include "Game/gcaxSnd.h"
#include <math.h>
#include <string.h>

static u8 lbl_8056D098[256] = {
    255, 255, 255, 255, 255, 255, 255, 251, 247, 243, 239, 235, 231, 227, 223, 219, 215, 209, 203,
    197, 193, 187, 183, 179, 177, 173, 169, 165, 163, 159, 157, 153, 151, 149, 147, 145, 143, 141,
    139, 137, 135, 133, 131, 129, 127, 125, 123, 122, 121, 119, 117, 116, 115, 113, 111, 110, 109,
    108, 107, 105, 103, 102, 101, 100, 99,  98,  97,  96,  95,  94,  93,  92,  91,  90,  89,  88,
    87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,  77,  76,  76,  75,  74,  73,  72,  71,
    71,  70,  70,  69,  68,  67,  66,  65,  65,  64,  64,  63,  62,  61,  61,  60,  60,  59,  59,
    58,  58,  57,  56,  55,  55,  54,  54,  53,  53,  52,  52,  51,  51,  50,  50,  49,  49,  48,
    48,  47,  47,  46,  46,  45,  45,  44,  44,  43,  43,  42,  42,  41,  41,  40,  40,  39,  39,
    38,  38,  37,  37,  36,  36,  35,  35,  35,  34,  34,  34,  33,  33,  32,  32,  31,  31,  30,
    30,  29,  29,  29,  28,  28,  28,  27,  27,  27,  26,  26,  26,  25,  25,  24,  24,  23,  23,
    23,  22,  22,  22,  21,  21,  21,  20,  20,  20,  19,  19,  19,  18,  18,  18,  17,  17,  17,
    16,  16,  16,  15,  15,  15,  14,  14,  14,  13,  13,  13,  13,  12,  12,  12,  12,  11,  11,
    11,  10,  10,  10,  9,   9,   9,   8,   8,   8,   7,   7,   7,   7,   6,   6,   6,   6,   5,
    5,   5,   5,   4,   4,   4,   4,   3,   3,
};

static u8 lbl_8056D198[32] = {
    64, 84, 92, 100, 106, 110, 114, 117, 119, 121, 122, 123, 124, 125, 126, 127,
    64, 43, 35, 27,  21,  17,  13,  10,  8,   6,   5,   4,   3,   2,   1,   0,
};

static u8 lbl_8056D1B8[128] = {
    31, 30, 29, 28, 27, 26, 25, 25, 24, 24, 23, 23, 22, 22, 22, 22, 21, 21, 21, 21, 20, 20,
    20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17,
    17, 17, 17, 17, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,
    5,  5,  6,  6,  6,  6,  7,  7,  8,  8,  9,  9,  10, 11, 12, 13, 14, 15,
};

static u8 lbl_8056D238[64] = {
    0,  1,  1,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,
    8,  8,  9,  9,  9,  9,  10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13,
    13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
};

static f32 lbl_8056D278[32] = {
    1.0f,    0.708008f,  0.5f,     0.354004f,   0.25f,     0.177002f,    0.125f,     0.088501f,
    0.0625f, 0.0442505f, 0.03125f, 0.02212525f, 0.015625f, 0.011062625f, 0.0078125f, 0.0f,
    1.0f,    1.0f,       1.0f,     1.0f,        1.0f,      1.0f,         1.0f,       1.0f,
    1.0f,    1.0f,       1.0f,     1.0f,        1.0f,      1.0f,         1.0f,       1.0f,
};

static f32 lbl_8056D2F8[64] = {
    1.0e+08f, 1.0e+08f, 8100.0f, 6900.0f, 6000.0f, 4800.0f, 4000.0f, 3400.0f, 3000.0f, 2400.0f,
    2000.0f,  1700.0f,  1500.0f, 1200.0f, 1000.0f, 860.0f,  760.0f,  600.0f,  500.0f,  430.0f,
    380.0f,   300.0f,   250.0f,  220.0f,  190.0f,  150.0f,  130.0f,  110.0f,  95.0f,   76.0f,
    63.0f,    55.0f,    47.0f,   38.0f,   31.0f,   27.0f,   24.0f,   19.0f,   15.0f,   13.0f,
    12.0f,    9.4f,     7.9f,    6.8f,    6.0f,    4.7f,    3.8f,    3.4f,    3.0f,    2.4f,
    2.0f,     1.8f,     1.6f,    1.3f,    1.1f,    0.93f,   0.85f,   0.65f,   0.53f,   0.44f,
    0.4f,     0.35f,    0.0f,    0.0f,
};

static f32 lbl_8056D3F8[64] = {
    1.0e+08f, 1.0e+08f, 118200.0f, 101300.0f, 88600.0f, 70900.0f, 59100.0f, 50700.0f,
    44300.0f, 35500.0f, 29600.0f,  25300.0f,  22200.0f, 17700.0f, 14800.0f, 12700.0f,
    11100.0f, 8900.0f,  7400.0f,   6300.0f,   5500.0f,  4400.0f,  3700.0f,  2800.0f,
    2200.0f,  1800.0f,  1600.0f,   1400.0f,   1100.0f,  920.0f,   790.0f,   690.0f,
    550.0f,   460.0f,   390.0f,    340.0f,    270.0f,   230.0f,   200.0f,   170.0f,
    140.0f,   110.0f,   98.0f,     85.0f,     68.0f,    57.0f,    49.0f,    43.0f,
    34.0f,    28.0f,    25.0f,     22.0f,     18.0f,    14.0f,    12.0f,    11.0f,
    8.5f,     7.1f,     6.1f,      5.4f,      4.3f,     3.6f,     3.1f,     0.0f,
};

static f32 lbl_8056D4F8[128] = {
    0.0f,      0.012368f, 0.024734f, 0.037097f, 0.049454f, 0.061803f, 0.074143f, 0.086471f,
    0.098786f, 0.111087f, 0.12337f,  0.135634f, 0.147877f, 0.160098f, 0.172295f, 0.184465f,
    0.196606f, 0.208718f, 0.220798f, 0.232844f, 0.244854f, 0.256827f, 0.268761f, 0.280654f,
    0.292503f, 0.304308f, 0.316066f, 0.327776f, 0.339436f, 0.351044f, 0.362598f, 0.374097f,
    0.385538f, 0.396921f, 0.408243f, 0.419502f, 0.430697f, 0.441826f, 0.452888f, 0.463881f,
    0.474802f, 0.485651f, 0.496425f, 0.507124f, 0.517745f, 0.528287f, 0.538748f, 0.549126f,
    0.559421f, 0.56963f,  0.579752f, 0.589785f, 0.599728f, 0.609579f, 0.619337f, 0.629f,
    0.638567f, 0.648036f, 0.657406f, 0.666676f, 0.675843f, 0.684908f, 0.693867f, 0.70272f,
    0.711466f, 0.720103f, 0.72863f,  0.737045f, 0.745348f, 0.753536f, 0.76161f,  0.769566f,
    0.777405f, 0.785125f, 0.792725f, 0.800204f, 0.807561f, 0.814793f, 0.821901f, 0.828884f,
    0.835739f, 0.842467f, 0.849066f, 0.855535f, 0.861873f, 0.868079f, 0.874153f, 0.880093f,
    0.885898f, 0.891567f, 0.897101f, 0.902497f, 0.907754f, 0.912873f, 0.917853f, 0.922692f,
    0.92739f,  0.931946f, 0.936359f, 0.940629f, 0.944755f, 0.948737f, 0.952574f, 0.956265f,
    0.959809f, 0.963207f, 0.966457f, 0.96956f,  0.972514f, 0.97532f,  0.977976f, 0.980482f,
    0.982839f, 0.985045f, 0.987101f, 0.989006f, 0.990759f, 0.992361f, 0.993811f, 0.995109f,
    0.996254f, 0.997248f, 0.998088f, 0.998776f, 0.999312f, 0.999694f, 0.999924f, 1.0f,
};
/* Resolves a bank offset word (low 24 bits) to an address in the bank */
static inline void *gcaxSndBankData(GcaxBankHeader *header, u32 word) {
    return (u8 *)header + GCAX_OFFSET(word);
}

/* Takes the next usable voice round-robin: a dropped voice that gets its AX voice back, or an idle
 * voice that still has one. NULL if there is none. A separate function in the E3 2005 prototype
 * (G9SJ8P) at 0x8037E0D0, called from fn_8043D4D8's counterpart 0x8037DFC0. */
static inline GcaxVoice *gcaxSndGetFreeVoice(void) {
    int i;

    for (i = 0; i < GCAX_VOICES; i++) {
        GcaxVoice *voice;

        lbl_805F1404->voiceCursor++;
        lbl_805F1404->voiceCursor %= GCAX_VOICES;
        voice = &lbl_805F1404->voices[lbl_805F1404->voiceCursor];
        if ((voice->flags & GCAX_VOICE_USED) && (voice->flags & GCAX_VOICE_DROPPED)) {
            AXVPB *vpb = AXAcquireVoice(27, fn_8043D6FC, (u32)voice);

            if (vpb != NULL) {
                lbl_805F1404->acquiredVoices++;
                voice->vpb = vpb;
                voice->flags &= ~GCAX_VOICE_DROPPED;
                return voice;
            }
        }
        if (!(voice->flags & GCAX_VOICE_USED) && voice->vpb != NULL) {
            return voice;
        }
    }
    return NULL;
}

/* Adds the attenuation for a 0-15 level (0 is silence) to db */
#define GCAX_ADD_LEVEL_DB(db, level)                                                               \
    do {                                                                                           \
        s32 index_ = (level) << 2;                                                                 \
        s32 step_;                                                                                 \
                                                                                                   \
        if (index_ < 0) {                                                                          \
            index_ = 0;                                                                            \
        }                                                                                          \
        if (index_ > 63) {                                                                         \
            index_ = 63;                                                                           \
        }                                                                                          \
        step_ = lbl_8056D238[index_];                                                              \
        if (step_ == 0) {                                                                          \
            (db) += -96.0f;                                                                        \
        } else {                                                                                   \
            (db) += (f32)(-(15 - step_) * 3);                                                      \
        }                                                                                          \
    } while (0)

/* AX drop callback: stops the dropped voice and tries to acquire a replacement AX voice. */
void fn_8043D6FC(void *arg) {
    GcaxVoice *voice;
    AXVPB *vpb = arg;

    voice = (GcaxVoice *)vpb->userContext;

    AXSetVoiceState(vpb, 0);
    MIXReleaseChannel(vpb);
    voice->flags |= GCAX_VOICE_DROPPED;
    lbl_805F1404->acquiredVoices--;
    voice->vpb = AXAcquireVoice(27, fn_8043D6FC, (u32)voice);
    if (voice->vpb != NULL) {
        voice->flags &= ~GCAX_VOICE_DROPPED;
        AXSetVoiceState(voice->vpb, 0);
        lbl_805F1404->acquiredVoices++;
    } else {
        OSReport("AX SDLIB: AXAcquireVoice() failed in drop callback\n");
    }
}

/* Clears the voices and acquires up to maxVoices (at most 64) AX voices for them. */
void fn_8043D61C(void) {
    int count;
    int i;

    AXSetMode(0);
    memset(lbl_805F1404->voices, 0, sizeof(lbl_805F1404->voices));
    for (i = 0; i < GCAX_VOICES; i++) {
        lbl_805F1404->voices[i].index = i;
    }
    count = lbl_805F1404->maxVoices;
    if (count > GCAX_VOICES) {
        count = GCAX_VOICES;
    }
    for (i = 0; i < count; i++) {
        lbl_805F1404->voices[i].vpb =
            AXAcquireVoice(27, fn_8043D6FC, (u32)&lbl_805F1404->voices[i]);
        if (lbl_805F1404->voices[i].vpb == NULL) {
            break;
        }
        lbl_805F1404->acquiredVoices++;
    }
}

/* Allocates the next free voice for note (round robin), reacquiring an AX voice for a dropped
 * one; NULL when none is free. */
GcaxVoice *fn_8043D4D8(GcaxNote *note, s32 arg1, s32 arg2) {
    GcaxVoice *voice = gcaxSndGetFreeVoice();

    if (voice != NULL) {
        voice->volume = 1.0f;
        voice->note = note;
        voice->field_0x8 = arg1;
        voice->field_0xC = arg2;
        voice->flags |= GCAX_VOICE_USED;
        voice->flags &= ~GCAX_VOICE_STOPPED;
        voice->flags &= ~GCAX_VOICE_STARTED;
        voice->field_0x104 = 0;
    }
    return voice;
}

/* The number of voices with any flag set. */
int fn_8043D49C(void) {
    int count = 0;
    int i;

    if (lbl_805F1404 != NULL) {
        for (i = 0; i < GCAX_VOICES; i++) {
            if (lbl_805F1404->voices[i].flags != 0) {
                count++;
            }
        }
    }
    return count;
}

/* The sample-rate ratio that plays a region with rootKey and fineTune (1/128 semitones) at key. */
f32 fn_8043D3BC(s32 rootKey, s32 fineTune, s32 key) {
    f64 ratio = 90316.8;
    int semitones;

    if (fineTune < 0) {
        rootKey++;
        fineTune += 128;
    }
    semitones = rootKey - key;
    if (semitones >= 0) {
        while (semitones >= 12) {
            ratio *= 0.5;
            semitones -= 12;
        }
        while (semitones > 0) {
            ratio /= 1.0594630943592953;
            semitones--;
        }
    } else {
        semitones = -semitones;
        while (semitones >= 12) {
            ratio *= 2.0;
            semitones -= 12;
        }
        while (semitones > 0) {
            ratio *= 1.0594630943592953;
            semitones--;
        }
    }
    while (fineTune > 16) {
        ratio *= 1.007246412223704;
        fineTune -= 16;
    }
    while (fineTune > 0) {
        ratio *= 1.0004513695322617;
        fineTune--;
    }
    return ratio;
}

/* 2 to the power of steps/3072 (steps of 1/256 semitone). The remainder below a semitone is
 * applied in pairs: an odd positive remainder overshoots to -1, which the separate negative pass
 * undoes with one division, so an odd count rounds down to the next lower even step. */
f32 fn_8043D318(s32 steps) {
    f64 ratio = 1.0;

    if (steps > 0) {
        while (steps >= 256) {
            ratio *= 1.0594630943592953;
            steps -= 256;
        }
        while (steps > 0) {
            ratio *= 1.0004513695322617;
            steps -= 2;
        }
    }
    if (steps < 0) {
        while (steps <= -256) {
            ratio /= 1.0594630943592953;
            steps += 256;
        }
        while (steps < 0) {
            ratio /= 1.0004513695322617;
            steps += 2;
        }
    }
    return ratio;
}

/* Applies a pitch bend in 1/8191 semitones to a sample-rate ratio; returns the ratio as AX's
 * 16.16 source ratio: below 0x200 becomes 0x200 and above 0x40000 becomes 0x3FFFF (0x40000 itself is
 * kept). */
u32 fn_8043D20C(s32 cents, f32 base) {
    f32 ratio = 1.0f;
    int semitones;
    int down = 0;
    int fine;
    u32 result;

    if (cents < 0) {
        down = 1;
        cents = -cents;
    }
    semitones = cents / 8191;
    fine = ((cents % 8191) << 7) / 8190;
    while (semitones >= 12) {
        ratio *= 2.0;
        semitones -= 12;
    }
    while (semitones > 0) {
        ratio *= 1.0594630943592953;
        semitones--;
    }
    while (fine > 16) {
        ratio *= 1.007246412223704;
        fine -= 16;
    }
    while (fine > 0) {
        ratio *= 1.0004513695322617;
        fine--;
    }
    if (down) {
        base /= ratio;
    } else {
        base *= ratio;
    }
    result = base;
    if (result < 0x200) {
        result = 0x200;
    }
    if (result > 0x40000) {
        result = 0x3FFFF;
    }
    return result;
}

/* Starts voice on region of a layer for key and velocity: sets its level, pan, pitch and
 * envelope from the region, programs its AX voice for the sample's format, and starts it. The
 * arguments are kept so that fn_8043BD80 can restart the voice. */
void fn_8043CA7C(s32 bank, GcaxVoice *voice, GcaxLayer *layer, GcaxBankHeader *header,
                 GcaxRegion *region, s32 key, u8 *velocityCurves, s32 velocity) {
    GcaxPlayer *player;
    GcaxSample *sample;
    int channel;
    u8 *curve;
    s32 cents;
    u32 ratio;
    f64 log2;
    s32 rate;
    s32 octave;
    s32 shift;
    f64 octaveBase;
    s32 fraction;
    s32 half;
    s32 index;

    voice->startBank = bank;
    voice->startLayer = layer;
    voice->startBankData = header;
    voice->startRegion = region;
    voice->startKey = key;
    voice->startVelocityCurves = velocityCurves;
    voice->startVelocity = velocity;
    sample = (GcaxSample *)gcaxSndBankData(header, header->samples);
    sample += region->sample;
    player = voice->note->owner;
    channel = voice->note->channel;
    curve = velocityCurves + region->velocityCurve * 128;

    /* velocity */
    if (velocity < region->velocityMin) {
        velocity = region->velocityMin;
    }
    if (velocity > region->velocityMax) {
        velocity = region->velocityMax;
    }
    if (velocity < 0) {
        velocity = 0;
    }
    if (velocity > 127) {
        velocity = 127;
    }
    velocity = curve[velocity];
    if (velocity < 0) {
        velocity = 0;
    }
    if (velocity > 127) {
        velocity = 127;
    }
    voice->velocity = velocity;
    voice->field_0x20 = region->field_0xA;
    voice->field_0x24 = lbl_8056D198[region->pan & 0x1F];
    voice->field_0x28 = region->field_0x16;
    voice->volume = 1.0f;

    /* pitch */
    voice->pitchRatio = fn_8043D3BC(region->rootKey, region->fineTune, key);
    voice->bendRangeUp = layer->bendRangeUp;
    voice->bendRangeDown = layer->bendRangeDown;
    cents = player->channels[channel].pitchBendOffset;
    if (cents >= 0) {
        cents *= voice->bendRangeUp;
    } else {
        cents *= voice->bendRangeDown;
    }
    ratio = fn_8043D20C(cents, voice->pitchRatio);
    voice->field_0xDC = region->field_0xE;
    voice->field_0xE0 = region->field_0xF;
    voice->src.ratioHi = ratio >> 16;
    voice->src.ratioLo = ratio;
    voice->src.currentAddressFrac = 0;
    voice->src.last_samples[0] = 0;
    voice->src.last_samples[1] = 0;
    voice->src.last_samples[2] = 0;
    voice->src.last_samples[3] = 0;

    /* envelope times scale with the playback rate's octave */
    log2 = log10(2.0);
    rate = (s32)(32000.0f * ((f32)ratio / 65536.0f) + 0.5f);
    octave = (s32)(log10(rate / 172.265625) / log2);
    shift = (octave - 8) & 0xF;
    octaveBase = 172.265625 / pow(2.0, -octave);
    fraction = (s32)(1024.0 * (rate - octaveBase) / octaveBase);
    if (shift > 8) {
        shift = -(16 - shift);
    }
    half = (fraction >> 9) & 1;
    index = region->envScale + shift + half + region->attack * 2;
    if (index < 0) {
        index = 0;
    }
    if (index > 63) {
        index = 63;
    }
    voice->attackTime = lbl_8056D2F8[index];
    index = region->envScale + shift + half + region->decay * 2;
    if (index < 0) {
        index = 0;
    }
    if (index > 63) {
        index = 63;
    }
    voice->decayTime = lbl_8056D3F8[index];
    index = region->envScale + shift + half + region->field_0x12 * 2;
    if (index < 0) {
        index = 0;
    }
    if (index > 63) {
        index = 63;
    }
    voice->field_0xBC = lbl_8056D3F8[index];
    index = region->envScale + shift + half + region->release * 2;
    if (index < 0) {
        index = 0;
    }
    if (index > 63) {
        index = 63;
    }
    voice->releaseTime = lbl_8056D3F8[index];
    if (voice->attackTime < 5.0f) {
        voice->attackRate = 1.0f;
    } else {
        voice->attackRate = 5.0f / voice->attackTime;
    }
    voice->envState = 0;
    if (voice->attackRate >= 1.0f) {
        voice->attackRate = 1.0f;
        voice->envLevel = 1.0f;
        voice->envGain = 0.0f;
        voice->envState = 1;
    } else {
        voice->envLevel = 0.0f;
        voice->envGain = -96.0f;
    }
    voice->decayRate = 5.0f / voice->decayTime;
    voice->field_0xD0 = 5.0f / voice->field_0xBC;
    voice->releaseRate = 5.0f / voice->releaseTime;
    if (region->sustain == 0) {
        voice->sustainLevel = 0.0f;
    } else {
        voice->sustainLevel = fn_8043B3A0(1.5f * -(f32)(31 - region->sustain));
    }
    voice->time = 0.0f;
    MIXInitChannel(voice->vpb, 0, 0, -60, -60, 64, 0, 0);

    switch (sample->format) {
    case 0: {
        /* ADPCM: addresses in nibbles */
        u32 loop = sample->loopStart + sample->offset * 2 + lbl_805F1404->aramBank[bank].addr * 2;
        u32 end = sample->end + sample->offset * 2 + lbl_805F1404->aramBank[bank].addr * 2;
        u32 current = sample->current + sample->offset * 2 + lbl_805F1404->aramBank[bank].addr * 2;

        voice->addr.loopFlag = sample->loopFlag;
        voice->addr.format = 0;
        voice->addr.loopAddressHi = loop >> 16;
        voice->addr.loopAddressLo = loop;
        voice->addr.endAddressHi = end >> 16;
        voice->addr.endAddressLo = end;
        voice->addr.currentAddressHi = current >> 16;
        voice->addr.currentAddressLo = current;
        memcpy(voice->adpcm.a, sample->coef, sizeof(sample->coef));
        voice->adpcm.gain = sample->gain;
        voice->adpcm.pred_scale = sample->predScale;
        voice->adpcm.yn1 = sample->yn1;
        voice->adpcm.yn2 = sample->yn2;
        voice->adpcmLoop.loop_pred_scale = sample->loopPredScale;
        voice->adpcmLoop.loop_yn1 = sample->loopYn1;
        voice->adpcmLoop.loop_yn2 = sample->loopYn2;
        AXSetVoiceAddr(voice->vpb, &voice->addr);
        AXSetVoiceSrcType(voice->vpb, 1);
        AXSetVoiceSrc(voice->vpb, &voice->src);
        AXSetVoiceAdpcm(voice->vpb, &voice->adpcm);
        AXSetVoiceAdpcmLoop(voice->vpb, &voice->adpcmLoop);
        fn_8043C2A4(voice, player);
        if (sample->numSamples >= 3) {
            AXSetVoiceState(voice->vpb, 1);
        }
        voice->flags |= GCAX_VOICE_STARTED;
        break;
    }
    case 10: {
        /* PCM16: addresses in 16-bit samples */
        u32 loop = sample->loopStart * 2 + sample->offset + lbl_805F1404->aramBank[bank].addr;
        u32 end = sample->end * 2 + sample->offset + lbl_805F1404->aramBank[bank].addr;
        u32 current = sample->current * 2 + sample->offset + lbl_805F1404->aramBank[bank].addr;

        if (sample->loopFlag == 0) {
            loop = lbl_805F1404->silenceAddr;
        }
        voice->addr.loopFlag = sample->loopFlag;
        voice->addr.format = 10;
        voice->addr.loopAddressHi = loop >> 17;
        voice->addr.loopAddressLo = loop >> 1;
        voice->addr.endAddressHi = end >> 17;
        voice->addr.endAddressLo = end >> 1;
        voice->addr.currentAddressHi = current >> 17;
        voice->addr.currentAddressLo = current >> 1;
        AXSetVoiceAddr(voice->vpb, &voice->addr);
        AXSetVoiceSrcType(voice->vpb, 1);
        AXSetVoiceSrc(voice->vpb, &voice->src);
        fn_8043C2A4(voice, player);
        AXSetVoiceState(voice->vpb, 1);
        voice->flags |= GCAX_VOICE_STARTED;
        break;
    }
    case 25: {
        /* PCM8: addresses in bytes */
        u32 loop = sample->loopStart + sample->offset + lbl_805F1404->aramBank[bank].addr;
        u32 end = sample->end + sample->offset + lbl_805F1404->aramBank[bank].addr;
        u32 current = sample->current + sample->offset + lbl_805F1404->aramBank[bank].addr;

        if (sample->loopFlag == 0) {
            loop = lbl_805F1404->silenceAddr;
        }
        voice->addr.loopFlag = sample->loopFlag;
        voice->addr.format = 25;
        voice->addr.loopAddressHi = loop >> 16;
        voice->addr.loopAddressLo = loop;
        voice->addr.endAddressHi = end >> 16;
        voice->addr.endAddressLo = end;
        voice->addr.currentAddressHi = current >> 16;
        voice->addr.currentAddressLo = current;
        AXSetVoiceAddr(voice->vpb, &voice->addr);
        AXSetVoiceSrcType(voice->vpb, 1);
        AXSetVoiceSrc(voice->vpb, &voice->src);
        fn_8043C2A4(voice, player);
        AXSetVoiceState(voice->vpb, 1);
        voice->flags |= GCAX_VOICE_STARTED;
        break;
    }
    }
}

/* The number of regions of program in bank whose key range holds key (voices a note needs). */
int fn_8043C9D0(s32 bank, s32 program, s32 key) {
    int count;
    GcaxLayer *layers;
    GcaxRegion *regions;
    GcaxBankHeader *header;
    int numLayers;
    int i;
    u32 *programs;
    u32 entry;
    int numRegions;
    int j;

    count = 0;
    header = (GcaxBankHeader *)lbl_805F1404->bankData[bank].addr;
    if (header != NULL && GCAX_COUNT(header->programs) > program) {
        programs = (u32 *)gcaxSndBankData(header, header->programs);
        programs += program;
        entry = *programs;
        numLayers = GCAX_COUNT(entry);
        layers = (GcaxLayer *)gcaxSndBankData(header, entry);

        for (i = 0; i < numLayers; i++) {
            numRegions = GCAX_COUNT(layers[i].regions);
            regions = (GcaxRegion *)gcaxSndBankData(header, layers[i].regions);

            for (j = 0; j < numRegions; j++) {
                if (regions[j].keyMin <= key && key <= regions[j].keyMax) {
                    count++;
                }
            }
        }
    }
    return count;
}

/* Starts the note's voices, one per region of program in bank whose key range holds key. */
void fn_8043C89C(GcaxList *voices, s32 bank, s32 program, s32 key, s32 velocity) {
    GcaxRegion *region;
    GcaxBankHeader *header = (GcaxBankHeader *)lbl_805F1404->bankData[bank].addr;
    GcaxNode *ref = NULL;

    if (header != NULL && GCAX_COUNT(header->programs) > program) {
        u32 *programs = (u32 *)gcaxSndBankData(header, header->programs);
        u32 entry;
        u8 *curves;
        int numLayers;
        int i;

        programs += program;
        entry = *programs;
        curves = gcaxSndBankData(header, header->velocityCurves);
        numLayers = GCAX_COUNT(entry);

        for (i = 0; i < numLayers; i++) {
            GcaxLayer *layers = (GcaxLayer *)gcaxSndBankData(header, entry);
            GcaxRegion *regions = (GcaxRegion *)gcaxSndBankData(header, layers[i].regions);
            int numRegions = GCAX_COUNT(layers[i].regions);
            int j;

            for (j = 0; j < numRegions; j++) {
                region = &regions[j];
                if (region->keyMin <= key && key <= region->keyMax) {
                    ref = fn_8043C10C(voices, ref);
                    if (ref != NULL && ref->data != NULL) {
                        fn_8043CA7C(bank, ref->data, &layers[i], header, region, key, curves,
                                    velocity);
                    }
                }
            }
        }
    }
}

/* Computes a voice's AX mix from its channel (volume, expression, pan, sends), its velocity and
 * envelope, and the library's mono/aux settings, and sends it to AX. */
void fn_8043C2A4(GcaxVoice *voice, GcaxPlayer *player) {
    int channel;
    s32 level;     /* volume, then table index, then attenuation in 0.375 dB steps */
    s32 noteLevel; /* voice velocity and level */
    f32 db;
    f32 gain;
    s32 pan;
    s32 panIndex;
    f32 gainB;
    f32 gainA;
    f32 sendScale;
    f32 send;
    f32 sendL;
    f32 sendR;

    if (!(player->flags & 0x80000000)) {
        return;
    }
    channel = voice->note->channel;
    voice->mix.vL = 0;
    voice->mix.vR = 0;
    voice->mix.vAuxAL = 0;
    voice->mix.vAuxAR = 0;
    voice->mix.vAuxBL = 0;
    voice->mix.vAuxBR = 0;
    voice->mix.vAuxBS = 0;
    voice->mix.vS = 0;
    voice->mix.vAuxAS = 0;
    voice->mix.vDeltaL = 0;
    voice->mix.vDeltaR = 0;
    voice->mix.vDeltaAuxAL = 0;
    voice->mix.vDeltaAuxAR = 0;
    voice->mix.vDeltaAuxBL = 0;
    voice->mix.vDeltaAuxBR = 0;
    voice->mix.vDeltaAuxBS = 0;
    voice->mix.vDeltaS = 0;
    voice->mix.vDeltaAuxAS = 0;

    /* channel volume and expression, voice velocity and level, player volume: an attenuation in
     * 0.375 dB steps */
    level = (player->channels[channel].volume + 1) * (player->channels[channel].expression + 1) - 1;
    level /= 128;
    if (level < 0) {
        level = 0;
    }
    if (level > 127) {
        level = 127;
    }
    noteLevel = (level + 1) * voice->velocity;
    noteLevel = (noteLevel * (256 - voice->field_0x28) - 1) >> 14 & 0xFF;
    level = (s32)(255.0f * player->volume);
    level += noteLevel;
    if (level < 0) {
        level = 0;
    }
    if (level > 255) {
        level = 255;
    }
    level = lbl_8056D098[level] - 3;
    if (level > 255) {
        level = 255;
    }
    db = 0.0f;
    GCAX_ADD_LEVEL_DB(db, voice->field_0x20);
    db += lbl_805F1404->field_0x2C;
    if (level & 0x1) {
        db += -0.375f;
    }
    if (level & 0x2) {
        db += -0.75f;
    }
    if (level & 0x4) {
        db += -1.5f;
    }
    if (level & 0x8) {
        db += -3.0f;
    }
    if (level & 0x10) {
        db += -6.0f;
    }
    if (level & 0x20) {
        db += -12.0f;
    }
    if (level & 0x40) {
        db += -24.0f;
    }
    if (level & 0x80) {
        db += -48.0f;
    }
    db += voice->envGain;
    gain = fn_8043B3A0(db);

    /* pan */
    pan = voice->field_0x24 + player->channels[channel].pan - 64;
    if (pan < 0) {
        pan = 0;
    }
    if (pan > 127) {
        pan = 127;
    }
    panIndex = lbl_8056D1B8[pan];
    gainA = lbl_8056D278[panIndex];
    gainB = lbl_8056D278[panIndex ^ 0x10];
    if (lbl_805F1404->monoMix) {
        f32 mono = 0.707f * (gainA + gainB);

        if (mono > 1.0f) {
            mono = 1.0f;
        }
        gainB = mono;
        gainA = mono;
    }
    voice->gainL = gainA;
    gainA *= gain;
    voice->gainR = gainB;
    gainB *= gain;
    player->channels[channel].gainL = gainA;
    player->channels[channel].gainR = gainB;

    if (lbl_805F1404->auxMix == 0) {
        voice->mix.vL = 32768.0f * (voice->volume * player->channels[channel].gainL);
        voice->mix.vR = 32768.0f * (voice->volume * player->channels[channel].gainR);
        voice->mix.vAuxBL = 0;
        voice->mix.vAuxBR = 0;
    } else {
        f32 left;
        f32 right;

        if (player->channels[channel].field_0x24 < 0.0f) {
            left = voice->volume * player->channels[channel].gainL;
            right = voice->volume * player->channels[channel].gainR;
        } else {
            left =
                lbl_8056D4F8[127 - player->channels[channel].field_0x24] * (voice->volume * gain);
            right = lbl_8056D4F8[player->channels[channel].field_0x24] * (voice->volume * gain);
        }
        if (player->channels[channel].field_0x20 <= 64) {
            voice->mix.vL = 32768.0f * left;
            voice->mix.vR = 32768.0f * right;
            voice->mix.vAuxBL = 0;
            voice->mix.vAuxBR = 0;
        } else {
            voice->mix.vL = 0;
            voice->mix.vR = 0;
            voice->mix.vAuxBL = 32768.0f * left;
            voice->mix.vAuxBR = 32768.0f * right;
        }
    }

    /* aux A send */
    sendScale = -2.0f;
    if (voice->field_0xDC == 0) {
        sendScale = 1.0f;
    }
    db = 0.0f;
    GCAX_ADD_LEVEL_DB(db, voice->field_0xE0);
    send = sendScale * fn_8043B3A0(db) + player->field_0x3C;
    if (send < 0.0f) {
        send = 0.0f;
    }
    if (send > 1.0f) {
        send = 1.0f;
    }
    sendL = gainA * send;
    sendR = gainB * send;
    voice->mix.vAuxAL = 32768.0f * sendL;
    voice->mix.vAuxAR = 32768.0f * sendR;
    if (lbl_805F1404->auxMix == 0) {
        voice->mix.vAuxAL = 32768.0f * sendL;
        voice->mix.vAuxAR = 32768.0f * sendR;
        voice->mix.vAuxAS = 0;
        voice->mix.vAuxBS = 0;
    } else {
        f32 dry = (127.0f - player->channels[channel].field_0x20) / 127.0f;

        voice->mix.vAuxAL = 32768.0f * (dry * sendL);
        voice->mix.vAuxAR = 32768.0f * (dry * sendR);
        /* the original writes vAuxAS twice; the second value is the one AX gets */
        voice->mix.vAuxAS = 32768.0f * (dry * sendL);
        voice->mix.vAuxAS = 32768.0f * (dry * sendR);
    }
    AXSetVoiceMix(voice->vpb, &voice->mix);
}

/* Applies a pitch bend (in units of 1/8192 semitone per bend-range step) and a pitch scale to
 * a voice's sample rate. */
void fn_8043C170(GcaxVoice *voice, s32 bend, f32 scale) {
    s32 cents;
    s32 amount;
    f32 ratio;
    f32 srcRatio;
    int down;
    int semitones;
    int fine;

    if (bend >= 0) {
        cents = bend * voice->bendRangeUp;
    } else {
        cents = bend * voice->bendRangeDown;
    }
    ratio = 1.0f;
    amount = cents;
    down = 0;
    if (cents < 0) {
        down = 1;
        amount = -cents;
    }
    semitones = amount / 8192;
    fine = ((amount % 8192) << 7) / 8192;
    while (semitones >= 12) {
        ratio *= 2.0;
        semitones -= 12;
    }
    while (semitones > 0) {
        ratio *= 1.0594630943592953;
        semitones--;
    }
    while (fine > 16) {
        ratio *= 1.007246412223704;
        fine -= 16;
    }
    while (fine > 0) {
        ratio *= 1.0004513695322617;
        fine--;
    }
    if (down) {
        ratio = 1.0f / ratio;
    }
    srcRatio = ratio * voice->pitchRatio;
    srcRatio /= 65536.0f;
    srcRatio *= scale;
    AXSetVoiceSrcRatio(voice->vpb, srcRatio);
}

/* Appends a reference to voice to a note's voice list. */
GcaxNode *fn_8043C12C(GcaxList *voices, GcaxVoice *voice) {
    GcaxNode *ref = fn_8043B560(sizeof(GcaxNode), voices, NULL);

    if (ref != NULL) {
        ref->data = voice;
    }
    return ref;
}

/* The next voice reference of a note's voice list (see fn_8043B3DC). fn_8043C89C calls it directly:
 * the E3 prototype makes that call out of line (0x8037D0F4); retail expands it there and keeps this
 * copy. The unit inlines only explicit inline functions, so this one needs auto_inline (a file-wide
 * auto_inline gives the same object). */
#pragma auto_inline on
GcaxNode *fn_8043C10C(GcaxList *voices, GcaxNode *ref) {
    return fn_8043B3DC(voices, ref);
}
#pragma auto_inline reset

/* Destroys a voice reference: stops its AX voice and marks the voice free. */
void fn_8043C084(GcaxNode *ref) {
    GcaxVoice *voice;

    if (ref != NULL && (voice = ref->data) != NULL) {
        voice->field_0x104 = 0;
        if (voice->vpb != NULL) {
            AXSetVoiceState(voice->vpb, 0);
            MIXReleaseChannel(voice->vpb);
        }
        voice->flags &= ~GCAX_VOICE_USED;
        voice->flags &= ~GCAX_VOICE_STOPPED;
        voice->flags &= ~GCAX_VOICE_STARTED;
        voice->field_0x8 = 0;
    }
}

/* Frees every voice reference of a note's voice list. */
void fn_8043C05C(GcaxList *voices) {
    fn_8043B44C(voices, fn_8043C084);
}

/* Advances a voice's envelope by one 5 ms step: attack, decay to the sustain level, sustain,
 * and release (states 0, 1, 3 and 5; 2 and 4 are transitions). */
void fn_8043BE6C(GcaxVoice *voice) {
    if (voice->flags & GCAX_VOICE_STOPPED) {
        return;
    }
    switch (voice->envState) {
    case 0:
        voice->envLevel += voice->attackRate;
        voice->envGain = -(96.0f - 96.0f * voice->envLevel);
        if (voice->envLevel >= 1.0f) {
            voice->envLevel = 1.0f;
            voice->envGain = -(96.0f - 96.0f * voice->envLevel);
            voice->envState++;
        }
        break;
    case 1:
        if (voice->envGain <= voice->sustainLevel) {
            voice->envState++;
        } else {
            voice->envLevel -= voice->decayRate;
            voice->envGain = -(96.0f - 96.0f * voice->envLevel);
            if (voice->envLevel <= 0.0f) {
                voice->envLevel = 0.0f;
                voice->envGain = -(96.0f - 96.0f * voice->envLevel);
            }
            if (!(voice->envGain <= voice->sustainLevel)) {
                break;
            }
            voice->envState++;
        }
        /* fallthrough */
    case 2:
        voice->envState++;
        /* fallthrough */
    case 3:
        voice->envLevel -= voice->field_0xD0;
        voice->envGain = -(96.0f - 96.0f * voice->envLevel);
        if (voice->envLevel <= 0.0f) {
            voice->envLevel = 0.0f;
            voice->envGain = -(96.0f - 96.0f * voice->envLevel);
        }
        break;
    case 4:
        voice->envState++;
        /* fallthrough */
    case 5:
        voice->envLevel -= voice->releaseRate;
        voice->envGain = -(96.0f - 96.0f * voice->envLevel);
        if (voice->envLevel <= 0.0f) {
            voice->envLevel = 0.0f;
            voice->envGain = -(96.0f - 96.0f * voice->envLevel);
        }
        break;
    }
    voice->time += 5.0f;
}

/* Stops a voice's AX voice (it can be restarted with fn_8043BD80). */
void fn_8043BE04(GcaxVoice *voice) {
    if (voice != NULL && voice->vpb != NULL && (voice->flags & GCAX_VOICE_USED) &&
        !(voice->flags & GCAX_VOICE_DROPPED)) {
        voice->flags |= GCAX_VOICE_STOPPED;
        AXSetVoiceState(voice->vpb, 0);
        MIXReleaseChannel(voice->vpb);
    }
}

/* Restarts a stopped voice with its saved start arguments, unless AX dropped it. */
void fn_8043BD80(GcaxVoice *voice) {
    if (voice != NULL && voice->vpb != NULL && (voice->flags & GCAX_VOICE_USED) &&
        (voice->flags & GCAX_VOICE_STOPPED)) {
        if (!(voice->flags & GCAX_VOICE_DROPPED)) {
            fn_8043CA7C(voice->startBank, voice, voice->startLayer, voice->startBankData,
                        voice->startRegion, voice->startKey, voice->startVelocityCurves,
                        voice->startVelocity);
        }
        voice->flags &= ~GCAX_VOICE_STOPPED;
    }
}
