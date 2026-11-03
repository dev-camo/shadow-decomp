/* Stores a halfword value or word mask and sets the record's mode (inferred purpose). */

#include "Game/fn_8040E310.h"

/* Both setters clear the object first; the cleared fields are then overwritten (inferred
 * helper). */
static inline void fn_8040E310_clear(Fn8040E310Object *object) {
    object->field_0x4 = 0;
    object->field_0x0 = 0;
    object->mode = 0;
}

void fn_8040E310(Fn8040E310Object *object, unsigned int value) {
    fn_8040E310_clear(object);
    object->field_0x4 = value;
    object->mode = 2;
}

void fn_8040E330(Fn8040E310Object *object, unsigned short value) {
    fn_8040E310_clear(object);
    object->field_0x0 = value;
    object->mode = 1;
}
