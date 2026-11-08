#ifndef GAME_PLAYERBASE_H
#define GAME_PLAYERBASE_H

#include "Game/CharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

/* The object that a PlayerBase subclass shares through the slot fn_80073604 (placeholder; Player::Player,
 * vtable 0x805200AC, returns a copy of its shared pointer at 0x290). */
class UnkPlayerShared_80076D24;

class Target;

namespace Player {
class PlayerMotion;
}

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

/* The 0x88-byte object that PlayerBase's constructor creates for the owner at 0x218 (placeholder
 * name; USA, GUPE8P): it calls operator new(0x88) and fn_800A87C0 at 0x80079F8C-0x80079FA4.
 * fn_800A8784, its destructor, frees it when its flag is positive. Nothing else is recovered. */
class UnkObject_80079F8C;

/* The object that the pointer at 0x244 of a Player::PlayerBase refers to (placeholder name). Its
 * destructor, fn_800A25E0, destroys a member at 0x10 (fn_800A2630) and frees it when its flag is
 * positive. Its size and the code that creates it are not recovered. */
class UnkObject_80079BA4;

/* The one-word owner at 0x218 of a Player::PlayerBase (placeholder name; owner inferred). In USA
 * (GUPE8P), PlayerBase's constructor stores the new UnkObject_80079F8C straight into its word at
 * 0x80079FA4, with no separate call for the owner, and the destructor destroys it in place
 * at 0x80079C08-0x80079C10 with fn_80079E64 and flag -1; fn_80079E64 calls the object's destructor
 * (fn_800A8784) with flag 1, freeing it, and frees the owner itself only for a positive flag.
 * As safeguards for this view, the destructor (fn_80079E64) is declared without a body, so no
 * no-op destructor stands in for that cleanup, and the copy constructor and assignment are
 * private and undefined; the original copy contract is not established. */
class UnkOwner_80079E64 {
  public:
    /* fn_80079E64 (not recovered). */
    ~UnkOwner_80079E64();

    /* 0x0 */ UnkObject_80079F8C *field_0x0;

  private:
    UnkOwner_80079E64(const UnkOwner_80079E64 &);
    UnkOwner_80079E64 &operator=(const UnkOwner_80079E64 &);
};
CHECK_SIZEOF(UnkOwner_80079E64, 0x4);

namespace Player {

/* Player::PlayerObject (class name from RTTI, with CharInfo as its only base): 0x68 bytes. Its
 * constructor, inlined into PlayerBase's (0x80079F30-54), installs its vtable (0x80520388) and
 * clears the word at 0x64. The vtable overrides only the destructor (0x8007A418); every other slot
 * is CharInfo's. */
class PlayerObject : public CharInfo {
  public:
    virtual ~PlayerObject();

