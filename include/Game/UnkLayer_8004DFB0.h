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

/* The 0x28-byte draw layer that fn_8004DFB0 creates (placeholder name; only the members the gadget
 * uses are mapped): its callback at 0x14 is called when the layer is drawn. */
class UnkLayer_8004DFB0 {
  public:
    /* 0x00 */ u8 field_0x0[0x14];
    /* 0x14 */ RenderDrawCallback m_drawCallback;
    /* 0x20 */ u8 field_0x20[0x8];
};
CHECK_SIZEOF(UnkLayer_8004DFB0, 0x28);

/* The manager that fn_80046178 returns (placeholder). Its word at 0x8 is what fn_8004DFB0 takes. */
class UnkLayerManager_80046178 {
  public:
    /* 0x0 */ u8 field_0x0[0x8];
    /* 0x8 */ void *field_0x8;
};

extern "C" {
UnkLayerManager_80046178 *fn_80046178(void);
boost::shared_ptr<UnkLayer_8004DFB0> fn_8004DFB0(void *arg0, const char *name);
}

#endif
