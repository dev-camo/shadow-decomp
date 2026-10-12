typedef struct Fn8040D608Object {
    /* 0x00 */ unsigned char state;
    /* 0x01 */ unsigned char pad_01;
    /* 0x02 */ unsigned short flags;
    /* 0x04 */ unsigned short limit;
    /* 0x06 */ unsigned short pad_06;
    /* 0x08 */ unsigned int field_08;
    /* 0x0C */ unsigned int field_0C;
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
} Fn8040D608Object;

void fn_8040D608(Fn8040D608Object *object) {
    object->state = 0;
    object->flags = 0;
    object->limit = 0xFFFF;
    object->field_08 = 0;
    object->field_0C = 4;
    object->field_10 = 30;
    object->field_14 = 0;
    object->field_18 = 0;
}
