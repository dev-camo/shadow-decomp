#ifndef GAME_UNKOBJECT_8027C3C4_H
#define GAME_UNKOBJECT_8027C3C4_H

#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The three values that fn_8027C3C4 passes to the constructor of the object it creates
 * (placeholder name; the original type is unknown). Its two known users zero the block, then store
 * their values one by one. The BA GUNSHIP gadget fills it from its parameters "ComId", "num" and
 * "delay", so the member names are inferred from those. */
struct UnkObjectParams_8027C3C4 {
    UnkObjectParams_8027C3C4() : comId(0), num(0), delay(0.0f) {}

    /* 0x0 */ s32 comId;
    /* 0x4 */ s32 num;
    /* 0x8 */ float delay;
};
CHECK_SIZEOF(UnkObjectParams_8027C3C4, 0xC);

/* The 0x1C-byte object that UnkObject_8027C3C4's constructor allocates and constructs from the
 * parameters (0x8027CB14; placeholder name, layout not recovered here). */
class UnkObjectInner_8027CB14;

/* The 4-byte object that fn_8027C3C4 allocates and, when the allocation succeeds (0x8027C3E8, USA
 * (GUPE8P)), constructs, and returns in a shared pointer (placeholder name; the original class is
 * unknown). Its constructor (0x8027C78C) allocates the inner object, constructs it when that
 * allocation succeeds (0x8027C7B0) and keeps the result in its only word; 0x8027C61C returns that
 * word. The functions below are the ones the BA
 * GUNSHIP gadget calls (behaviour inferred from their bodies). */
class UnkObject_8027C3C4 {
  public:
    /* Clear (fn_8027C650) or set (fn_8027C67C) bit 0 of the flags at 0x18 of the inner object. */
    void fn_8027C650();
    void fn_8027C67C();
    /* Passes dt to the update of the inner object (fn_8027C7DC). */
    void fn_8027C6A8(float dt);

    /* 0x0 */ UnkObjectInner_8027CB14 *field_0x0;
};
CHECK_SIZEOF(UnkObject_8027C3C4, 0x4);

extern "C" {
/* Creates the object from params as described above; the pointer is null when the allocation
 * fails (behaviour inferred). Taking params by reference is inferred: both callers pass the
 * address of a block on their stack. */
boost::shared_ptr<UnkObject_8027C3C4> fn_8027C3C4(const UnkObjectParams_8027C3C4 &params);
}

#endif
