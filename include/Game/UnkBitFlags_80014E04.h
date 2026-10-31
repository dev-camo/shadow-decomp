#ifndef GAME_UNKBITFLAGS_80014E04_H
#define GAME_UNKBITFLAGS_80014E04_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A word of flag bits addressed by bit number (placeholder name: the original class is unknown).
 * The game calls every member out of line; the copies the linker kept are the constructor at
 * 0x80014E04, Clear at 0x80014E10, Set at 0x80014E1C, Test at 0x80014E34 and Reset at 0x8006127C.
 * Method names are inferred from what those copies do. */
class UnkBitFlags_80014E04 {
  public:
    /* Clears every bit. */
    UnkBitFlags_80014E04();
    void Clear();
    void Set(int bit);
    bool Test(int bit) const;
    void Reset(int bit);

    /* 0x0 */ u32 m_bits;
};
CHECK_SIZEOF(UnkBitFlags_80014E04, 0x4);

#endif
