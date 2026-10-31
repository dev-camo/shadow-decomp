/* Replaceable memory functions (the original file name is unknown; the names are inferred from the
 * code): the game can install its own allocation and release functions, and the library code at
 * 0x80410630-0x8042166C allocates and frees through them. */
#include <stdlib.h>

#include "Game/fn_804110EC_allocator_hooks.h"

/* The uninitialised globals are laid out last-defined first, so they are listed from the highest
 * address down. */
ReleaseHook lbl_805F13A0;
AllocateClearedHook lbl_805F139C;
AllocateHook lbl_805F1398;

void fn_804110EC(AllocateHook allocate, AllocateClearedHook allocateCleared, ReleaseHook release) {
    lbl_805F1398 = allocate;
    lbl_805F139C = allocateCleared;
    lbl_805F13A0 = release;
}

void fn_804110FC(void *block) {
    if (lbl_805F13A0 != 0) {
        lbl_805F13A0(block);
    } else {
        free(block);
    }
}

void *fn_80411134(unsigned long count, unsigned long size) {
    if (lbl_805F139C != 0) {
        return lbl_805F139C(count, size);
    }
    return calloc(count, size);
}
