/* RenderWare pipeline library: the pipeline heap, a scratch heap for packet data that is reset
 * before each pipeline execution. Its memory comes in superblocks; each superblock holds a chain
 * of blocks, every block starting with a header, and the free blocks are listed in a table that
 * grows as needed. The original file name is unknown. */
#include "rw/rwcore.h"

/* The free block table grows by this many entries */
#define rxHEAPFREEBLOCKGROWTH 32

/* The smallest heap, and the smallest superblock */
#define rxHEAPMINSIZE 0x400
#define rxHEAPMINSUPERBLOCKSIZE 0x80

/* Block data is aligned to this, and superblocks to rxHEAPSUPERBLOCKALIGNMENT */
#define rxHEAPALIGNMENT 32
#define rxHEAPSUPERBLOCKALIGNMENT 128

/* Takes a new entry from the heap's free block table, growing the table when it is full; NULL if
 * it cannot grow (name inferred) */
static inline rxHeapFreeBlock *HeapFreeBlockAlloc(RxHeap *heap) {
    rxHeapFreeBlock *freeBlocks = heap->freeBlocks;
    RwUInt32 entriesUsed = heap->entriesUsed;

    if (heap->entriesAlloced <= entriesUsed) {
        RwUInt32 entriesAlloced = heap->entriesAlloced + rxHEAPFREEBLOCKGROWTH;

        heap->entriesAlloced = entriesAlloced;
        freeBlocks = RwRealloc(freeBlocks, entriesAlloced * sizeof(rxHeapFreeBlock),
                               rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_PIPEMODULE);
        if (freeBlocks == NULL) {
            RWERRORCORE((E_RW_NOMEM, heap->entriesAlloced * sizeof(rxHeapFreeBlock)));
            heap->entriesAlloced -= rxHEAPFREEBLOCKGROWTH;
        } else {
            /* the table moved: point each free block at its entry again */
            if (freeBlocks != heap->freeBlocks) {
                rxHeapFreeBlock *entry;

                if (entriesUsed != 0) {
                    entry = freeBlocks;
                    do {
                        entry->ptr->freeEntry = entry;
                        entry++;
                    } while (--entriesUsed != 0);
                }
            }
            heap->freeBlocks = freeBlocks;
        }
    }

    if (freeBlocks != NULL) {
        freeBlocks += heap->entriesUsed++;
    }
    return freeBlocks;
}

/* Makes a superblock one free block between an empty header at its start and one at its end,
 * and chains it after the previous superblock's end header (name inferred) */
static inline RwBool HeapSuperBlockReset(RxHeap *heap, rxHeapSuperBlockDescriptor *superBlock,
                                         rxHeapSuperBlockDescriptor *prevSuperBlock) {
    rxHeapFreeBlock *freeEntry = HeapFreeBlockAlloc(heap);

    if (freeEntry != NULL) {
        rxHeapBlockHeader *first = superBlock->start;
        rxHeapBlockHeader *block = first + 1;
        rxHeapBlockHeader *last =
            (rxHeapBlockHeader *)((RwUInt8 *)first + superBlock->size - sizeof(rxHeapBlockHeader));

        first->prev = NULL;
        first->next = NULL;
        first->size = 0;
        first->freeEntry = NULL;
        *last = *first;

        first->next = block;
        block->prev = first;
        block->next = last;
        last->prev = block;
        block->size = (RwUInt8 *)last - (RwUInt8 *)(block + 1);
        block->freeEntry = freeEntry;
        freeEntry->ptr = block;
        freeEntry->size = block->size;

        if (prevSuperBlock != NULL) {
            rxHeapBlockHeader *prevLast =
                (rxHeapBlockHeader *)((RwUInt8 *)prevSuperBlock->start + prevSuperBlock->size -
                                      sizeof(rxHeapBlockHeader));

            prevLast->next = first;
            first->prev = prevLast;
        }
        return TRUE;
    }
    return FALSE;
}

/* Frees a block, merging it with free neighbours */
void fn_8049EA2C(RxHeap *heap, void *ptr) {
    rxHeapBlockHeader *block = (rxHeapBlockHeader *)ptr - 1;
    RwBool prevFree = block->prev != NULL && block->prev->freeEntry != NULL;
    RwBool nextFree = block->next != NULL && block->next->freeEntry != NULL;

    if (prevFree) {
        if (nextFree) {
            /* the three become the previous block: drop the next block's table entry by moving
             * the last entry into its place */
            rxHeapFreeBlock *lastEntry = &heap->freeBlocks[heap->entriesUsed - 1];
            rxHeapFreeBlock *nextEntry = block->next->freeEntry;

            if (lastEntry != nextEntry) {
                *nextEntry = heap->freeBlocks[heap->entriesUsed - 1];
                nextEntry->ptr->freeEntry = nextEntry;
            }
            heap->entriesUsed--;

            block->prev->size += block->size + block->next->size + 2 * sizeof(rxHeapBlockHeader);
            block->prev->freeEntry->size = block->prev->size;
            block->prev->next = block->next->next;
            if (block->next->next != NULL) {
                block->next->next->prev = block->prev;
            }
        } else {
            block->prev->size += block->size + sizeof(rxHeapBlockHeader);
            block->prev->freeEntry->size = block->prev->size;
            block->prev->next = block->next;
            if (block->next != NULL) {
                block->next->prev = block->prev;
            }
        }
    } else if (nextFree) {
        block->size += block->next->size + sizeof(rxHeapBlockHeader);
        block->freeEntry = block->next->freeEntry;
        block->next->freeEntry->ptr = block;
        block->next->freeEntry->size = block->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    } else {
        rxHeapFreeBlock *freeEntry = HeapFreeBlockAlloc(heap);

        if (freeEntry != NULL) {
            freeEntry->ptr = block;
            freeEntry->size = block->size;
            block->freeEntry = freeEntry;
        }
    }
}

/* Frees every block of a heap: each superblock becomes one free block again, the ones after the
 * first in their list order and the first last */
RwBool fn_8049ECFC(RxHeap *heap) {
    rxHeapSuperBlockDescriptor *prevSuperBlock = NULL;
    rxHeapSuperBlockDescriptor *superBlock;

    heap->entriesUsed = 0;
    for (superBlock = heap->head->next; superBlock != NULL; superBlock = superBlock->next) {
        if (!HeapSuperBlockReset(heap, superBlock, prevSuperBlock)) {
            return FALSE;
        }
        if (prevSuperBlock == NULL) {
            heap->headBlock = superBlock->start;
        }
        prevSuperBlock = superBlock;
    }

    superBlock = heap->head;
    if (!HeapSuperBlockReset(heap, superBlock, prevSuperBlock)) {
        return FALSE;
    }
    if (prevSuperBlock == NULL) {
        heap->headBlock = superBlock->start;
    }
    heap->dirty = FALSE;
    return TRUE;
}

/* Frees a superblock, whose descriptor is at the start of its memory (name inferred) */
static inline void HeapSuperBlockDestroy(rxHeapSuperBlockDescriptor *superBlock) {
    if (superBlock != NULL) {
        RwFree(superBlock);
    }
}

/* Destroys a heap and frees its memory */
void fn_8049F0CC(RxHeap *heap) {
    if (heap != NULL) {
        rxHeapSuperBlockDescriptor *superBlock;

        if (heap->freeBlocks != NULL) {
            RwFree(heap->freeBlocks);
            heap->freeBlocks = NULL;
        }
        superBlock = heap->head;
        while (superBlock != NULL) {
            rxHeapSuperBlockDescriptor *next = superBlock->next;

            HeapSuperBlockDestroy(superBlock);
            superBlock = next;
        }
        RwFree(heap);
    }
}

/* Allocates a superblock of size bytes; its descriptor sits before its aligned start (name
 * inferred) */
static inline rxHeapSuperBlockDescriptor *HeapSuperBlockCreate(RwUInt32 size) {
    rxHeapSuperBlockDescriptor *superBlock =
        RwMalloc(size + sizeof(rxHeapSuperBlockDescriptor) + rxHEAPSUPERBLOCKALIGNMENT - 1,
                 rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL | rwID_PIPEMODULE);

    if (superBlock != NULL) {
        superBlock->start = (void *)(((RwUInt32)superBlock + sizeof(rxHeapSuperBlockDescriptor) +
                                      rxHEAPSUPERBLOCKALIGNMENT - 1) &
                                     ~(rxHEAPSUPERBLOCKALIGNMENT - 1));
        superBlock->size = size;
        superBlock->next = NULL;
    }
    return superBlock;
}

/* Creates a heap of at least size bytes; NULL on failure */
RxHeap *fn_8049F168(RwUInt32 size) {
    RxHeap *heap;

    if (size < rxHEAPMINSIZE) {
        size = rxHEAPMINSIZE;
    }
    heap = RwMalloc(sizeof(RxHeap), rwMEMHINTDUR_GLOBAL | rwID_PIPEMODULE);
    if (heap != NULL) {
        rxHeapSuperBlockDescriptor *superBlock;

        size = (size + rxHEAPALIGNMENT - 1) & ~(rxHEAPALIGNMENT - 1);
        if (size < rxHEAPMINSUPERBLOCKSIZE) {
            size = rxHEAPMINSUPERBLOCKSIZE;
        }
        superBlock = HeapSuperBlockCreate(size);
        if (superBlock != NULL) {
            heap->superBlockSize = size;
            heap->head = superBlock;
            heap->freeBlocks = NULL;
            heap->entriesAlloced = 0;
            heap->entriesUsed = 0;
            heap->dirty = TRUE;
            if (RxHeapReset(heap)) {
                return heap;
            }
            HeapSuperBlockDestroy(superBlock);
        }
        RwFree(heap);
    }
    return NULL;
}
