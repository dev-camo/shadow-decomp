#ifndef GAME_FN_8040E310_H
#define GAME_FN_8040E310_H

#include "layout_check.h"

/* An element of the array constructed by fn_8040E274. Mode 1 selects the halfword;
 * mode 2 selects the word mask. The original type name is unknown. */
typedef struct Fn8040E310Object {
    /* 0x00 */ unsigned short field_0x0;
    /* 0x04 */ unsigned int field_0x4;
    /* 0x08 */ unsigned int mode;
} Fn8040E310Object;
CHECK_SIZEOF(Fn8040E310Object, 0xC);

#ifdef __cplusplus
extern "C" {
#endif

void fn_8040E310(Fn8040E310Object *object, unsigned int value);
void fn_8040E330(Fn8040E310Object *object, unsigned short value);
void fn_8040E38C(Fn8040E310Object *object);

#ifdef __cplusplus
}
#endif

#endif
