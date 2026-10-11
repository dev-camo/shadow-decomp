typedef struct Fn8040C244Args {
    /* 0x00 */ char padding[8];
    /* 0x08 */ int count;
} Fn8040C244Args;

void fn_8040C244(Fn8040C244Args *object) {
    if (object->count > 0) {
        object->count--;
    }
}
