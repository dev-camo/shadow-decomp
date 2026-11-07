#ifndef GAME_PLAYERBASE_H
#define GAME_PLAYERBASE_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

/* The object that a PlayerBase subclass shares through the slot fn_80073604 (placeholder; Player::Player,
 * vtable 0x805200AC, returns a copy of its shared pointer at 0x290). */
class UnkPlayerShared_80076D24;

class Target;

/* A partial view of the object at 0x24C of a Player::PlayerBase (placeholder name). The code at
 * 0x800AD4BC-0x800AD4C4 (USA) stores a Player::Shadow::PlayerTarget there, whose
 * constructor calls the Player::PlayerTargetBase constructor (0x80206E20; both class names from
 * RTTI). Which of the two this pointer's static type and fn_80206574's class are is not
 * established. fn_80206574 returns a weak pointer to the Target that it holds at 0xC (see
 * LockonTarget.h).
 *
 * The prefix hides the vtable pointer and a strong owner at 0x4 that the native constructor and
 * destructor copy and release, so the view is only borrowed through pointers: construction,
 * copying, assignment and destruction are declared private and left undefined. */
class UnkTargetSource_80206574 {
  public:
    /* Returns a weak pointer built from the member at 0xC (fn_80206598, which constructs the
     * weak count from that member's shared count; behaviour inferred). */
    boost::weak_ptr<Target> fn_80206574();

    /* 0x00 */ u8 field_0x0[0xC];
    /* 0x0C */ boost::shared_ptr<Target> field_0xC;

  private:
    UnkTargetSource_80206574();
    UnkTargetSource_80206574(const UnkTargetSource_80206574 &);
    UnkTargetSource_80206574 &operator=(const UnkTargetSource_80206574 &);
    ~UnkTargetSource_80206574();
};
/* Checks only the recovered view; the complete native extent remains unknown. */
CHECK_SIZEOF(UnkTargetSource_80206574, 0x14);

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
 * character info they act on to it. Only three members are mapped: the first word of the bit set
 * at 0xA8 (the TornadoCollision gadget tests bit 0x14 of the set), the vector at 0xE8 (the
 * ElecFan gadget scales it by -30 into the acceleration it sends the player) and the pointer at
 * 0x24C (see UnkTargetSource_80206574).
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

    /* 0x068 */ u8 field_0x68[0xA8 - 0x68];
    /* The first word of a bit set; its other words are not mapped (fn_80079770 tests bits 0x24
     * and 0x20 of the set, the TornadoCollision gadget bit 0x14). */
    /* 0x0A8 */ u32 field_0xA8;
    /* 0x0AC */ u8 field_0xAC[0xE8 - 0xAC];
    /* 0x0E8 */ Fn8040Vector3 field_0xE8;
    /* 0x0F4 */ u8 field_0xF4[0x24C - 0xF4];
    /* 0x24C */ UnkTargetSource_80206574 *field_0x24C;
    /* 0x250 */ u8 field_0x250[0x290 - 0x250];
};
CHECK_SIZEOF(PlayerBase, 0x290);
CHECK_OFFSETOF(PlayerBase, field_0xA8, 0xA8);
CHECK_OFFSETOF(PlayerBase, field_0x24C, 0x24C);

} // namespace Player

/* A 0x70-byte record of the ring that the pointer at 0x250 of a Player::PlayerBase refers to
 * (records at its +0x4, newest index at its +0x8, 0x78 records of 0x70 bytes; placeholder name).
 * Only the vector at 0xC is mapped: the ELEC_BAR gadget takes the vectors of the two newest
 * records as the ends of a segment of the player's movement (inferred). */
struct UnkPlayerRecord_80078CEC {
    /* 0x00 */ u8 field_0x0[0xC];
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ u8 field_0x18[0x70 - 0x18];
};
CHECK_SIZEOF(UnkPlayerRecord_80078CEC, 0x70);

/* Returns the record index places before the newest one in the ring that the pointer at 0x250 of
 * player refers to (behaviour inferred). fn_800A82C8 subtracts index from the ring's newest index
 * and adds 120 (0x78) once if the result is negative, so only indices 0 to 119 are supported; the
 * ELEC_BAR gadget passes 0 and 1. */
extern "C" UnkPlayerRecord_80078CEC *fn_80078CEC(Player::PlayerBase *player, int index);

#endif
