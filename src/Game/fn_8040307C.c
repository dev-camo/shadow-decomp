#include "CRI/lsc.h"

typedef struct Fn8040307CObject {
    /* 0x00 */ unsigned char padding[0x48];
    /* 0x48 */ LSC lsc;
} Fn8040307CObject;

void fn_8040307C(Fn8040307CObject *object, Sint32 min_val) {
    LSC lsc = object->lsc;

    if (lsc != 0) {
        LSC_SetFlowLimit(lsc, min_val);
    }
}
