/* Library support code at 0x804211CC-0x8042166C, the end of the library code at
 * 0x80410630-0x8042166C (the original file name is unknown; every name here is inferred from the
 * code): the byte-order setting, the transfers between ARAM and main memory, and a first-fit
 * allocator for the 6 MB of ARAM that the library reserves. Definitions are in address order, with
 * automatic inlining one level deep (see configure.py). */
#include <dolphin/ar.h>
#include <dolphin/os/OSCache.h>

#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_8042_name_registry.h"
#include "layout_check.h"

/* The size of the ARAM area that fn_80421560 reserves. */
#define ARAM_AREA_SIZE 0x600000

/* A block of the ARAM area. The blocks form a list in the order of their addresses. The first
 * block has size 0 and stands for the start of the area, and the previous pointer of every other
 * block is the block before it. fn_80421560 points the first block's previous at itself and
 * fn_804215C4 sets it to the block before a removed last block. Appending does not update it
 * (original behaviour), so after appends with no removal of a last block, fn_80421500 releases
 * only the first block. */
typedef struct AramBlock {
    /* 0x00 */ struct AramBlock *next;
    /* 0x04 */ struct AramBlock *previous;
    /* 0x08 */ unsigned int address;
    /* 0x0C */ unsigned int size;
} AramBlock;
CHECK_SIZEOF(AramBlock, 0x10);

/* The uninitialised globals are laid out last-defined first, so they are listed from the highest
 * address down. The second word of lbl_805F13E8 and of lbl_805F13F8 is not read or written by any
 * code in the executable; dtk sizes these symbols 8 bytes from the gaps after them (the shape is
 * unknown).
 *
 * Matching workaround: lbl_805F13E8 and lbl_805F13F8 are 8-byte arrays of which only the first
 * word is used. The .sbss holes at 0x805F13EC and 0x805F13FC are inside this unit, and scalars
 * close them (the other globals then move). Revisit when another reader of these objects or a use
 * of the second word is found.
 *
 * lbl_805F13F8[0] is set to 1 by the completion callback fn_804212F4 while the transfer functions
 * wait for it, so it is volatile: the original's wait loops load it again on every pass. */
volatile int lbl_805F13F8[2];
/* The first block of the ARAM area, or null before fn_80421560 and after fn_80421500. */
AramBlock *lbl_805F13F4;
/* The address of the ARAM area. fn_80421500 does not clear it when it releases the area (original
 * behaviour), so fn_80421560 reuses the old address without allocating again. */
unsigned int lbl_805F13F0;
/* [0] is the byte-order setting that fn_804211CC returns and fn_804211D4 sets. */
int lbl_805F13E8[2];

/* Defined below. */
void fn_804212F4(u32 request);
AramBlock *fn_804215C4(AramBlock *block, short deleteFlag);

int fn_804211CC(void) {
    return lbl_805F13E8[0];
}

int fn_804211D4(int byteOrder) {
    int previous = lbl_805F13E8[0];

    lbl_805F13E8[0] = byteOrder;
    return previous;
}

void fn_804211E4(void *destination, unsigned int aramAddress, int size) {
    ARQRequest request;

    DCInvalidateRange(destination, size);
    lbl_805F13F8[0] = 0;
    ARQPostRequest(&request, 0, ARQ_TYPE_ARAM_TO_MRAM, ARQ_PRIORITY_HIGH, aramAddress,
                   (unsigned int)destination, size, fn_804212F4);
    DCInvalidateRange(destination, size);
    while (lbl_805F13F8[0] != 1) {
    }
    DCInvalidateRange(destination, size);
}

void fn_80421270(unsigned int aramAddress, const void *source, int size) {
    ARQRequest request;

    DCFlushRange((void *)source, size);
    lbl_805F13F8[0] = 0;
    ARQPostRequest(&request, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (unsigned int)source,
                   aramAddress, size, fn_804212F4);
    while (lbl_805F13F8[0] != 1) {
    }
    DCInvalidateRange((void *)source, size);
}

void fn_804212F4(u32 request) {
    lbl_805F13F8[0] = 1;
}

void fn_80421300(unsigned int aramAddress) {
    AramBlock *block;

    if (aramAddress == 0) {
        return;
    }
    if (lbl_805F13F4 == 0) {
        return;
    }

    block = lbl_805F13F4;
    while (block->address != aramAddress || block->size == 0) {
        block = block->next;
        if (block == 0) {
            return;
        }
    }
    fn_804215C4(block, 1);
}

/* Allocates the record of a block of size bytes of the area and links it after the block (name
 * inferred). When after is null the new block starts the list at the area's address instead.
 * Returns the new record, or null when it cannot be allocated. */
static inline AramBlock *NewAramBlock(AramBlock *after, unsigned int size) {
    AramBlock *block = fn_80411134(1, sizeof(AramBlock));

    if (block != 0) {
        if (after != 0) {
            block->next = after->next;
            if (block->next != 0) {
                block->next->previous = block;
            }
            after->next = block;
            block->previous = after;
            block->address = after->address + after->size;
            block->size = size;
        } else {
            block->next = 0;
            block->previous = block;
            block->address = lbl_805F13F0;
            block->size = size;
        }
    }
    return block;
}

unsigned int fn_80421368(unsigned int size) {
    unsigned int alignedSize;
    AramBlock *block;

    if (size == 0) {
        return 0;
    }
    if (lbl_805F13F4 == 0) {
        return 0;
    }

    alignedSize = (size + 31) & ~31;
    block = lbl_805F13F4;
    while (block != 0 && block->next != 0) {
        AramBlock *next = block->next;

        if (next->address - (block->address + block->size) >= alignedSize) {
            /* Original behaviour: no null check before reading the address. */
            return NewAramBlock(block, alignedSize)->address;
        }
        block = next;
    }

    /* Original behaviour: the end of the area is tested with the unaligned size. */
    if (block->address + block->size + size <= lbl_805F13F0 + ARAM_AREA_SIZE) {
        /* Original behaviour: no null check before reading the address. */
        return NewAramBlock(block, alignedSize)->address;
    }
    return 0;
}

void fn_80421500(void) {
    if (lbl_805F13F4 != 0) {
        while (lbl_805F13F4 != 0 && lbl_805F13F4->previous != 0) {
            fn_804215C4(lbl_805F13F4->previous, 1);
        }
        fn_804215C4(lbl_805F13F4, 1);
    }
    ARFree(0);
}

void fn_80421560(void) {
    if (lbl_805F13F0 == 0) {
        lbl_805F13F0 = ARAlloc(ARAM_AREA_SIZE);
    }
    lbl_805F13F4 = NewAramBlock(0, 0);
}

AramBlock *fn_804215C4(AramBlock *block, short deleteFlag) {
    if (block != 0) {
        if (lbl_805F13F4 == block) {
            block->previous = 0;
            block->next = 0;
            block->address = 0;
            block->size = 0;
            lbl_805F13F4 = 0;
        } else {
            if (block->next != 0) {
                block->next->previous = block->previous;
            } else {
                lbl_805F13F4->previous = block->previous;
            }
            block->previous->next = block->next;
            block->next = 0;
            block->previous = 0;
            block->address = 0;
            block->size = 0;
        }
        if (deleteFlag > 0) {
            fn_804110FC(block);
        }
    }
    return block;
}
