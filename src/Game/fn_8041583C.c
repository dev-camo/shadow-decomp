typedef struct Fn8041583CObject {
    /* 0x00 */ char padding[32];
    /* 0x20 */ void *value;
#include "layout_check.h"
} Fn8041583CObject;
CHECK_SIZEOF(Fn8041583CObject, 0x24);

void *fn_8041583C(Fn8041583CObject *object) {
    if (object != 0) {
        return object->value;
    }
    return 0;
}