    /* PlayerBase's constructor stores its second argument here. The HintController file compares
     * it with 9 as a signed word and indexes a table of 20 message ids by it. */
    /* 0x64 */ s32 field_0x64;
};
CHECK_SIZEOF(PlayerObject, 0x68);

/* Player::PlayerBase (class name from RTTI, with CharInfo and Player::PlayerObject as its bases):
 * 0x290 bytes. Its constructor (0x80079F04) initialises members from 0x68 through 0x28C, and a
 * derived class's constructor starts its own members at 0x290 (0x80077328-3C). Gadgets cast the
 * character info they act on to it. Few members are mapped: the flags at 0xA0, the three-word
 * bit set at 0xA8 (the TornadoCollision gadget tests bit 0x14 of the set), the vectors at
 * 0xB8, 0xDC and 0x1C8 that Player::Behavior::Super reads, the vector at 0xE8 (the ElecFan gadget
 * scales it by -30 into the acceleration it sends the player), the owner at 0x218 (see
 * UnkOwner_80079E64) and the pointers at 0x230 (see Player::PlayerMotion), 0x244 (see
 * UnkObject_80079BA4) and 0x24C (see UnkTargetSource_80206574). The word at 0x240 is not mapped
 * here: Player::SoundBase's constructor stores the new sound task in it and its destructor clears
 * it.
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

    /* 0x068 */ u8 field_0x68[0xA0 - 0x68];
    /* Player::Behavior::Super sets bits 2, 7 and 9 when it starts and resets them when it ends. */
    /* 0x0A0 */ UnkBitFlags_80014E04 field_0xA0;
    /* 0x0A4 */ u8 field_0xA4[0xA8 - 0xA4];
    /* A bit set of three words, which the constructor initialises at 0xA8, 0xAC and 0xB0; bit n
     * is in word n / 32 (fn_8006CD78). fn_80079770 tests bits 0x24 and 0x20 of the set, the
     * TornadoCollision gadget bit 0x14 and Player::Behavior::Super bits 0x1B and 0x3C. */
    /* 0x0A8 */ u32 field_0xA8[3];
    /* 0x0B4 */ u8 field_0xB4[0xB8 - 0xB4];
    /* 0x0B8 */ Fn8040Vector3 field_0xB8;
    /* 0x0C4 */ u8 field_0xC4[0xDC - 0xC4];
    /* 0x0DC */ Fn8040Vector3 field_0xDC;
    /* 0x0E8 */ Fn8040Vector3 field_0xE8;
    /* 0x0F4 */ u8 field_0xF4[0x1C8 - 0xF4];
    /* Passed as the onto vector of fn_8040F990, which takes it to be of unit length. */
    /* 0x1C8 */ Fn8040Vector3 field_0x1C8;
    /* 0x1D4 */ u8 field_0x1D4[0x218 - 0x1D4];
    /* 0x218 */ UnkOwner_80079E64 field_0x218;
    /* 0x21C */ u8 field_0x21C[0x230 - 0x21C];
    /* The constructor clears it; Player::PlayerMotion's destructor clears it too. */
    /* 0x230 */ PlayerMotion *field_0x230;
    /* 0x234 */ u8 field_0x234[0x244 - 0x234];
    /* The constructor clears it (0x8007A108). The destructor, when it is not null, calls its
     * destructor (fn_800A25E0) with flag 1, freeing it, and clears it (0x80079BA4-0x80079BBC). */
    /* 0x244 */ UnkObject_80079BA4 *field_0x244;
    /* 0x248 */ u8 field_0x248[0x24C - 0x248];
    /* 0x24C */ UnkTargetSource_80206574 *field_0x24C;
    /* 0x250 */ u8 field_0x250[0x290 - 0x250];
};
CHECK_SIZEOF(PlayerBase, 0x290);
CHECK_OFFSETOF(PlayerBase, field_0xA0, 0xA0);
CHECK_OFFSETOF(PlayerBase, field_0xA8, 0xA8);
CHECK_OFFSETOF(PlayerBase, field_0xB8, 0xB8);
CHECK_OFFSETOF(PlayerBase, field_0xDC, 0xDC);
CHECK_OFFSETOF(PlayerBase, field_0x1C8, 0x1C8);
CHECK_OFFSETOF(PlayerBase, field_0x218, 0x218);
CHECK_OFFSETOF(PlayerBase, field_0x230, 0x230);
CHECK_OFFSETOF(PlayerBase, field_0x244, 0x244);
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

/* Player functions that Player::Behavior::Super calls. Their names, the void returns and the
 * const qualifiers are not known (inferred); the behaviour comes from their bodies. */
extern "C" {
/* Returns (0, 0, -1) transformed by the rotation of the matrix at 0x1E4 of player, turned by the
 * angles in CharInfo::m_rotation (fn_80079628). */
Fn8040Vector3 fn_800796DC(Player::PlayerBase *player);
/* Calls fn_800936F4 with the shared pointer that boost::weak_ptr<Player::PlayerBase>::lock
 * (0x8006D754 in USA (GUPE8P)) makes from the weak pointer at 0x260 of player and with the float
 * that CharInfo's pointer at 0x60 refers to. */
void fn_80079720(Player::PlayerBase *player);
/* Returns fn_800796DC's vector when the word at 0x54 of the player is -1. Otherwise it takes a
 * vector from fn_80093F74 for that word, normalises it (fn_8040CE74), crosses the vector at 0x1C8
 * with it, crosses that result with the vector at 0x1C8 again and returns the normalised result
 * (fn_8040CEE8). */
Fn8040Vector3 fn_80093FC0(const boost::shared_ptr<Player::PlayerBase> &player);
/* Stores in the player's float at 0x10 (the y component of CharInfo::m_rotation) what
 * fn_8040F314 returns for it, for an angle that it derives from direction through the matrix at
 * 0x1E4 (fn_8040BEF8, fn_8040B67C, fn_8003B9E8) and for rate, which it first replaces with the
 * float at 0x805F29B0 (USA, GUPE8P) when rate is greater. */
void fn_80093BA0(const boost::shared_ptr<Player::PlayerBase> &player,
                 const Fn8040Vector3 &direction, float rate);
/* When bit 0x1B of the bit set at 0xA8 is set, calls fn_80093BA0 with a copy of the vector at
 * 0xB8 and rate; otherwise takes the part of the vector at 0x18 that is perpendicular to the
 * vector at 0x1C8 (fn_8040F990) and calls fn_80093BA0 with it and rate unless its squared length
 * is less than the square of the float at 0x805E4598 (10.0f). */
void fn_80093AF8(const boost::shared_ptr<Player::PlayerBase> &player, float rate);
/* Starts by resetting bits 0 and 4 of the flags at 0xA4 of the player; the rest of its 0x714
 * bytes is not described here. */
void fn_8009728C(const boost::shared_ptr<Player::PlayerBase> &player, int arg1);
}

#endif
