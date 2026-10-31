/* Getter of an object's float member (the original file name is unknown; the member name is
 * inferred from the code). */
#include "layout_check.h"

typedef struct Fn80424D18Object {
    /* 0x00 */ char unknown00[0x10];
    /* 0x10 */ float value;
} Fn80424D18Object;
CHECK_SIZEOF(Fn80424D18Object, 0x14);

float fn_80424D18(Fn80424D18Object *object) {
    return object->value;
}
