#include "layout_check.h"

/* Takes the pointer out of *slot, leaving NULL behind (behaviour inferred). */
void *fn_8040D6F4(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}

/* A 56-byte array element; its contents are unknown. */
typedef struct Fn8040D708Entry {
    /* 0x00 */ char data[56];
} Fn8040D708Entry;
CHECK_SIZEOF(Fn8040D708Entry, 0x38);

/* Returns &base[index]. */
Fn8040D708Entry *fn_8040D708(Fn8040D708Entry *base, unsigned int index) {
    return base + index;
}
