typedef struct Fn8040E38CArgs {
    /* 0x00 */ short first;
    /* 0x02 */ short padding;
    /* 0x04 */ int second;
    /* 0x08 */ int third;
} Fn8040E38CArgs;

void fn_8040E38C(Fn8040E38CArgs *object) {
    object->second = 0;
    object->first = 0;
    object->third = 0;
}
