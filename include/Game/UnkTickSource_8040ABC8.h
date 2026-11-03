#ifndef GAME_UNK_TICK_SOURCE_8040ABC8_H
#define GAME_UNK_TICK_SOURCE_8040ABC8_H

#include "Game/UnkTick.h"
#include "layout_check.h"

/* An object without data that the game allocates into a global pointer and deletes again, and that
 * reads the current tick (placeholder class: its name is unknown). */
class UnkTickSource_8040ABC8 {
  public:
    UnkTickSource_8040ABC8();
    ~UnkTickSource_8040ABC8();

    /* Returns the current value of the processor tick counter (name inferred). */
    UnkTick GetTick();
};
CHECK_SIZEOF(UnkTickSource_8040ABC8, 0x1);

extern UnkTickSource_8040ABC8 *lbl_TickSource_805EE1E4;

#endif
