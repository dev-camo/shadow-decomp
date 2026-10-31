#ifndef GAME_GADGETCHARINFO_H
#define GAME_GADGETCHARINFO_H

#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* GadgetCharInfo (class name from RTTI, with CharInfo as its only base): the 0x6C-byte character
 * info of a gadget. */
class GadgetCharInfo : public CharInfo {
  public:
    virtual ~GadgetCharInfo();

    /* 0x64 */ u8 field_0x64[0x6C - 0x64];
};
CHECK_SIZEOF(GadgetCharInfo, 0x6C);

/* Builds the character info of a gadget named name (behaviour inferred; arguments unknown). */
extern "C" boost::shared_ptr<GadgetCharInfo> fn_8015CB88(int arg0, int arg1, const char *name);

/* Builds a character info without a name (behaviour inferred; arguments unknown): arg0 is passed to
 * the GadgetCharInfo constructor and arg1 is stored at 0x5C of the new object, as fn_8015CB88 does
 * with its arg1. */
extern "C" boost::shared_ptr<GadgetCharInfo> fn_8015CD90(int arg0, int arg1);

#endif
