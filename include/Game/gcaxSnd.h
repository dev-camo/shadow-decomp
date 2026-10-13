#ifndef GAME_GCAXSND_H
#define GAME_GCAXSND_H

#include <dolphin/ar.h>
#include <dolphin/ax.h>
#include <dolphin/os.h>
#include <dolphin/types.h>

/* Sega's GameCube AX sound library ("AX SDLIB" in its messages). Only the parts of its work area
 * used by recovered code are mapped; the type and field names are inferred. */

#define GCAX_ARQ_REQUESTS 64
#define GCAX_BANKS 16

/* Header of a block in the sound heap; the block's memory follows it. */
typedef struct GcaxMemBlock {
    /* 0x00 */ u16 magic; /* GCAX_MEM_FREE or GCAX_MEM_USED */
    /* 0x02 */ u16 field_0x2;
    /* 0x04 */ struct GcaxMemBlock *next;
    /* 0x08 */ u8 field_0x8[0x18];
} GcaxMemBlock;

#define GCAX_MEM_FREE 0x4D46 /* 'MF' */
#define GCAX_MEM_USED 0x4D42 /* 'MB' */

/* An address range: an ARAM bank, or a main-memory copy of a bank's data. */
typedef struct GcaxRange {
    /* 0x0 */ u32 addr;
    /* 0x4 */ u32 size;
} GcaxRange;

typedef void (*GcaxCallback)(void *arg);

/* A singly linked list of heap-allocated nodes; the node's payload follows its next pointer. */
typedef struct GcaxNode {
    /* 0x0 */ struct GcaxNode *next;
    /* 0x4 */ void *data;
} GcaxNode;

typedef struct GcaxList {
    /* 0x0 */ GcaxNode *head;
} GcaxList;

typedef void (*GcaxNodeFunc)(GcaxNode *node);
typedef int (*GcaxNodePredicate)(GcaxNode *node);

/* Bank data (the MPBP section copied into the sound heap). Its tables are located by words that
 * hold a count in the top byte and a byte offset from the bank header in the low 24 bits. */
#define GCAX_COUNT(word) ((word) >> 24)
#define GCAX_OFFSET(word) ((word) & 0xFFFFFF)

typedef struct GcaxBankHeader {
    /* 0x0 */ u32 samples;        /* GcaxSample table */
    /* 0x4 */ u32 velocityCurves; /* 128-byte curves */
    /* 0x8 */ u32 programs;       /* words locating each program's GcaxLayer table */
} GcaxBankHeader;

/* A sample: a DSP ADPCM header (addresses relative to the sample's ARAM data), its sample format
 * and its offset in the bank's ARAM range (0x50 bytes). */
typedef struct GcaxSample {
    /* 0x00 */ u32 numSamples;
    /* 0x04 */ u32 numNibbles;
    /* 0x08 */ u32 sampleRate;
    /* 0x0C */ u16 loopFlag;
    /* 0x0E */ u16 field_0xE;
    /* 0x10 */ u32 loopStart;
    /* 0x14 */ u32 end;
    /* 0x18 */ u32 current;
    /* 0x1C */ u16 coef[16];
    /* 0x3C */ u16 gain;
    /* 0x3E */ u16 predScale;
    /* 0x40 */ u16 yn1;
    /* 0x42 */ u16 yn2;
    /* 0x44 */ u16 loopPredScale;
    /* 0x46 */ u16 loopYn1;
    /* 0x48 */ u16 loopYn2;
    /* 0x4A */ u8 format; /* 0: ADPCM, 10: PCM16, 25: PCM8 */
    /* 0x4C */ u32 offset;
} GcaxSample;

/* A layer of a program (0x10 bytes). */
typedef struct GcaxLayer {
    /* 0x0 */ u32 regions; /* locates the GcaxRegion table */
    /* 0x4 */ u8 bendRangeUp;
    /* 0x5 */ u8 bendRangeDown;
    /* 0x6 */ u8 field_0x6[0xA];
} GcaxLayer;

