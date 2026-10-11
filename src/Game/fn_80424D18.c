typedef struct Fn80424D18Args {
    /* 0x00 */ char padding[16];
    /* 0x10 */ float value;
} Fn80424D18Args;

float fn_80424D18(Fn80424D18Args *object) {
    return object->value;
}
