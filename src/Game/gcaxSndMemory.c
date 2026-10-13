/* gcaxSndMemory.c (name from the panic messages): sound heap, ARAM banks and ARAM requests of Sega's
 * GameCube AX sound library, in reverse address order (deferred inlining). Names other than
 * gcaxSndMalloc and gcaxNewAramBank, which its messages name, are placeholders. */
#include "Game/gcaxSnd.h"
#include <string.h>

/* Reserves size bytes (rounded up to 32) of the remaining ARAM for bank bank; returns 0, or -1
 * after a report for a bad bank number or too little ARAM. */
int gcaxNewAramBank(int bank, u32 size) {
    u32 aligned = (size + 31) & ~31;
    int result = -1;

    if (bank >= 0 && bank < GCAX_BANKS && lbl_805F1404->aramRest >= (s32)aligned) {
        result = 0;
        lbl_805F1404->aramBank[bank].addr = lbl_805F1404->aramNext;
        lbl_805F1404->aramBank[bank].size = aligned;
        lbl_805F1404->aramNext += aligned;
        lbl_805F1404->aramRest -= aligned;
    }
    if (result != 0) {
        OSReport("AX SDLIB: gcaxNewAramBank(%d,%d) failed (should not happen\n", bank, aligned);
    }
    return result;
}

/* Loads a bank file (0x10-byte header, then MPBW and MPBP sections): the MPBW data goes to the bank's
 * ARAM range through an ARAM request, the MPBP data is copied into the sound heap. Returns the
 * request, or a negative error for a bad section or a full bank. A callback of -1 means none. */
int fn_8043B05C(int bank, u8 *file, GcaxCallback callback, void *arg, int async) {
    GcaxFileHeader *param;
    GcaxFileHeader *wave = (GcaxFileHeader *)(file + 0x10);
    int request;

    if (wave->magic != GCAX_MAGIC || wave->kind != GCAX_KIND_MPBW) {
        return -1;
    }
    if (lbl_805F1404->aramBank[bank].size < wave->size) {
        return -2;
    }
    if (callback == (GcaxCallback)-1) {
        async = 0;
        callback = 0;
    }
    request = fn_8043AA7C((u32)(wave + 1), lbl_805F1404->aramBank[bank].addr, wave->size, callback,
                          arg, async);

    param = (GcaxFileHeader *)((u8 *)wave + wave->size + sizeof(GcaxFileHeader));
    if (param->magic != GCAX_MAGIC || param->kind != GCAX_KIND_MPBP) {
        return -3;
    }
    if (lbl_805F1404->bankData[bank].addr != 0) {
        fn_8043ACE8((void *)lbl_805F1404->bankData[bank].addr);
    }
    lbl_805F1404->bankData[bank].addr = (u32)gcaxSndMalloc(param->size);
    if (lbl_805F1404->bankData[bank].addr == 0) {
        return -4;
    }
    lbl_805F1404->bankData[bank].size = param->size;
    memcpy((void *)lbl_805F1404->bankData[bank].addr, param + 1, param->size);
    return request;
}

/* Copies a sound bank file (MSB section) into the sound heap, then optionally calls callback. */
int fn_8043AF64(int bank, GcaxFileHeader *file, GcaxCallback callback, void *arg, int notify) {
    if (file->magic != GCAX_MAGIC || file->kind != GCAX_KIND_MSB) {
        return -1;
    }
    if (callback == (GcaxCallback)-1) {
        notify = 0;
        callback = 0;
    }
    if (lbl_805F1404->soundData[bank].addr != 0) {
        fn_8043ACE8((void *)lbl_805F1404->soundData[bank].addr);
    }
    lbl_805F1404->soundData[bank].addr = (u32)gcaxSndMalloc(file->size);
    if (lbl_805F1404->soundData[bank].addr == 0) {
        return -2;
    }
    lbl_805F1404->soundData[bank].size = file->size;
    memcpy((void *)lbl_805F1404->soundData[bank].addr, file + 1, file->size);
    if (notify) {
        callback(arg);
    }
    return 0;
}

/* Sets up the sound heap in [base, base + size), aligned to 32 bytes. */
void fn_8043AEBC(u8 *base, u32 size) {
    BOOL enabled = OSDisableInterrupts();

    lbl_805F1404->heapHead = (GcaxMemBlock *)(((u32)base + 31) & ~31);
    size -= (32 - ((u32)base & 31)) & 31;
    lbl_805F1404->heapSize = size & ~3;
    gcaxSndResetHeap();
    OSRestoreInterrupts(enabled);
}