/* A key/velocity region of a layer (0x40 bytes). */
typedef struct GcaxRegion {
    /* 0x00 */ u32 sample;
    /* 0x04 */ u8 field_0x4;
    /* 0x05 */ u8 velocityCurve;
    /* 0x06 */ u8 velocityMin;
    /* 0x07 */ u8 velocityMax;
    /* 0x08 */ u8 keyMin;
    /* 0x09 */ u8 keyMax;
    /* 0x0A */ u8 field_0xA;
    /* 0x0B */ u8 pan;
    /* 0x0C */ u8 rootKey;
    /* 0x0D */ u8 fineTune;
    /* 0x0E */ u8 field_0xE;
    /* 0x0F */ u8 field_0xF;
    /* 0x10 */ u8 attack;
    /* 0x11 */ u8 decay;
    /* 0x12 */ u8 field_0x12;
    /* 0x13 */ u8 sustain;
    /* 0x14 */ u8 release;
    /* 0x15 */ u8 envScale;
    /* 0x16 */ u8 field_0x16;
    /* 0x17 */ u8 field_0x17[0x29];
} GcaxRegion;

struct GcaxPlayer;
struct GcaxNote;

#define GCAX_VOICES 64
#define GCAX_NOTES 64
#define GCAX_CHANNELS 16

/* Voice flags */
#define GCAX_VOICE_USED 0x80000000
#define GCAX_VOICE_DROPPED 0x40000000 /* AX dropped the voice; it waits for a new one */
#define GCAX_VOICE_STARTED 0x2
#define GCAX_VOICE_STOPPED 0x1

/* One AX voice playing a region of an instrument (0x10C bytes). */
typedef struct GcaxVoice {
    /* 0x000 */ s32 flags; /* GCAX_VOICE_* */
    /* 0x004 */ s32 index;
    /* 0x008 */ s32 field_0x8;
    /* 0x00C */ s32 field_0xC;
    /* 0x010 */ f32 volume;
    /* 0x014 */ f32 pitchRatio;
    /* 0x018 */ s32 bendRangeUp;
    /* 0x01C */ s32 bendRangeDown;
    /* 0x020 */ s32 field_0x20;
    /* 0x024 */ s32 field_0x24;
    /* 0x028 */ s32 field_0x28;
    /* 0x02C */ f32 gainL;
    /* 0x030 */ f32 gainR;
    /* 0x034 */ AXVPB *vpb;
    /* 0x038 */ AXPBADDR addr;
    /* 0x048 */ AXPBSRC src;
    /* 0x056 */ AXPBADPCM adpcm;
    /* 0x07E */ AXPBADPCMLOOP adpcmLoop;
    /* 0x084 */ AXPBMIX mix;
    /* 0x0A8 */ f32 time;
    /* 0x0AC */ f32 envLevel; /* dB */
    /* 0x0B0 */ f32 envGain;
    /* 0x0B4 */ f32 attackTime;
    /* 0x0B8 */ f32 decayTime;
    /* 0x0BC */ f32 field_0xBC;
    /* 0x0C0 */ f32 releaseTime;
    /* 0x0C4 */ f32 sustainLevel;
    /* 0x0C8 */ f32 attackRate;
    /* 0x0CC */ f32 decayRate;
    /* 0x0D0 */ f32 field_0xD0;
    /* 0x0D4 */ f32 releaseRate;
    /* 0x0D8 */ s32 envState;
    /* 0x0DC */ s32 field_0xDC;
    /* 0x0E0 */ s32 field_0xE0;
    /* 0x0E4 */ s32 startBank; /* fn_8043CA7C's arguments, kept to restart the voice */
    /* 0x0E8 */ GcaxLayer *startLayer;
    /* 0x0EC */ GcaxBankHeader *startBankData;
    /* 0x0F0 */ GcaxRegion *startRegion;
    /* 0x0F4 */ s32 startKey;
    /* 0x0F8 */ u8 *startVelocityCurves;
    /* 0x0FC */ s32 startVelocity;
    /* 0x100 */ s32 velocity;
    /* 0x104 */ u32 field_0x104;
    /* 0x108 */ struct GcaxNote *note;
} GcaxVoice;

