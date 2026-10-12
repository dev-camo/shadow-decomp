#include "Game/fn_8040_stream_object.h"

void fn_80406F08(Fn8040StreamObject *object) {
    int max_nsct = object->field_1C8;
    fn_8040773C(object, (int)(max_nsct * 0.8), max_nsct);
}
