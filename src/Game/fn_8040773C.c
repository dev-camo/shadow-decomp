#include "CRI/mwstm.h"
#include "Game/fn_8040_stream_object.h"

/* Parameter names from MWSTM_SetFlowLimit, which receives them unchanged. */
void fn_8040773C(Fn8040StreamObject *object, int min_nsct, int max_nsct) {
    MWSTM_SetFlowLimit(object->field_40, min_nsct, max_nsct);
    fn_8040307C(object, min_nsct);
}
