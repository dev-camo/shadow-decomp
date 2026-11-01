#ifndef GAME_UNKTASKGROUP_801D0A78_H
#define GAME_UNKTASKGROUP_801D0A78_H

#include "Game/Task.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* A group of three words, the first two of which are tasks (placeholder name). The layout comes
 * from the constructor at 0x801D0A78 and the destructor at 0x801D09F8. The constructor clears the
 * three words, then allocates a 0x24-byte object, constructs it with fn_801D048C and passes it to
 * fn_8000D1E0 together with the manager that fn_80009548 returns. It never writes the two task
 * pointers, and the code that stores them is not identified. The destructor sets bit 0 of the
 * flags (0x4) of the tasks at 0x0 and 0x4 and clears the pointers. RainEffect's Create takes its
 * parent task from field_0x4. The constructor and destructor are out of line. */
class UnkTaskGroup_801D0A78 {
  public:
    UnkTaskGroup_801D0A78();
    ~UnkTaskGroup_801D0A78();

    /* 0x00 */ Task *field_0x0;
    /* 0x04 */ Task *field_0x4;
    /* 0x08 */ u32 field_0x8;
};
CHECK_SIZEOF(UnkTaskGroup_801D0A78, 0xC);

#endif
