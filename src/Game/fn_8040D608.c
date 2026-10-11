typedef struct Fn8040D608Args {
    /* 0x00 */ unsigned char state;
    /* 0x01 */ unsigned char padding1;
    /* 0x02 */ unsigned short flags;
    /* 0x04 */ unsigned short limit;
    /* 0x06 */ unsigned short padding6;
    /* 0x08 */ unsigned int field8;
    /* 0x0C */ unsigned int field12;
    /* 0x10 */ unsigned int field16;
    /* 0x14 */ unsigned int field20;
    /* 0x18 */ unsigned int field24;
} Fn8040D608Args;

void fn_8040D608(Fn8040D608Args *object) {
    object->state = 0;
    object->flags = 0;
    object->limit = 0xFFFF;
    object->field8 = 0;
    object->field12 = 4;
    object->field16 = 30;
    object->field20 = 0;
    object->field24 = 0;
}
