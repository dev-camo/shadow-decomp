#ifndef GAME_CHARCOMMANDUNIT_H
#define GAME_CHARCOMMANDUNIT_H

#include "Game/CameraModeUser.h"
#include "boost/function.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

typedef boost::function1<void, CameraUpdateEventArgs &> CameraUpdateCallback;
CHECK_SIZEOF(CameraUpdateCallback, 0xC);

/* CharCommandUnit (class name from the RTTI of the vtable at 0x8053D118): the 0x10-byte object
 * that a GadgetCharInfo holds in a shared pointer at 0x64. Its constructor at 0x801DA780 sets the
 * vtable and initialises the callback at 0x4. The whole object is mapped; the last word of the
 * vtable (after the destructor) is not explained. */
class CharCommandUnit {
  public:
    virtual ~CharCommandUnit();

    /* 0x04 */ CameraUpdateCallback m_cameraCallback; /* the Coaster gadget binds its handler */
};
CHECK_SIZEOF(CharCommandUnit, 0x10);

#endif
