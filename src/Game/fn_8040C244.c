typedef struct Fn8040C244Object {
    /* 0x00 */ char padding[8];
    /* 0x08 */ int count;
#include "layout_check.h"
} Fn8040C244Object;
CHECK_SIZEOF(Fn8040C244Object, 0xC);

void fn_8040C244(Fn8040C244Object *object) {
    if (object->count > 0) {
        object->count--;
    }
}
