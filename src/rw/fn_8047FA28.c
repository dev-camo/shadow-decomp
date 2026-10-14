/* RenderWare free lists, which allocate fixed size entries in blocks, and the engine's memory
 * functions. The original file name is unknown. */
#include "rw/rwcore.h"
#include <stdlib.h>

#define rwMEMHINTDUR_FUNCTION 0x10000
#define rwMEMHINTDUR_GLOBAL 0x40000
#define rwMEMHINTDUR_MASK 0xFF0000

/* free list flags (inferred) */
#define rwFREELISTFLAG_STATIC 0x1     /* the free list is not allocated by its creator */
#define rwFREELISTFLAG_FREEBLOCKS 0x2 /* blocks are freed when their last entry is */

/* The alignment of entries when a free list is created with none */
#define rwFREELISTDEFAULTALIGNMENT 32

/* The free list of free lists: its entries per block and alignment */
#define rwFREELISTFREELISTENTRIES 16
#define rwFREELISTFREELISTALIGNMENT 32

/* A block's bitmap follows its link */
#define rwFREELISTBLOCKBITMAP(block) ((RwUInt8 *)(block) + sizeof(RwLLLink))

/* The entries of a block start after the bitmap, at the alignment */
#define rwFREELISTBLOCKENTRIES(block, heapSize, alignment)                                         \
    ((RwUInt32)(rwFREELISTBLOCKBITMAP(block) + (heapSize) + (alignment) - 1) & ~((alignment) - 1))

/* The free list a link in the list of all free lists belongs to */
#define FREELISTFROMLINK(link) ((RwFreeList *)((RwUInt8 *)(link) - offsetof(RwFreeList, link)))

RwFreeList lbl_805E2A80; /* the free list of free lists */

RwBool lbl_805EDEA0 = TRUE; /* whether free lists preallocate blocks (inferred) */

RwLinkList lbl_805F1570;  /* all free lists */
RwFreeList *lbl_805F156C; /* the free list of free lists, while it is open */
RwBool lbl_805F1568;      /* whether free lists are open (inferred) */

void fn_8047FA28(RwBool preallocate) {
    lbl_805EDEA0 = preallocate;
}

/* Creates a free list, or initializes freeList when it is not NULL, with numBlocksToPreallocate
 * blocks */
RwFreeList *fn_8047FA30(RwUInt32 entrySize, RwUInt32 entriesPerBlock, RwUInt32 alignment,
                        RwUInt32 numBlocksToPreallocate, RwFreeList *freeList, RwUInt32 hint) {
    RwUInt32 heapSize;
    RwUInt32 blockSize;
    RwUInt32 alignedSize;

    if (!lbl_805EDEA0) {
        numBlocksToPreallocate = 0;
    }
    if (alignment == 0) {
        alignment = rwFREELISTDEFAULTALIGNMENT;
    }

    if (freeList == NULL) {
        if (lbl_805F156C != NULL) {
            freeList = RwFreeListAlloc(lbl_805F156C, hint & rwMEMHINTDUR_MASK);
        } else {
            freeList = RwMalloc(sizeof(RwFreeList), hint & rwMEMHINTDUR_MASK);
        }
        if (freeList == NULL) {
            return NULL;
        }
        freeList->flags = rwFREELISTFLAG_FREEBLOCKS;
    } else {
        freeList->flags = rwFREELISTFLAG_STATIC | rwFREELISTFLAG_FREEBLOCKS;
    }

    alignedSize = (entrySize + (alignment - 1)) & ~(alignment - 1);
    heapSize = (entriesPerBlock + 7) >> 3;
    freeList->entrySize = alignedSize;
    freeList->entriesPerBlock = entriesPerBlock;
    freeList->alignment = alignment;
    freeList->heapSize = heapSize;
    rwLinkListInitialize(&freeList->blockList);

    blockSize = sizeof(RwLLLink) + heapSize + entriesPerBlock * alignedSize + alignment - 1;
    while (numBlocksToPreallocate != 0) {
        RwLLLink *block = RwMalloc(blockSize, hint);

        if (block == NULL) {
            fn_8047FC24(freeList);
            return NULL;
        }
        block->prev = NULL;
        block->next = NULL;
        rwLinkListAddLLLink(&freeList->blockList, block);
        memset(rwFREELISTBLOCKBITMAP(block), 0, heapSize);
        numBlocksToPreallocate--;
    }

    rwLinkListAddLLLink(&lbl_805F1570, &freeList->link);
    return freeList;
}

