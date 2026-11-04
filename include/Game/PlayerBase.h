#ifndef GAME_PLAYERBASE_H
#define GAME_PLAYERBASE_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

/* The object that a PlayerBase subclass shares through the slot fn_80073604 (placeholder; the class
 * at 0x805200AC returns a copy of its shared pointer at 0x290). */
class UnkPlayerShared_80076D24;

namespace Player {

/* Player::PlayerObject (class name from RTTI, with CharInfo as its only base): 0x68 bytes. Its
 * constructor, inlined into PlayerBase's (0x80079F30-54), installs its vtable (0x80520388) and
 * clears the word at 0x64. The vtable overrides only the destructor (0x8007A418); every other slot
 * is CharInfo's. */
class PlayerObject : public CharInfo {
  public:
    virtual ~PlayerObject();

    /* 0x64 */ u32 field_0x64; /* PlayerBase's constructor stores its second argument here */
};
CHECK_SIZEOF(PlayerObject, 0x68);

/* Player::PlayerBase (class name from RTTI, with CharInfo and Player::PlayerObject as its bases):
 * 0x290 bytes. Its constructor (0x80079F04) initialises members from 0x68 through 0x28C, and a
 * derived class's constructor starts its own members at 0x290 (0x80077328-3C). Gadgets cast the
 * character info they act on to it. Only the vector at 0xE8 is mapped (the ElecFan gadget scales it
 * by -30 into the acceleration it sends the player).
 *
 * Its vtable (0x80520310, 0x54 bytes) overrides the destructor and the first three CharInfo slots
 * and adds nine slots. The added slots are named after their implementations here, or, for the
 * three pure ones, after the implementations in the subclass vtable at 0x8051FF2C; their purpose
 * is unknown and the signatures are inferred from the implementations and callers noted. */
class PlayerBase : public PlayerObject {
  public:
    virtual ~PlayerBase();
    /* 0x800798E8: also stores the argument at 0x54, then passes it on to the object at 0x254. */
    virtual void fn_8007A4CC(int arg0);
    virtual void fn_801DC980(CharCommand *command);
    virtual void fn_801DC97C(CharCommand *command);

    /* Creates the objects at 0x250 and 0x254 (reads only this). */
    virtual void fn_80079AB0();
    /* Pure. The subclass implementations clear (fn_80073AA8) and set (fn_80073A80) a flag and
     * return nothing, and the third (fn_80073A20) returns 0 or 1. */
    virtual void fn_80073AA8() = 0;
    virtual void fn_80073A80() = 0;
    virtual bool fn_80073A20() = 0;
    /* Empty here; the overrides read only this, and the call at 0x80076E40-50 to the second
     * passes only this. */
    virtual void fn_800735F8();
    virtual void fn_800735FC();
    virtual void fn_80073600();
    /* Returns an empty shared pointer here; the override at 0x80076D24 returns a copy of its
     * object's shared pointer at 0x290. */
    virtual boost::shared_ptr<UnkPlayerShared_80076D24> fn_80073604();
    /* Returns false when bit 0x24 or 0x20 of the bit set at 0xA8 is set, when the word at 0x54 is
     * negative or when fn_80178B5C finds no object for it; otherwise whether fn_8016F280 returns
     * 0 for that object. */
    virtual bool fn_80079770();

    /* 0x068 */ u8 field_0x68[0xE8 - 0x68];
    /* 0x0E8 */ Fn8040Vector3 field_0xE8;
    /* 0x0F4 */ u8 field_0xF4[0x290 - 0xF4];
};
CHECK_SIZEOF(PlayerBase, 0x290);

} // namespace Player

#endif