/* A sounding note and the voices playing it (0x14 bytes). */
typedef struct GcaxNote {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 numVoices;
    /* 0x02 */ u8 channel;
    /* 0x03 */ u8 key;
    /* 0x04 */ u8 velocity;
    /* 0x08 */ s32 priority;
    /* 0x0C */ struct GcaxPlayer *owner;
    /* 0x10 */ GcaxList voices;
} GcaxNote;

/* MIDI channel state of a player (0x48 bytes). */
typedef struct GcaxChannel {
    /* 0x00 */ u32 flags; /* bit 0: sustain pedal */
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ s32 bank;
    /* 0x0C */ s32 program;
    /* 0x10 */ s32 volume;
    /* 0x14 */ s32 expression;
    /* 0x18 */ s32 pan;
    /* 0x1C */ s32 pitchBend;
    /* 0x20 */ s32 field_0x20;
    /* 0x24 */ s32 field_0x24;
    /* 0x28 */ s32 field_0x28;
    /* 0x2C */ s32 pitchBendOffset; /* pitchBend - 0x2000 */
    /* 0x30 */ f32 gainR;
    /* 0x34 */ f32 gainL;
    /* 0x38 */ u32 field_0x38;
    /* 0x3C */ f32 field_0x3C;
    /* 0x40 */ f32 field_0x40;
    /* 0x44 */ u32 field_0x44;
} GcaxChannel;

#define GCAX_PLAYERS 8
#define GCAX_EVENTS 1024

/* A linear fade of a player value from start to target; the times count down in 5 ms ticks. */
typedef struct GcaxFade {
    /* 0x0 */ f32 target;
    /* 0x4 */ f32 start;
    /* 0x8 */ f32 time;
    /* 0xC */ f32 remaining; /* negative when no fade runs */
} GcaxFade;

/* A MIDI sequence player; only the members used so far are mapped. */
typedef struct GcaxPlayer {
    /* 0x0000 */ u32 flags; /* bit 31: in use */
    /* 0x0004 */ u8 field_0x4[0x4];
    /* 0x0008 */ f32 volume;
    /* 0x000C */ f32 field_0xC;
    /* 0x0010 */ f32 field_0x10;
    /* 0x0014 */ GcaxFade volumeFade;
    /* 0x0024 */ f32 pitchScale;
    /* 0x0028 */ f32 field_0x28;
    /* 0x002C */ GcaxFade pitchFade;
    /* 0x003C */ f32 field_0x3C;
    /* 0x0040 */ u8 field_0x40[0x60];
    /* 0x00A0 */ GcaxList notes; /* nodes whose data is a GcaxNote */
    /* 0x00A4 */ GcaxChannel channels[GCAX_CHANNELS];
    /* 0x0524 */ u32 events[GCAX_EVENTS]; /* queued MIDI: status, data 1, data 2, priority */
    /* 0x1524 */ s32 eventRead;
    /* 0x1528 */ s32 eventWrite;
    /* 0x152C */ s32 eventCount;
} GcaxPlayer;

