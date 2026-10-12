typedef struct Fn8040D850Object {
    /* 0x00 */ char padding[36];
    /* 0x24 */ float value;
} Fn8040D850Object;

void fn_8040D850(Fn8040D850Object *object, float value) {
    object->value = value;
}
