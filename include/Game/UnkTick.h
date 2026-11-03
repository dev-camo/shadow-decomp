#ifndef GAME_UNK_TICK_H
#define GAME_UNK_TICK_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A 64-bit tick count (placeholder type: only its size and use are known). */
struct UnkTick {
    UnkTick() : ticks(0) {}
    UnkTick(u32 count) : ticks(count) {}
    UnkTick(u64 count);

    /* 0x00 */ u64 ticks;
};
CHECK_SIZEOF(UnkTick, 0x8);

#endif