/* First-fit allocation of size bytes (rounded up to 32) from the sound heap; NULL on failure. */
void *gcaxSndMalloc(u32 size) {
    GcaxMemBlock *block = lbl_805F1404->heapHead;
    BOOL enabled = OSDisableInterrupts();
    u32 aligned = (size + 31) & ~31;
    GcaxMemBlock *next;
    u32 avail;

    for (; (next = gcaxSndNextBlock(block)) != NULL; block = next) {
        if (block->magic == GCAX_MEM_FREE) {
            avail = (u32)next - (u32)block - sizeof(GcaxMemBlock);
            if (avail >= aligned) {
                goto found;
            }
        }
    }
    /* report the total free memory; the message spells the function "gacxSndMalloc" */
    {
        u32 freeTotal = 0;
        BOOL enabled2 = OSDisableInterrupts();
        if (lbl_805F1404 != NULL && lbl_805F1404->heapHead != NULL) {
            for (block = lbl_805F1404->heapHead; (next = block->next) != NULL; block = next) {
                if (block->magic == GCAX_MEM_FREE) {
                    freeTotal += (u32)next - (u32)block - sizeof(GcaxMemBlock);
                }
            }
        }
        OSRestoreInterrupts(enabled2);
        OSReport("AX SDLIB: gacxSndMalloc(%d) failed (should not happen, rest=%d\n", size,
                 freeTotal);
    }
    OSRestoreInterrupts(enabled);
    return NULL;

found:
    /* split off the rest when it can hold another block */
    if (avail > aligned + sizeof(GcaxMemBlock)) {
        GcaxMemBlock *rest = (GcaxMemBlock *)((u8 *)(block + 1) + aligned);

        rest->magic = GCAX_MEM_FREE;
        rest->field_0x2 = 0;
        rest->next = next;
        block->next = rest;
    }
    block->magic = GCAX_MEM_USED;
    OSRestoreInterrupts(enabled);
    return block + 1;
}

/* Frees a block from gcaxSndMalloc, merging it with free neighbours. */
void fn_8043ACE8(void *ptr) {
    BOOL enabled = OSDisableInterrupts();
    GcaxMemBlock *block = (GcaxMemBlock *)ptr - 1;
    GcaxMemBlock *prev;

    block->magic = GCAX_MEM_FREE;
    if (block->next != NULL && block->next->magic == GCAX_MEM_FREE) {
        block->next = block->next->next;
    }

    prev = gcaxSndFindPrevBlock(block);
    if (prev != NULL && prev->magic == GCAX_MEM_FREE) {
        prev->next = block->next;
    }
    OSRestoreInterrupts(enabled);
}

/* ARQ completion: frees the request slot and calls its callback. */
static void fn_8043AC4C(u32 request) {
    int i;

    for (i = 0; i < GCAX_ARQ_REQUESTS; i++) {
        if ((u32)&lbl_805F1404->requests[i] == request) {
            goto found;
        }
    }
    GCAX_PANIC("AX SDLIB: Panic: arq not found at %s:%s\n");

found:
    /* release the slot before the callback, which may post the next request */
    lbl_805F1404->requestBusy[i] = 0;
    if (lbl_805F1404->requestCallback[i] != NULL) {
        lbl_805F1404->requestCallback[i](lbl_805F1404->requestCallbackArg[i]);
    }
}

/* Claims a free ARAM request slot.
 * The slot stays busy until fn_8043AC4C sees its transfer complete. */
static inline int gcaxSndGetFreeRequest(void) {
    int i;

    for (i = 0; i < GCAX_ARQ_REQUESTS; i++) {
        if (lbl_805F1404->requestBusy[i] == 0) {
            goto found;
        }
    }
    GCAX_PANIC("AX SDLIB: Panic: no more at %s:%s\n");

found:
    lbl_805F1404->requestBusy[i] = 1;
    return i;
}

/* Copies main memory to ARAM and waits until the transfer is done. */
void fn_8043AB6C(u32 source, u32 dest, u32 length, GcaxCallback callback, void *arg) {
    int i = gcaxSndGetFreeRequest();

    lbl_805F1404->requestCallback[i] = callback;
    lbl_805F1404->requestCallbackArg[i] = arg;
    ARQPostRequest(&lbl_805F1404->requests[i], lbl_805F1404->arqOwner, ARQ_TYPE_MRAM_TO_ARAM,
                   ARQ_PRIORITY_HIGH, source, dest, length, fn_8043AC4C);
    while (lbl_805F1404->requestBusy[i] != 0) {
    }
}

/* Copies main memory to ARAM; waits for the transfer unless async. Returns the request slot. */
int fn_8043AA7C(u32 source, u32 dest, u32 length, GcaxCallback callback, void *arg, int async) {
    int i = gcaxSndGetFreeRequest();
    volatile u8 *busy;

    lbl_805F1404->requestCallback[i] = callback;
    lbl_805F1404->requestCallbackArg[i] = arg;
    ARQPostRequest(&lbl_805F1404->requests[i], lbl_805F1404->arqOwner, ARQ_TYPE_MRAM_TO_ARAM,
                   ARQ_PRIORITY_HIGH, source, dest, length, fn_8043AC4C);
    busy = &lbl_805F1404->requestBusy[i];
    if (!async) {
        while (*busy != 0) {
        }
    }
    return i;
}
