#ifndef GAME_UNKSINGLETON_8000DCC0_H
#define GAME_UNKSINGLETON_8000DCC0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The record that the object returned by fn_8000DCC0 points to (placeholder name; only two floats
 * at 0x68 and 0x6C are mapped, which RainEffect reads; the size is not known). */
struct UnkRecord_8000DCC0 {
    /* 0x00 */ u8 field_0x0[0x68];
    /* 0x68 */ float field_0x68;
    /* 0x6C */ float field_0x6C;
};
CHECK_OFFSETOF(UnkRecord_8000DCC0, field_0x6C, 0x6C);

/* The object that fn_8000DCC0 returns: a static object (at 0x8056EE5C in the USA release) that the
 * function constructs and registers for destruction on first use (behaviour inferred, as for
 * fn_80009548). Its first member points to the record above. The original class is unknown. */
class UnkSingleton_8000DCC0 {
  public:
    /* 0x00 */ UnkRecord_8000DCC0 *field_0x0;
};

extern "C" {
UnkSingleton_8000DCC0 *fn_8000DCC0(void);
}

#endif
