typedef struct Fn8040DB6CEntry {
    /* 0x00 */ char data[44];
#include "layout_check.h"
} Fn8040DB6CEntry;
CHECK_SIZEOF(Fn8040DB6CEntry, 0x2C);

Fn8040DB6CEntry *fn_8040DB6C(Fn8040DB6CEntry *base, unsigned int index) {
    return base + index;
}

typedef struct Fn8040DB78Object {
    /* 0x00 */ char data[176];
    /* 0xB0 */ unsigned int value;
} Fn8040DB78Object;
CHECK_SIZEOF(Fn8040DB78Object, 0xB4);

unsigned int fn_8040DB78(Fn8040DB78Object *object) {
    return object->value;
}
