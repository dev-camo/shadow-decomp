typedef struct Fn8040DB6CEntry {
    /* 0x00 */ char data[44];
} Fn8040DB6CEntry;

Fn8040DB6CEntry *fn_8040DB6C(Fn8040DB6CEntry *base, unsigned int index) {
    return base + index;
}

typedef struct Fn8040DB78Args {
    /* 0x00 */ char data[176];
    /* 0xB0 */ unsigned int value;
} Fn8040DB78Args;

unsigned int fn_8040DB78(Fn8040DB78Args *object) {
    return object->value;
}
