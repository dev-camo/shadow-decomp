void *fn_8040D6F4(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}

typedef struct Fn8040D708Entry {
    /* 0x00 */ char data[56];
#include "layout_check.h"
} Fn8040D708Entry;
CHECK_SIZEOF(Fn8040D708Entry, 0x38);

Fn8040D708Entry *fn_8040D708(Fn8040D708Entry *base, unsigned int index) {
    return base + index;
}
