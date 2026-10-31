/* RenderWare free lists, which allocate fixed size entries in blocks, and the engine's memory
 * functions. The original file name is unknown. */
#include "rw/rwcore.h"
#include <stdlib.h>

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
    RwUInt8 bits;
    RwUInt8 mask;
    void *entry = NULL;
    RwUInt32 i;
    RwUInt32 bit;
    RwUInt32 heapSize;
    RwUInt8 *bitmap;
    RwLLLink *block;
    RwLLLink *end;
    RwUInt32 entriesLeft;

    heapSize = freeList->heapSize;
    block = freeList->blockList.link.next;
    end = &freeList->blockList.link;

    /* The first entry not in use */
    while (block != end && entry == NULL) {
        bitmap = rwFREELISTBLOCKBITMAP(block);
        entriesLeft = freeList->entriesPerBlock;

        for (i = 0; i < heapSize; i++) {
            bits = bitmap[i];

            if (bits != 0xFF) {
                bit = 0;
                while (bit < 8 && entriesLeft > 0) {
                    mask = 0x80 >> bit;
                    if ((bits & mask) == 0) {
                        RwUInt8 *entries;

                        bitmap[i] = bitmap[i] | mask;
                        entries = rwFREELISTBLOCKBITMAP(block) + heapSize + freeList->alignment - 1;
                        entries = (RwUInt8 *)((RwUInt32)entries & ~(freeList->alignment - 1));
                        entry = entries + ((i << 3) + bit) * freeList->entrySize;
                        break;
                    }
                    bit++;
                    entriesLeft--;
                }
            } else {
                entriesLeft -= 8;
            }
            if (entry != NULL) {
                break;
            }
        }
        block = block->next;
    }

    /* Otherwise a new block */
    if (entry == NULL) {
        RwUInt8 *entries;
        RwLLLink *newBlock =
            RwMalloc(sizeof(RwLLLink) + heapSize + freeList->entriesPerBlock * freeList->entrySize +
                         freeList->alignment - 1,
                     hint);

        if (newBlock == NULL) {
            return NULL;
        }
        memset(rwFREELISTBLOCKBITMAP(newBlock), 0, heapSize);
        rwLinkListAddLLLink(&freeList->blockList, newBlock);
        *rwFREELISTBLOCKBITMAP(newBlock) = 0x80;
        entries = rwFREELISTBLOCKBITMAP(newBlock) + heapSize + freeList->alignment - 1;
        entries = (RwUInt8 *)((RwUInt32)entries & ~(freeList->alignment - 1));
        entry = entries;
    }
    return entry;
}

/* Whether no entry of a block is in use (inferred helper; fn_8047FF50 contains its loop).
 * Matching workaround: declared inline. As a plain static function it links to the same DOL bytes,
 * because its unused out-of-line copy is dropped at link; inline only keeps that copy out of the
 * per-object comparison. Revisit when that comparison ignores unreferenced local functions. */
static inline RwBool FreeListBlockUnused(RwUInt8 *bitmap, RwUInt32 heapSize) {
    RwUInt32 i;
    RwUInt32 used = 0;

    for (i = 0; i < heapSize; i++) {
        used += bitmap[i];
    }
    return used == 0;
}

RwFreeList *fn_8047FF50(RwFreeList *freeList, void *mem) {
    RwUInt32 heapSize;
    RwUInt8 *blockData;
    RwLLLink *block;
    RwLLLink *end;

    heapSize = freeList->heapSize;
    block = freeList->blockList.link.next;
    end = &freeList->blockList.link;

    while (block != end) {
        /* The entries start here before alignment; the index below is taken from this address */
        blockData = rwFREELISTBLOCKBITMAP(block) + heapSize;

        if ((RwUInt8 *)mem >= blockData &&
            (RwUInt8 *)mem <= blockData + freeList->entriesPerBlock * freeList->entrySize) {
            RwUInt8 *bitmap;
            RwUInt32 index;
            RwUInt32 byteIndex;
            RwUInt8 mask;

            index = ((RwUInt8 *)mem - blockData) / freeList->entrySize;
            byteIndex = index >> 3;
            mask = (RwUInt8)(0x80 >> (index - (byteIndex << 3)));
            bitmap = rwFREELISTBLOCKBITMAP(block);
            bitmap[byteIndex] &= (RwUInt8)~mask;

            if (freeList->flags & rwFREELISTFLAG_FREEBLOCKS) {
                if (FreeListBlockUnused(bitmap, heapSize)) {
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
    RwUInt32 heapSize;
    RwLLLink *block;
    RwLLLink *end;
    RwLLLink *next;

    heapSize = freeList->heapSize;
    block = freeList->blockList.link.next;
    end = &freeList->blockList.link;

    while (block != end) {
        RwUInt8 *blockBitmap;
        RwUInt8 *bitmap;
        RwUInt32 bits;
        RwUInt8 mask;
        RwUInt32 i;

        blockBitmap = rwFREELISTBLOCKBITMAP(block);

        /* The callback may free entries, so a copy of the bitmap is walked */
        bitmap = RwMalloc(heapSize, rwMEMHINTDUR_FUNCTION);
        if (bitmap == NULL) {
            return NULL;
        }
        memcpy(bitmap, blockBitmap, heapSize);
        next = block->next;

        for (i = 0; i < heapSize; i++) {
            bits = bitmap[i];
            if (bits != 0) {
                RwUInt32 bit;

                for (bit = 0; bit < 8; bit++) {
                    mask = (RwUInt8)(0x80 >> bit);
                    if ((bits & mask) != 0) {
                        RwUInt8 *entries;
                        RwUInt8 *entry;

                        entries = rwFREELISTBLOCKBITMAP(block) + heapSize + freeList->alignment - 1;
                        entries = (RwUInt8 *)((RwUInt32)entries & ~(freeList->alignment - 1));
                        entry = entries + ((i << 3) + bit) * freeList->entrySize;
                        callBack(entry, data);
                    }
                }
            }
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
