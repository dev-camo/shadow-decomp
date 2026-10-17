void *fn_8040D498(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}

typedef struct Fn8040D4ACEntry {
    /* 0x00 */ char data[48];
#include "layout_check.h"
} Fn8040D4ACEntry;
CHECK_SIZEOF(Fn8040D4ACEntry, 0x30);

Fn8040D4ACEntry *fn_8040D4AC(Fn8040D4ACEntry *base, unsigned int index) {
    return base + index;
}
