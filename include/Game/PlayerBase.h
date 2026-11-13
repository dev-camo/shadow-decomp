#ifndef GAME_PLAYERBASE_H
#define GAME_PLAYERBASE_H

#include "Game/CharInfo.h"
#include "Game/PlayerMotionEventArgs.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;
struct Fn8040DB6CEntry;
class UnkPlayerHistory_800A83D0;

/* The object that a PlayerBase subclass shares through the slot fn_80073604 (placeholder; Player::Player,
 * vtable 0x805200AC, returns a copy of its shared pointer at 0x290). */
class UnkPlayerShared_80076D24;

class Target;

namespace Player {
class PlayerMotion;
class WeaponBase;
class Attack;
class Effect;
class SoundBase;
namespace Behavior {
class Behavior;
}
} // namespace Player

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
 * fn_800A8784, its destructor, frees it when its flag is positive. With an entry present, its
 * update copies the two vectors and maps the flags through the 22 masks; without an entry,
 * it clears both vectors and the current flag word at 0x18. */
class UnkObject_80079F8C {
  public:
    UnkObject_80079F8C();
    ~UnkObject_80079F8C();
    void fn_800A871C(int index);
    void fn_800A85B0(float dt);

    /* 0x00 */ Fn8040Vector2 field_0x0;
    /* 0x08 */ Fn8040Vector2 field_0x8;
    /* 0x10 */ Fn8040DB6CEntry *field_0x10;
    /* 0x14 */ u32 field_0x14;
    /* 0x18 */ u32 field_0x18;
    /* 0x1C */ u32 field_0x1C;
    /* 0x20 */ u32 field_0x20;
    /* 0x24 */ u32 field_0x24;
    /* 0x28 */ u32 field_0x28;
    /* 0x2C */ u32 field_0x2C[22];
    /* 0x84 */ float field_0x84;
};
CHECK_SIZEOF(UnkObject_80079F8C, 0x88);

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

/* The object at PlayerBase offset 0x22C (placeholder name). fn_80074200 returns its shared
 * behaviour at 0x30; the remaining layout is not mapped here. */
class UnkObject_800A78FC;

namespace Player {

/* Player::PlayerObject (class name from RTTI, with CharInfo as its only base): 0x68 bytes. Its
 * constructor, inlined into PlayerBase's (0x80079F30-54) and into those of Player::ExternalOption
 * and Player::InternalOption (0x800AB49C-0x800AB4C0, 0x800AB65C-0x800AB680), installs its vtable
 * (0x80520388) and clears the word at 0x64. The vtable overrides only the destructor (0x8007A418);
 * every other slot is CharInfo's. The destructor is defined in the class: the option destructors
 * expand it (0x800AB430-0x800AB44C, 0x800AB5F0-0x800AB60C). */
class PlayerObject : public CharInfo {
  public:
    PlayerObject(int arg0) : CharInfo(arg0) {
        field_0x64 = 0;
    }
    virtual ~PlayerObject() {}

    /* Returns the word at 0x64 (name inferred). The original has one copy, at 0x8008769C in USA
     * (GUPE8P), with four callers: 0x80087278, 0x80087DF0 and the constructors of
     * Player::ExternalOption and Player::InternalOption (0x800AB4F4 and 0x800AB6B4). */
    s32 GetField0x64() const {
        return field_0x64;
    }
    /* Stores value in the word at 0x64 (name inferred).
     * Matching workaround: this inline member stands for an unknown original accessor, like
     * CharInfo::SetField0x5C. The option constructors call the smart pointer's operator-> and
     * GetField0x64 out of line for the value they store here (0x800AB4EC-0x800AB4F8), which this
     * compiler does only for the arguments of an inline call that is a whole statement; a plain
     * member store expands both in place (each constructor 0x98 bytes against 0x9C). Revisit
     * when the original accessor is identified. */
    void SetField0x64(s32 value) {
        field_0x64 = value;
    }

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
 * UnkOwner_80079E64) and the pointers at 0x22C (see UnkObject_800A78FC), 0x230 (see
 * Player::PlayerMotion), 0x244 (see UnkObject_80079BA4) and 0x24C (see
 * UnkTargetSource_80206574). Player::SoundBase's constructor stores the new sound task at 0x240
 * and its destructor clears it. The motion task dispatches events through the callback at 0x74
 * and mirrors its work flags at 0xB4; the attack and effect tasks occupy 0x234 and 0x23C.
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

