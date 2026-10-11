typedef struct Fn8040E310Args {
    /* 0x00 */ unsigned short state;
    /* 0x02 */ unsigned short padding;
    /* 0x04 */ unsigned int value;
    /* 0x08 */ unsigned int mode;
} Fn8040E310Args;

void fn_8040E310(Fn8040E310Args *object, unsigned int value) {
    object->value = 0;
    object->state = 0;
    object->mode = 0;
    object->value = value;
    object->mode = 2;
}

void fn_8040E330(Fn8040E310Args *object, unsigned short state) {
    object->value = 0;
    object->state = 0;
    object->mode = 0;
    object->state = state;
    object->mode = 1;
}
