#ifndef GAME_UNKGADGETOWNER_800C9CD0_H
#define GAME_UNKGADGETOWNER_800C9CD0_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

class SetAdapterState;

/* Accessed prefix of the owner's record; member names inferred from the accessors.
 * The full original extent is unknown. */
struct UnkOwnerRecord_800C9D24 {
    /* 0x00 */ Fn8040Vector3 position;
    /* 0x0C */ Fn8040Vector3 rotation;
    /* 0x18 */ u32 flags;
    /* 0x1C */ u8 field_0x1C[4];
    /* 0x20 */ u16 kind;
    /* 0x22 */ u8 field_0x22;
    /* 0x24 */ u32 parameterSize;
    /* 0x28 */ void *parameters;
};
CHECK_SIZEOF(UnkOwnerRecord_800C9D24, 0x2C); // Mapped prefix only.

/* The owner object that gadgets are registered with (placeholder name from the function that
 * registers a task with it). Only the accessed prefix is mapped; the complete size is not known. */
class UnkGadgetOwner_800C9CD0 {
  public:
    /* 0x00 */ UnkOwnerRecord_800C9D24 *m_record;
    /* 0x04 */ u8 field_0x4[4];
    /* 0x08 */ UnkGadgetOwner_800C9CD0 *m_next;
    /* 0x0C */ u8 field_0xC[0x8];
    /* The object that fn_800C9930 stores here, a SetAdapterState or a class derived from it (the
     * Chao gadget reads it back with dynamic_cast; name inferred). */
    /* 0x14 */ SetAdapterState *m_adapterState;
};
CHECK_OFFSETOF(UnkGadgetOwner_800C9CD0, m_adapterState, 0x14);
CHECK_OFFSETOF(UnkGadgetOwner_800C9CD0, m_next, 0x8);
CHECK_SIZEOF(UnkGadgetOwner_800C9CD0, 0x18); // Mapped prefix only.

#endif