    /* 0x068 */ u8 field_0x68[0x74 - 0x68];
    /* The motion task dispatches its decoded events through this callback. */
    /* 0x074 */ MotionEventCallback field_0x74;
    /* 0x080 */ u8 field_0x80[0xA0 - 0x80];
    /* Player::Behavior::Super sets bits 2, 7 and 9 when it starts and resets them when it ends. */
    /* 0x0A0 */ UnkBitFlags_80014E04 field_0xA0;
    /* 0x0A4 */ UnkBitFlags_80014E04 field_0xA4;
    /* A bit set of three words, which the constructor initialises at 0xA8, 0xAC and 0xB0; bit n
     * is in word n / 32 (fn_8006CD78). fn_80079770 tests bits 0x24 and 0x20 of the set, the
     * TornadoCollision gadget bit 0x14 and Player::Behavior::Super bits 0x1B and 0x3C. */
    /* 0x0A8 */ u32 field_0xA8[3];
    /* The motion task mirrors its sixteen control work flags here. */
    /* 0x0B4 */ UnkBitFlags_80014E04 field_0xB4;
    /* 0x0B8 */ Fn8040Vector3 field_0xB8;
    /* 0x0C4 */ u8 field_0xC4[0xDC - 0xC4];
    /* 0x0DC */ Fn8040Vector3 field_0xDC;
    /* 0x0E8 */ Fn8040Vector3 field_0xE8;
    /* 0x0F4 */ u8 field_0xF4[0x1C8 - 0xF4];
    /* Passed as the onto vector of fn_8040F990, which takes it to be of unit length. */
    /* 0x1C8 */ Fn8040Vector3 field_0x1C8;
    /* 0x1D4 */ u8 field_0x1D4[0x218 - 0x1D4];
    /* 0x218 */ UnkOwner_80079E64 field_0x218;
    /* 0x21C */ u8 field_0x21C[0x22C - 0x21C];
    /* May be null. The command handlers of Player::ExternalOption and Player::InternalOption pass
     * it to fn_800A78FC with a command of type 0x1C. */
    /* 0x22C */ UnkObject_800A78FC *field_0x22C;
    /* The constructor clears it; Player::PlayerMotion's destructor clears it too. */
    /* 0x230 */ PlayerMotion *field_0x230;
    /* 0x234 */ Attack *field_0x234;
    /* Player::WeaponBase's constructor stores the new object here through its owner pointer, and
     * its destructor clears it. */
    /* 0x238 */ WeaponBase *field_0x238;
    /* 0x23C */ Effect *field_0x23C;
    /* 0x240 */ SoundBase *field_0x240;
    /* The constructor clears it (0x8007A108). The destructor, when it is not null, calls its
     * destructor (fn_800A25E0) with flag 1, freeing it, and clears it (0x80079BA4-0x80079BBC). */
    /* 0x244 */ UnkObject_80079BA4 *field_0x244;
    /* 0x248 */ u8 field_0x248[0x24C - 0x248];
    /* 0x24C */ UnkTargetSource_80206574 *field_0x24C;
    /* Owned: fn_80079AB0 allocates this ring, and PlayerBase's destructor frees it. */
    /* 0x250 */ UnkPlayerHistory_800A83D0 *field_0x250;
    /* 0x254 */ u8 field_0x254[0x290 - 0x254];
};
CHECK_SIZEOF(PlayerBase, 0x290);
CHECK_OFFSETOF(PlayerBase, field_0x74, 0x74);
CHECK_OFFSETOF(PlayerBase, field_0xA0, 0xA0);
CHECK_OFFSETOF(PlayerBase, field_0xA4, 0xA4);
CHECK_OFFSETOF(PlayerBase, field_0xA8, 0xA8);
CHECK_OFFSETOF(PlayerBase, field_0xB4, 0xB4);
CHECK_OFFSETOF(PlayerBase, field_0xB8, 0xB8);
CHECK_OFFSETOF(PlayerBase, field_0xDC, 0xDC);
CHECK_OFFSETOF(PlayerBase, field_0x1C8, 0x1C8);
CHECK_OFFSETOF(PlayerBase, field_0x218, 0x218);
CHECK_OFFSETOF(PlayerBase, field_0x22C, 0x22C);
CHECK_OFFSETOF(PlayerBase, field_0x230, 0x230);
CHECK_OFFSETOF(PlayerBase, field_0x234, 0x234);
CHECK_OFFSETOF(PlayerBase, field_0x238, 0x238);
CHECK_OFFSETOF(PlayerBase, field_0x23C, 0x23C);
CHECK_OFFSETOF(PlayerBase, field_0x240, 0x240);
CHECK_OFFSETOF(PlayerBase, field_0x244, 0x244);
CHECK_OFFSETOF(PlayerBase, field_0x24C, 0x24C);
CHECK_OFFSETOF(PlayerBase, field_0x250, 0x250);

} // namespace Player

