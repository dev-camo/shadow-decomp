#ifndef GAME_UNKOBJECT_80051734_H
#define GAME_UNKOBJECT_80051734_H

#include <dolphin/types.h>
#include "layout_check.h"

/* Borrowed view of fn_80051734's singleton (placeholder name). Its constructor at 0x8034A3AC
 * sets these two words to 0 and 1, and fn_800A871C indexes them. The unknown prefix includes
 * constructed members; this view does not recover their types or the native lifetime. */
class UnkObject_80051734 {
  public:
    /* 0x000 */ u8 field_0x0[0x11C];
    /* 0x11C */ u32 field_0x11C[2];

  private:
    UnkObject_80051734();
    UnkObject_80051734(const UnkObject_80051734 &);
    UnkObject_80051734 &operator=(const UnkObject_80051734 &);
    ~UnkObject_80051734();
};
/* Checks the recovered prefix, rather than establishing the complete native extent. */
CHECK_SIZEOF(UnkObject_80051734, 0x124);

/* Returns the cached singleton, constructing it on first use (signature inferred). */
extern "C" UnkObject_80051734 *fn_80051734();

#endif
