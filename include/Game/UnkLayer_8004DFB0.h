#ifndef GAME_UNKLAYER_8004DFB0_H
#define GAME_UNKLAYER_8004DFB0_H

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

// RednerDrawArgs is the original spelling (from the RTTI name).
class RednerDrawArgs;

typedef boost::function1<void, RednerDrawArgs &> RenderDrawCallback;
CHECK_SIZEOF(RenderDrawCallback, 0xC);

class Task;

/* The 0x28-byte draw layer that fn_8004DFB0 creates (placeholder name; only the members the gadgets
 * use are mapped). Its constructor (0x8004E618) calls an empty constructor (0x8004E6CC) on the
 * first 0x10 bytes, whose class is not established, and creates the layer's task at 0x24 with the
 * parent that fn_8004DFB0 receives; fn_8004DFB0 then stores its name argument as that task's
 * name. */
class UnkLayer_8004DFB0 {
  public:
    /* 0x00 */ u8 field_0x0[0x10];
    /* Called by the layer task's update when set (0x8004E368-7C). The functions that fn_8004E2D4
     * installs here (0x802E1534-74: fn_802E1594, fn_802E1780, fn_802E192C) read no arguments and
     * return no value, so the signature is inferred. */
    /* 0x10 */ void (*field_0x10)();
    /* 0x14 */ RenderDrawCallback m_drawCallback; /* called when the layer is drawn */
    /* 0x20 */ u8 field_0x20[0x4];
    /* The layer's task; the layer's destructor marks it finished and clears the pointer
     * (0x8004E4CC-E8). */
    /* 0x24 */ Task *field_0x24;
};
CHECK_SIZEOF(UnkLayer_8004DFB0, 0x28);

/* The 0x6C-byte manager that fn_80046178 returns (placeholder; a static object constructed by
 * fn_801E47FC, which creates tasks and stores them in its words up to 0x68). Its words at 0x8, 0x28
 * and 0x38 are the parent tasks that fn_8004DFB0 takes (CircusPole passes the first, ElecFanDraw the
 * second, the GONG gadget's GongCSD the third); the rest of the class is not mapped. */
class UnkLayerManager_80046178 {
  public:
    /* 0x00 */ u8 field_0x0[0x8];
    /* 0x08 */ Task *field_0x8;
    /* 0x0C */ u8 field_0xC[0x28 - 0xC];
    /* 0x28 */ Task *field_0x28;
    /* 0x2C */ u8 field_0x2C[0x38 - 0x2C];
    /* 0x38 */ Task *field_0x38;
    /* 0x3C */ u8 field_0x3C[0x6C - 0x3C];
};
CHECK_SIZEOF(UnkLayerManager_80046178, 0x6C);

extern "C" {
UnkLayerManager_80046178 *fn_80046178(void);
boost::shared_ptr<UnkLayer_8004DFB0> fn_8004DFB0(Task *parent, const char *name);
/* Creates a draw layer under parent as fn_8004DFB0 does, without giving its task a name
 * (behaviour inferred from its body). */
boost::shared_ptr<UnkLayer_8004DFB0> fn_8004E24C(Task *parent);
}

#endif