/* Three words of flags used by player history records (placeholder type). The native
 * constructor at 0x8007A4D4 clears three words, and fn_80095364 copies three words. */
class UnkPlayerFlags_8007A4D4 {
  public:
    UnkPlayerFlags_8007A4D4();
    void fn_80095364(const u32 *bits);

    /* 0x0 */ u32 field_0x0[3];
};
CHECK_SIZEOF(UnkPlayerFlags_8007A4D4, 0xC);

/* One record of the player's 120-entry history (placeholder name). Several vectors are copied
 * from PlayerBase; field_0x3C is the derived result of fn_800796DC. */
struct UnkPlayerRecord_80078CEC {
    UnkPlayerRecord_80078CEC();
    ~UnkPlayerRecord_80078CEC();
    void fn_800A849C(Player::PlayerBase *player);

    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ Fn8040Vector3 field_0x18;
    /* 0x24 */ Fn8040Vector3 field_0x24;
    /* 0x30 */ Fn8040Vector3 field_0x30;
    /* 0x3C */ Fn8040Vector3 field_0x3C;
    /* 0x48 */ UnkRotation3 field_0x48;
    /* 0x54 */ const char *field_0x54;
    /* 0x58 */ UnkPlayerFlags_8007A4D4 field_0x58;
    /* 0x64 */ UnkBitFlags_80014E04 field_0x64;
    /* 0x68 */ u32 field_0x68;
    /* 0x6C */ float field_0x6C;
};
CHECK_SIZEOF(UnkPlayerRecord_80078CEC, 0x70);

/* The 120-record ring owned at PlayerBase offset 0x250 (placeholder name). The update uses
 * its player pointer, newest index and records; the query indexes relative to the newest record. */
class UnkPlayerHistory_800A83D0 {
  public:
    explicit UnkPlayerHistory_800A83D0(Player::PlayerBase *player);
    ~UnkPlayerHistory_800A83D0();
    void fn_800A82EC();
    void fn_800A82E8();
    UnkPlayerRecord_80078CEC *fn_800A82C8(int index);

    /* 0x00 */ Player::PlayerBase *field_0x0;
    /* 0x04 */ UnkPlayerRecord_80078CEC *field_0x4;
    /* 0x08 */ int field_0x8;
    /* 0x0C */ int field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1C;

  private:
    /* Reconstruction safeguards: the native copy contract of this array owner is unknown. */
    UnkPlayerHistory_800A83D0(const UnkPlayerHistory_800A83D0 &);
    UnkPlayerHistory_800A83D0 &operator=(const UnkPlayerHistory_800A83D0 &);
};
CHECK_SIZEOF(UnkPlayerHistory_800A83D0, 0x20);

/* Returns the record index places before the newest one in the ring that the pointer at 0x250 of
 * player refers to (behaviour inferred). fn_800A82C8 subtracts index from the ring's newest index
 * and adds 120 (0x78) once if the result is negative, so only indices 0 to 119 are supported; the
 * ELEC_BAR gadget passes 0 and 1. */
extern "C" UnkPlayerRecord_80078CEC *fn_80078CEC(Player::PlayerBase *player, int index);

/* Signatures inferred from the original callers and bodies. fn_80074200 returns the
 * shared behaviour at offset 0x30 of the object at PlayerBase::field_0x22C. */
extern "C" boost::shared_ptr<Player::Behavior::Behavior> fn_80074200(UnkObject_800A78FC *object);
extern "C" void fn_80078B54(Player::PlayerBase *player);
extern "C" void fn_80079404(Player::PlayerBase *player);
extern "C" void fn_800794C8(Player::PlayerBase *player);

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
/* When the character info still exists and is a Player::PlayerBase, returns fn_800AADA4's weak
 * pointer for the object at 0x254 of the player (not tested here) and 1; otherwise an empty pointer
 * (behaviour from its body). The EggLastMech boss uses the result as the owner of its shots. */
boost::weak_ptr<CharInfo> fn_800B75DC(const boost::weak_ptr<CharInfo> &charInfo);
/* Takes the object at 0x22C of a player and a command and switches on the command's type; the rest
 * of its 0x3FC bytes is not described here. The void return is inferred: the two callers in
 * PlayerOption.cpp ignore the result. */
void fn_800A78FC(UnkObject_800A78FC *object, CharCommand *command);
}

#endif
