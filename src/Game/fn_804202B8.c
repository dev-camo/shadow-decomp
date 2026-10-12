typedef struct Fn804202B8FlagData {
    /* 0x00 */ char padding[10];
    /* 0x0A */ unsigned short flags;
} Fn804202B8FlagData;

typedef struct Fn804202B8Object {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
    /* 0x0C */ unsigned int unknown[2];
    /* 0x14 */ Fn804202B8FlagData *flag_data;
} Fn804202B8Object;

void fn_804202B8(Fn804202B8Object *object) {
    Fn804202B8FlagData *flag_data = object->flag_data;

    if (flag_data->flags & 1) {
        object->x = -object->x;
    }
    if (flag_data->flags & 2) {
        object->y = -object->y;
    }
    if (flag_data->flags & 4) {
        object->z = -object->z;
    }
}
