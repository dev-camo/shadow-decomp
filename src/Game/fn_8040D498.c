#include "layout_check.h"

/* Takes the pointer out of *slot, leaving NULL behind (behaviour inferred). */
void *fn_8040D498(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}

/* A 48-byte array element; its contents are unknown. */
typedef struct Fn8040D4ACEntry {
    /* 0x00 */ char data[48];
} Fn8040D4ACEntry;
CHECK_SIZEOF(Fn8040D4ACEntry, 0x30);

/* Returns &base[index]. */
Fn8040D4ACEntry *fn_8040D4AC(Fn8040D4ACEntry *base, unsigned int index) {
    return base + index;
}
