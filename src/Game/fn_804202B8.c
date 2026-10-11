typedef struct Fn804202B8FlagData {
    /* 0x00 */ char padding[10];
    /* 0x0A */ volatile unsigned short flags;
} Fn804202B8FlagData;

typedef struct Fn804202B8Vector {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
    /* 0x0C */ unsigned int unknown[2];
    /* 0x14 */ Fn804202B8FlagData *flag_data;
} Fn804202B8Vector;

void fn_804202B8(Fn804202B8Vector *vector) {
    Fn804202B8FlagData *flag_data = vector->flag_data;

    /* Keep these volatile reads at each test; the target reloads the flags every time. */
    if (flag_data->flags & 1) {
        vector->x = -vector->x;
    }
    if (flag_data->flags & 2) {
        vector->y = -vector->y;
    }
    if (flag_data->flags & 4) {
        vector->z = -vector->z;
    }
}
