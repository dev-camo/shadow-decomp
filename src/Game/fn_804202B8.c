/* Negates the components of an object that its flags select (the original file name is unknown;
 * the member names are inferred from the code). */
#include "layout_check.h"

typedef struct Fn804202B8FlagData {
    /* 0x00 */ char unknown00[0xA];
    /* 0x0A */ unsigned short flags;
} Fn804202B8FlagData;
CHECK_SIZEOF(Fn804202B8FlagData, 0xC);

typedef struct Fn804202B8Object {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
    /* 0x0C */ unsigned int unknown0C[2];
    /* 0x14 */ Fn804202B8FlagData *flag_data;
} Fn804202B8Object;
CHECK_SIZEOF(Fn804202B8Object, 0x18);

void fn_804202B8(Fn804202B8Object *object) {
    Fn804202B8FlagData *flag_data = object->flag_data;

    if (flag_data->flags & 1) {
        object->x = -object->x;
    }
    if (flag_data->flags & 2) {
        object->y = -object->y;
    }
    if (flag_data->flags & 4) {
        object->z = -object->z;
    }
}
