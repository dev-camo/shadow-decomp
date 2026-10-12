typedef struct Fn80424D18Object {
    /* 0x00 */ char padding[16];
    /* 0x10 */ float value;
} Fn80424D18Object;

float fn_80424D18(Fn80424D18Object *object) {
    return object->value;
}
