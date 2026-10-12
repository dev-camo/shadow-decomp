#include "CRI/mwstm.h"
#include "Game/fn_8040_stream_object.h"

void fn_8040773C(Fn8040StreamObject *object, int value, int value_18) {
    MWSTM_SetFlowLimit(object->field_40, value, value_18);
    fn_8040307C(object, value);
}
