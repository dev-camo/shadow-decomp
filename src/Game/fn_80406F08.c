#include "Game/fn_8040_stream_object.h"

void fn_80406F08(Fn8040StreamObject *object) {
    int value = object->field_1C8;
    fn_8040773C(object, (int)(value * 0.8), value);
}
