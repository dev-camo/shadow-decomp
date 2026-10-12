#include "Game/fn_8040_stream_object.h"

void fn_8040307C(Fn8040StreamObject *object, Sint32 min_val) {
    LSC lsc = object->lsc;

    if (lsc != 0) {
        LSC_SetFlowLimit(lsc, min_val);
    }
}
