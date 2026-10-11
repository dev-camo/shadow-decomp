#include "CRI/mwstm.h"

typedef struct Fn8040773CObject {
    /* 0x00 */ unsigned char padding[0x40];
    /* 0x40 */ void *field_40;
} Fn8040773CObject;

extern void fn_8040307C(Fn8040773CObject *object, int value);

void fn_8040773C(Fn8040773CObject *object, int value, int value_18) {
    MWSTM_SetFlowLimit(object->field_40, value, value_18);
    fn_8040307C(object, value);
}