RwFreeList *fn_8047FBD8(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwUInt32 hint) {
    return fn_8047FA30(entrySize, entriesPerBlock, alignment, 1, NULL, hint);
}

RwFreeList *fn_8047FC04(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwInt32 numBlocksToPreallocate, RwFreeList *freeList, RwUInt32 hint) {
    return fn_8047FA30(entrySize, entriesPerBlock, alignment, numBlocksToPreallocate, freeList,
                       hint);
}

/* Frees a free list's blocks, and the free list unless it is static */
void fn_8047FC24(RwFreeList *freeList) {
    RwLLLink *block = freeList->blockList.link.next;
    RwLLLink *end = &freeList->blockList.link;

    while (block != end) {
        rwLinkListRemoveLLLink(block);
        RwFree(block);
        block = freeList->blockList.link.next;
        end = &freeList->blockList.link;
    }

    if (!(freeList->flags & rwFREELISTFLAG_STATIC)) {
        if (lbl_805F156C == freeList || lbl_805F156C == NULL) {
            RwFree(freeList);
        } else {
            RwFreeListFree(lbl_805F156C, freeList);
        }
    }
}

RwBool fn_8047FCDC(RwFreeList *freeList) {
    rwLinkListRemoveLLLink(&freeList->link);
    fn_8047FC24(freeList);
    return TRUE;
}

void *fn_8047FDB0(RwFreeList *freeList, RwUInt32 hint) {
    RwUInt32 heapSize = freeList->heapSize;
    RwLLLink *block = freeList->blockList.link.next;
    RwLLLink *end = &freeList->blockList.link;
    RwUInt32 entry = 0;
    RwUInt8 *bitmap;
    RwUInt32 entriesLeft;
    RwUInt32 index;
    RwUInt32 i;
    RwUInt32 bit;
    RwUInt8 bits;
    RwUInt8 mask;

    /* The first entry not in use */

    while (block != end && entry == 0) {
        bitmap = rwFREELISTBLOCKBITMAP(block);
        entriesLeft = freeList->entriesPerBlock;
        index = 0;
        for (i = 0; i < heapSize; i++) {
            bits = bitmap[i];
            if (bits != 0xFF) {
                for (bit = 0; bit < 8 && entriesLeft != 0; bit++, entriesLeft--) {
                    mask = 0x80 >> bit;
                    if (!(bits & mask)) {
                        bitmap[i] = mask | bits;
                        entry = rwFREELISTBLOCKENTRIES(block, heapSize, freeList->alignment) +
                                freeList->entrySize * (index + bit);
                        break;
                    }
                }
            } else {
                entriesLeft -= 8;
            }
            if (entry != 0) {
                break;
            }
            index += 8;
        }
        block = block->next;
    }

    /* Otherwise a new block */
    if (entry == 0) {
        block =
            RwMalloc(sizeof(RwLLLink) + heapSize + freeList->entriesPerBlock * freeList->entrySize +
                         freeList->alignment - 1,
                     hint);
        if (block == NULL) {
            return NULL;
        }
        bitmap = rwFREELISTBLOCKBITMAP(block);
        memset(bitmap, 0, heapSize);
        rwLinkListAddLLLink(&freeList->blockList, block);
        *bitmap = 0x80;
        entry = rwFREELISTBLOCKENTRIES(block, heapSize, freeList->alignment);
    }
    return (void *)entry;
}

RwFreeList *fn_8047FF50(RwFreeList *freeList, void *mem) {
    RwUInt32 heapSize = freeList->heapSize;
    RwLLLink *block = freeList->blockList.link.next;

    RwLLLink *end = &freeList->blockList.link;

    while (block != end) {
        RwUInt8 *entries = rwFREELISTBLOCKBITMAP(block) + heapSize;

        if ((RwUInt8 *)mem >= entries &&
            (RwUInt8 *)mem <= entries + freeList->entriesPerBlock * freeList->entrySize) {
            RwUInt8 *bitmap = rwFREELISTBLOCKBITMAP(block);
            RwUInt32 index = ((RwUInt8 *)mem - entries) / freeList->entrySize;

            bitmap[index >> 3] &= (RwUInt8) ~(RwUInt8)(0x80 >> (index - (index & ~7)));

            if (freeList->flags & rwFREELISTFLAG_FREEBLOCKS) {
                RwUInt32 used = 0;
                RwUInt32 i;

                for (i = 0; i < heapSize; i++) {
                    used += bitmap[i];
                }
                if (used == 0) {
                    rwLinkListRemoveLLLink(block);
                    RwFree(block);
                }
            }
            return freeList;
        }
        block = block->next;
    }
    return NULL;
}

