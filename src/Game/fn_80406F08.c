typedef struct Fn80406F08Object {
    /* 0x000 */ unsigned char padding[0x1C8];
    /* 0x1C8 */ int field_1C8;
} Fn80406F08Object;

extern void fn_8040773C(Fn80406F08Object *object, int value, int value_18);

void fn_80406F08(Fn80406F08Object *object) {
    int value = object->field_1C8;
    fn_8040773C(object, (int)(value * 0.8), value);
}
