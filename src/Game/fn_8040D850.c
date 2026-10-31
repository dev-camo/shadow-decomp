#include "layout_check.h"

/* An object whose float at 0x24 is written; the first 0x24 bytes are unknown. */
typedef struct Fn8040D850Object {
    /* 0x00 */ char field_0x0[0x24];
    /* 0x24 */ float value;
} Fn8040D850Object;
CHECK_SIZEOF(Fn8040D850Object, 0x28);

void fn_8040D850(Fn8040D850Object *object, float value) {
    object->value = value;
}
