#include "layout_check.h"

/* A 44-byte array element; its contents are unknown. */
typedef struct Fn8040DB6CEntry {
    /* 0x00 */ char data[44];
} Fn8040DB6CEntry;
CHECK_SIZEOF(Fn8040DB6CEntry, 0x2C);

/* Returns &base[index]. */
Fn8040DB6CEntry *fn_8040DB6C(Fn8040DB6CEntry *base, unsigned int index) {
    return base + index;
}

/* An object whose word at 0xB0 is read; the rest is unknown. */
typedef struct Fn8040DB78Object {
    /* 0x00 */ char field_0x0[0xB0];
    /* 0xB0 */ unsigned int value;
} Fn8040DB78Object;
CHECK_SIZEOF(Fn8040DB78Object, 0xB4);

unsigned int fn_8040DB78(Fn8040DB78Object *object) {
    return object->value;
}
