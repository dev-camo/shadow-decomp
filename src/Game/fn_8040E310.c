typedef struct Fn8040E310Object {
    /* 0x00 */ unsigned short state;
    /* 0x02 */ unsigned short padding;
    /* 0x04 */ unsigned int value;
    /* 0x08 */ unsigned int mode;
} Fn8040E310Object;

/* Both setters clear the object first; the cleared fields are then overwritten (inferred helper). */
static inline void fn_8040E310_clear(Fn8040E310Object *object) {
    object->value = 0;
    object->state = 0;
    object->mode = 0;
}

void fn_8040E310(Fn8040E310Object *object, unsigned int value) {
    fn_8040E310_clear(object);
    object->value = value;
    object->mode = 2;
}

void fn_8040E330(Fn8040E310Object *object, unsigned short state) {
    fn_8040E310_clear(object);
    object->state = state;
    object->mode = 1;
}
