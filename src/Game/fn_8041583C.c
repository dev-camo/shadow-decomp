typedef struct Fn8041583CObject {
    /* 0x00 */ char padding[32];
    /* 0x20 */ void *value;
} Fn8041583CObject;

void *fn_8041583C(Fn8041583CObject *object) {
    if (object != 0) {
        return object->value;
    }
    return 0;
}
