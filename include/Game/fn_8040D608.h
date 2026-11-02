#ifndef GAME_FN_8040D608_H
#define GAME_FN_8040D608_H

#include "layout_check.h"

struct Fn8040D594Object;

/* Provisional layout of the manager's 0x30-stride elements. The observed access
 * widths and offsets do not fully recover the original member types, signedness,
 * alignment or lifetime. No original copying or lifetime behaviour is implied. */
typedef struct Fn8040D608Object {
    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x02 */ unsigned short field_0x2;
    /* 0x04 */ unsigned short field_0x4;
    /* 0x08 */ unsigned int field_0x8;
    /* 0x0C */ unsigned int field_0xC;
    /* 0x10 */ unsigned int field_0x10;
    /* 0x14 */ unsigned int field_0x14;
    /* 0x18 */ unsigned int field_0x18;
    /* Unrecovered storage; the original member composition is unknown. */
    /* 0x1C */ unsigned char field_0x1C[0x10];
    /* 0x2C */ struct Fn8040D594Object *field_0x2C; /* Containing manager. */
} Fn8040D608Object;
CHECK_SIZEOF(Fn8040D608Object, 0x30);

#ifdef __cplusplus
extern "C" {
#endif

// fn_8040D498.c
void *fn_8040D498(void **slot);
Fn8040D608Object *fn_8040D4AC(Fn8040D608Object *base, unsigned int index);

// fn_8040D608.c
void fn_8040D608(Fn8040D608Object *object);

#ifdef __cplusplus
}
#endif

#endif