typedef struct GcaxWork {
    /* 0x00000 */ u8 field_0x0[0x2C];
    /* 0x0002C */ f32 field_0x2C;
    /* 0x00030 */ u8 field_0x30[0x8];
    /* 0x00038 */ s32 maxVoices;
    /* 0x0003C */ s32 acquiredVoices;
    /* 0x00040 */ u8 field_0x40[0xC];
    /* 0x0004C */ s32 noteCursor;
    /* 0x00050 */ s32 voiceCursor;
    /* 0x00054 */ GcaxVoice voices[GCAX_VOICES];
    /* 0x04354 */ GcaxNote notes[GCAX_NOTES];
    /* 0x04854 */ GcaxMemBlock *heapHead;
    /* 0x04858 */ u32 heapSize;
    /* 0x0485C */ ARQRequest requests[GCAX_ARQ_REQUESTS];
    /* 0x0505C */ volatile u8 requestBusy[GCAX_ARQ_REQUESTS]; /* cleared by the ARQ callback */
    /* 0x0509C */ u32 arqOwner;
    /* 0x050A0 */ GcaxCallback requestCallback[GCAX_ARQ_REQUESTS];
    /* 0x051A0 */ void *requestCallbackArg[GCAX_ARQ_REQUESTS];
    /* 0x052A0 */ u8 field_0x52A0[0x8];
    /* 0x052A8 */ s32 aramRest;
    /* 0x052AC */ u32 aramNext;
    /* 0x052B0 */ u8 field_0x52B0[0x4];
    /* 0x052B4 */ u32 silenceAddr; /* ARAM sample used when a region has no loop (inferred) */
    /* 0x052B8 */ u32 config[16];  /* default sizes set by fn_8043B25C */
    /* 0x052F8 */ GcaxRange aramBank[GCAX_BANKS];
    /* 0x05378 */ GcaxRange bankData[GCAX_BANKS];
    /* 0x053F8 */ GcaxRange soundData[GCAX_BANKS];
    /* 0x05478 */ GcaxPlayer players[GCAX_PLAYERS];
    /* 0x0FDF8 */ u8 field_0xFDF8[0x2];
    /* 0x0FDFA */ u8 monoMix;
    /* 0x0FDFB */ u8 auxMix;
    /* 0x0FDFC */ struct GcaxNote *searchNote; /* the note fn_8043B810 looks for */
    /* 0x0FE00 */ u8 field_0xFE00[0x320];
    /* 0x10120 */ f32 field_0x10120[6];
    /* 0x10138 */ u8 field_0x10138[0x3900];
    /* 0x13A38 */ u8 field_0x13A38;
    /* 0x13A39 */ u8 field_0x13A39[0xB];
    /* 0x13A44 */ f32 field_0x13A44;
} GcaxWork;

/* Header of a bank file section: two magic words, then the size of the data that follows. */
typedef struct GcaxFileHeader {
    /* 0x0 */ u32 magic; /* 'gcax' */
    /* 0x4 */ u32 kind;  /* 'MSB ', 'MPBW' or 'MPBP' */
    /* 0x8 */ u32 field_0x8;
    /* 0xC */ u32 size;
} GcaxFileHeader;

