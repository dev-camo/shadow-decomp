#ifndef GAME_GCAXSND_H
#define GAME_GCAXSND_H

#include <dolphin/ar.h>
#include <dolphin/os.h>
#include <dolphin/types.h>

/* Sega's GameCube AX sound library ("AX SDLIB" in its messages). Only the parts of its work area
 * used by gcaxSndMemory.c are mapped; the type and field names are inferred. */

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

typedef struct GcaxWork {
    /* 0x0000 */ u8 field_0x0[0x4854];
    /* 0x4854 */ GcaxMemBlock *heapHead;
    /* 0x4858 */ u32 heapSize;
    /* 0x485C */ ARQRequest requests[GCAX_ARQ_REQUESTS];
    /* 0x505C */ volatile u8 requestBusy[GCAX_ARQ_REQUESTS]; /* cleared by the ARQ callback */
    /* 0x509C */ u32 arqOwner;
    /* 0x50A0 */ GcaxCallback requestCallback[GCAX_ARQ_REQUESTS];
    /* 0x51A0 */ void *requestCallbackArg[GCAX_ARQ_REQUESTS];
    /* 0x52A0 */ u8 field_0x52A0[0x8];
    /* 0x52A8 */ s32 aramRest;
    /* 0x52AC */ u32 aramNext;
    /* 0x52B0 */ u8 field_0x52B0[0x48];
    /* 0x52F8 */ GcaxRange aramBank[GCAX_BANKS];
    /* 0x5378 */ GcaxRange bankData[GCAX_BANKS];
    /* 0x53F8 */ GcaxRange soundData[GCAX_BANKS];
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
