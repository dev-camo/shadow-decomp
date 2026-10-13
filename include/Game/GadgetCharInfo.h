#ifndef GAME_GADGETCHARINFO_H
#define GAME_GADGETCHARINFO_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>

/* GadgetCharInfo (class name from RTTI): the 0x6C-byte character info of a gadget. Only the
 * position and rotation copied from the owner are mapped (names inferred); the rest of the layout
 * is not yet known. */
class GadgetCharInfo {
  public:
    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ Fn8040Vector3 m_rotation; /* radians */
    /* 0x18 */ u8 field_0x18[0x6C - 0x18];
};

/* Builds the character info of a gadget named name (behaviour inferred; arguments unknown). */
extern "C" boost::shared_ptr<GadgetCharInfo> fn_8015CB88(int arg0, int arg1, const char *name);

#endif