#ifdef __cplusplus
extern "C" {
#endif

extern GcaxWork *lbl_805F1404;

/* dB to linear gain: the first entry whose threshold exceeds a dB value gives its gain. */
typedef struct GcaxCurvePoint {
    f32 value;
    f32 threshold; /* dB */
} GcaxCurvePoint;

extern GcaxCurvePoint lbl_8056B090[1025];

#define GCAX_MAGIC 0x67636178     /* 'gcax' */
#define GCAX_KIND_MSB 0x4D534220  /* 'MSB ' */
#define GCAX_KIND_MPBW 0x4D504257 /* 'MPBW' */
#define GCAX_KIND_MPBP 0x4D504250 /* 'MPBP' */

#define GCAX_PANIC(message)                                                                        \
    do {                                                                                           \
        OSReport(message, __FILE__, __LINE__);                                                     \
        for (;;) {                                                                                 \
        }                                                                                          \
    } while (0)

void *gcaxSndMalloc(u32 size);
void fn_8043ACE8(void *ptr);
int fn_8043AA7C(u32 source, u32 dest, u32 length, GcaxCallback callback, void *arg, int async);

/* fn_8043B25C.c: voices, notes and instruments */
void fn_8043B25C(void);
f32 fn_8043B3A0(f32 db);
GcaxNode *fn_8043B3DC(GcaxList *list, GcaxNode *node);
GcaxNode *fn_8043B3F4(GcaxList *list, GcaxNodePredicate predicate);
void fn_8043B44C(GcaxList *list, GcaxNodeFunc destroy);
void fn_8043B4B0(GcaxNode *target, GcaxList *list, GcaxNodeFunc destroy);
GcaxNode *fn_8043B560(u32 size, GcaxList *list, GcaxNodeFunc init);
void fn_8043B5FC(GcaxList *notes);
void fn_8043B6D0(GcaxList *notes);
void fn_8043B794(GcaxList *notes);
int fn_8043B810(GcaxNode *node);
void fn_8043B840(GcaxList *notes);
void fn_8043B868(GcaxList *notes, GcaxNode *ref);
void fn_8043B89C(GcaxNode *ref);
GcaxNode *fn_8043B8F0(GcaxList *notes, GcaxNode *ref);
GcaxNode *fn_8043B910(GcaxList *notes, GcaxNote *note);
GcaxNote *fn_8043B954(GcaxPlayer *player, int channel, int key, int velocity, int priority);
int fn_8043BB18(void);
void fn_8043BB54(GcaxNote *note);
GcaxNote *fn_8043BBC8(int priority, int count);
GcaxNote *fn_8043BCDC(int priority);
void fn_8043BD80(GcaxVoice *voice);
void fn_8043BE04(GcaxVoice *voice);
void fn_8043BE6C(GcaxVoice *voice);
void fn_8043C05C(GcaxList *voices);
void fn_8043C084(GcaxNode *ref);
GcaxNode *fn_8043C10C(GcaxList *voices, GcaxNode *ref);
GcaxNode *fn_8043C12C(GcaxList *voices, GcaxVoice *voice);
void fn_8043C170(GcaxVoice *voice, s32 bend, f32 scale);
void fn_8043C2A4(GcaxVoice *voice, GcaxPlayer *player);
void fn_8043C89C(GcaxList *voices, s32 bank, s32 program, s32 key, s32 velocity);
int fn_8043C9D0(s32 bank, s32 program, s32 key);
void fn_8043CA7C(s32 bank, GcaxVoice *voice, GcaxLayer *layer, GcaxBankHeader *header,
                 GcaxRegion *region, s32 key, u8 *velocityCurves, s32 velocity);
u32 fn_8043D20C(s32 cents, f32 base);
f32 fn_8043D318(s32 steps);
f32 fn_8043D3BC(s32 rootKey, s32 fineTune, s32 key);
int fn_8043D49C(void);
GcaxVoice *fn_8043D4D8(GcaxNote *note, s32 arg1, s32 arg2);
void fn_8043D61C(void);
void fn_8043D6FC(void *arg);

/* players and their MIDI messages (Game/fn_8043D7B4.c) */
void fn_8043D7B4(GcaxPlayer *player, f32 target, f32 time);
void fn_8043D870(GcaxPlayer *player);
void fn_8043D8E0(GcaxPlayer *player, f32 target, f32 time);
void fn_8043D99C(GcaxPlayer *player);
void fn_8043DA0C(GcaxPlayer *player);
void fn_8043DAB0(GcaxPlayer *player);
void fn_8043DAE4(GcaxPlayer *player, u8 channel, u16 bend);
void fn_8043DB08(GcaxPlayer *player, u8 channel, u8 control, u8 value);
void fn_8043DD14(GcaxPlayer *player);
void fn_8043E004(GcaxPlayer *player, u32 event);
void fn_8043E088(GcaxPlayer *player);
GcaxPlayer *fn_8043E0E8(void);
void fn_8043E228(GcaxPlayer *player, int channel);

/* Makes the whole heap one free block, ending in a used sentinel block. */
static inline void gcaxSndResetHeap(void) {
    BOOL enabled = OSDisableInterrupts();
    GcaxMemBlock *head = lbl_805F1404->heapHead;
    GcaxMemBlock *tail =
        (GcaxMemBlock *)((u8 *)head + lbl_805F1404->heapSize - sizeof(GcaxMemBlock));

    head->magic = GCAX_MEM_FREE;
    head->field_0x2 = 0;
    head->next = tail;
    tail->magic = GCAX_MEM_USED;
    tail->field_0x2 = 0;
    tail->next = 0;
    OSRestoreInterrupts(enabled);
}

/* The block after block in the heap. */
static inline GcaxMemBlock *gcaxSndNextBlock(GcaxMemBlock *block) {
    return block->next;
}

/* The heap block before block, or NULL when block is the first one or not in the heap. */
static inline GcaxMemBlock *gcaxSndFindPrevBlock(GcaxMemBlock *block) {
    GcaxMemBlock *prev = 0;
    GcaxMemBlock *cur;

    for (cur = lbl_805F1404->heapHead; cur->next != 0; cur = cur->next) {
        if (cur == block) {
            return prev;
        }
        prev = cur;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif
