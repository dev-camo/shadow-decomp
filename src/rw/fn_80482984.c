/* RenderWare's resources heap: blocks in a fixed buffer, each after a 32-byte header, merged with
 * free neighbours when freed. The original file name is unknown. */
#include "rw/rwcore.h"

/* Blocks and their headers are 32-byte aligned */
#define rwRESHEAPALIGNMENT 32

/* A block's header (0x20 bytes) */
typedef struct RwResHeapBlock {
    /* 0x00 */ struct RwResHeap *heap;
    /* 0x04 */ struct RwResHeapBlock *next;
    /* 0x08 */ struct RwResHeapBlock *prev;
    /* 0x0C */ RwUInt32 size; /* the bytes after the header */
    /* 0x10 */ RwUInt32 used; /* bit 0: the block is allocated */
    /* 0x14 */ RwUInt8 pad[0xC];
} RwResHeapBlock;

/* A heap, at the start of its buffer */
typedef struct RwResHeap {
    /* 0x0 */ RwResHeapBlock *firstBlock;
    /* 0x4 */ RwResHeapBlock *freeBlocks; /* the first block that may be free */
} RwResHeap;

/* Initializes a heap of size bytes at heap: one free block fills the aligned buffer */
RwBool fn_80482984(void *heap, RwUInt32 size) {
    RwResHeap *resHeap = (RwResHeap *)heap;
    RwResHeapBlock *block =
        (RwResHeapBlock *)(((RwUInt32)heap + sizeof(RwResHeap) + rwRESHEAPALIGNMENT - 1) &
                           ~(rwRESHEAPALIGNMENT - 1));
    RwInt32 blockSize = (((RwUInt32)heap + size) & ~(rwRESHEAPALIGNMENT - 1)) - (RwUInt32)block -
                        sizeof(RwResHeapBlock);

    if (blockSize < rwRESHEAPALIGNMENT) {
        return FALSE;
    }
    block->heap = resHeap;
    block->next = NULL;
    block->prev = NULL;
    block->used = 0;
    block->size = blockSize;
    resHeap->firstBlock = block;
    resHeap->freeBlocks = block;
    return TRUE;
}

RwBool fn_804829D4(void *heap) {
    return TRUE;
}

/* Frees a block, merging it with free neighbours */
void fn_804829DC(void *mem) {
    RwResHeapBlock *block = (RwResHeapBlock *)mem - 1;
    RwResHeap *heap;
    RwResHeapBlock *prev;
    RwResHeapBlock *next;

    block->used = 0;
    heap = block->heap;
    prev = block->prev;
    next = block->next;
    if (heap->freeBlocks == NULL || block < heap->freeBlocks) {
        heap->freeBlocks = block;
    }

    if (prev != NULL && (~prev->used & 1)) {
        prev->next = next;
        if (next != NULL) {
            next->prev = prev;
        }
        prev->size += block->size + sizeof(RwResHeapBlock);
        block = prev;
    }
    if (next != NULL && (~next->used & 1)) {
        block->next = next->next;
        if (next->next != NULL) {
            next->next->prev = block;
        }
        block->size += next->size + sizeof(RwResHeapBlock);
    }
}

/* Allocates the first free block of at least size bytes, rounded up to the alignment, splitting
 * off what is left */
void *fn_80482A94(void *heap, RwUInt32 size) {
    RwResHeap *resHeap = (RwResHeap *)heap;
    RwResHeapBlock *cur = resHeap->freeBlocks;
    RwResHeapBlock *block = NULL;

    size = (size + rwRESHEAPALIGNMENT - 1) & ~(rwRESHEAPALIGNMENT - 1);
    while (cur != NULL && block == NULL) {
        if ((~cur->used & 1) && cur->size >= size) {
            block = cur;
        }
        cur = cur->next;
    }
    if (block == NULL) {
        return NULL;
    }

    /* Splits the block when what is left can hold a header and a block */
    if (block->size > size + 2 * sizeof(RwResHeapBlock)) {
        RwResHeapBlock *next = block->next;
        RwResHeapBlock *rest = (RwResHeapBlock *)((RwUInt8 *)block + size + sizeof(RwResHeapBlock));

        if (next != NULL && (~next->used & 1)) {
            /* The rest takes in the free block after it */
            rest->next = next->next;
            rest->size = block->next->size + (block->size - size);
        } else {
            rest->next = next;
            rest->size = block->size - size - sizeof(RwResHeapBlock);
        }
        block->next = rest;
        rest->used = 0;
        rest->prev = block;
        if (rest->next != NULL) {
            rest->next->prev = rest;
        }
        block->size = size;
        rest->heap = block->heap;
    }

    if (block == resHeap->freeBlocks) {
        do {
            resHeap->freeBlocks = resHeap->freeBlocks->next;
        } while (resHeap->freeBlocks != NULL && (resHeap->freeBlocks->used & 1));
    }
    block->used = 1;
    return block + 1;
}
