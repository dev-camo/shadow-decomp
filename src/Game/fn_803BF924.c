typedef struct Fn803BF924Object {
    /* 0x00 */ unsigned char padding[0x18];
    /* 0x18 */ int field_18;
    /* 0x1C */ int field_1C;
} Fn803BF924Object;

int fn_803BF924(Fn803BF924Object *object, int value_1C, int value_18) {
    object->field_1C = value_1C;
    object->field_18 = value_18;
    return 1;
}
