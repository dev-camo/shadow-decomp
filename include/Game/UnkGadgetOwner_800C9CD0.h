#ifndef GAME_UNKGADGETOWNER_800C9CD0_H
#define GAME_UNKGADGETOWNER_800C9CD0_H

#include <dolphin/types.h>
#include "layout_check.h"

class SetAdapterState;

/* The owner object that gadgets are registered with (placeholder name from the function that
 * registers a task with it). Only the member that the Chao gadget reads is mapped; the size is not
 * known. */
class UnkGadgetOwner_800C9CD0 {
  public:
    /* 0x00 */ u8 field_0x0[0x14];
    /* The object that fn_800C9930 stores here, a SetAdapterState or a class derived from it (the
     * Chao gadget reads it back with dynamic_cast; name inferred). */
    /* 0x14 */ SetAdapterState *m_adapterState;
};
CHECK_OFFSETOF(UnkGadgetOwner_800C9CD0, m_adapterState, 0x14);

#endif
