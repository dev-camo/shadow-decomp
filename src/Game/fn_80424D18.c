typedef struct Fn80424D18Object {
    /* 0x00 */ char padding[16];
    /* 0x10 */ float value;
#include "layout_check.h"
} Fn80424D18Object;
CHECK_SIZEOF(Fn80424D18Object, 0x14);

float fn_80424D18(Fn80424D18Object *object) {
    return object->value;
}