/* Calls callBack on each entry in use */
RwFreeList *fn_804800DC(RwFreeList *freeList, void (*callBack)(void *mem, void *data), void *data) {
    RwUInt32 heapSize = freeList->heapSize;
    RwLLLink *block = freeList->blockList.link.next;

    while (block != &freeList->blockList.link) {
        RwUInt8 *blockBitmap = rwFREELISTBLOCKBITMAP(block);
        RwUInt8 *bitmap = RwMalloc(heapSize, rwMEMHINTDUR_FUNCTION);
        RwLLLink *next;
        RwUInt8 *bits;
        RwUInt32 index;
        RwUInt32 i;

        if (bitmap == NULL) {
            return NULL;
        }

        /* The callback may free entries, so a copy of the bitmap is walked */
        memcpy(bitmap, blockBitmap, heapSize);
        next = block->next;
        bits = bitmap;
        index = 0;
        for (i = 0; i < heapSize; i++) {
            RwUInt8 byte = *bits;

            if (byte != 0) {
                RwUInt32 entryIndex = index;
                RwUInt32 bit;

                for (bit = 0; bit < 8; bit++) {
                    if (byte & (RwUInt8)(0x80 >> bit)) {
                        callBack(
                            (void *)(rwFREELISTBLOCKENTRIES(block, heapSize, freeList->alignment) +
                                     freeList->entrySize * entryIndex),
                            data);
                    }
                    entryIndex++;
                }
            }
            index += 8;
            bits++;
        }
        RwFree(bitmap);
        block = next;
    }
    return freeList;
}

void *fn_80480214(RwUInt32 size, RwUInt32 hint) {
    return malloc(size);
}

void *fn_80480234(void *mem, RwUInt32 newSize, RwUInt32 hint) {
    return realloc(mem, newSize);
}

void *fn_80480254(RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint) {
    return calloc(numObj, sizeObj);
}

/* Opens free lists (inferred): the list of them and the free list of free lists, which is not in
 * that list */
static inline RwBool FreeListsOpen(void) {
    rwLinkListInitialize(&lbl_805F1570);
    lbl_805F1568 = TRUE;
    lbl_805F156C = fn_8047FA30(sizeof(RwFreeList), rwFREELISTFREELISTENTRIES,
                               rwFREELISTFREELISTALIGNMENT, 0, &lbl_805E2A80, rwMEMHINTDUR_GLOBAL);
    if (lbl_805F156C == NULL) {
        lbl_805F1568 = FALSE;
        return FALSE;
    }
    rwLinkListRemoveLLLink(&lbl_805F156C->link);
    return TRUE;
}

/* Opens free lists and sets the engine's memory functions */
RwBool fn_80480274(const RwMemoryFunctions *memFuncs) {
    if (!FreeListsOpen()) {
        return FALSE;
    }

    if (memFuncs != NULL) {
        lbl_805F15B4->memoryFuncs = *memFuncs;
    } else {
        lbl_805F15B4->memoryFuncs.rwmalloc = fn_80480214;
        lbl_805F15B4->memoryFuncs.rwfree = free;
        lbl_805F15B4->memoryFuncs.rwrealloc = fn_80480234;
        lbl_805F15B4->memoryFuncs.rwcalloc = fn_80480254;
    }
    return TRUE;
}

/* Destroys all free lists, then the free list of free lists */
void fn_8048038C(void) {
    RwLLLink *link = lbl_805F1570.link.next;
    RwLLLink *end = &lbl_805F1570.link;

    while (link != end) {
        RwFreeList *freeList = FREELISTFROMLINK(link);

        fn_8047FCDC(freeList);
        link = lbl_805F1570.link.next;
        end = &lbl_805F1570.link;
    }
    fn_8047FCDC(lbl_805F156C);
    lbl_805F156C = NULL;
    lbl_805F1568 = FALSE;
}

RwMemoryFunctions *fn_80480530(void) {
    return &lbl_805F15B4->memoryFuncs;
}
