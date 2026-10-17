/* Setters for two words of an object whose type is otherwise unknown. */
typedef struct Fn80407920Object {
    /* 0x00 */ unsigned char padding[0x60];
    /* 0x60 */ unsigned int field_60;
    /* 0x64 */ unsigned int field_64;
#include "layout_check.h"
} Fn80407920Object;
CHECK_SIZEOF(Fn80407920Object, 0x68);

void fn_80407920(Fn80407920Object *object, unsigned int value) {
    object->field_64 = value;
}

void fn_80407928(Fn80407920Object *object, unsigned int value) {
    object->field_60 = value;
}
