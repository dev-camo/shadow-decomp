#ifndef GAME_CHARACOLLIATTACK_H
#define GAME_CHARACOLLIATTACK_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace SonicteamUSA {
namespace System {
namespace CharaColli {

class CharaColliAttack;
class CharaColliBase;

/* The object at 0x14 of an AttackCallbackParam (placeholder name; the original type is unknown).
 * Only the word at 0x20 is mapped; the complete extent is unknown. */
struct UnkAttackPart_80192F14 {
    /* 0x00 */ u8 field_0x0[0x20];
    /* 0x20 */ u32 field_0x20;
};
CHECK_SIZEOF(UnkAttackPart_80192F14, 0x24);

/* SonicteamUSA::System::CharaColli::AttackCallbackParam (class name from the RTTI names of the
 * weapon stick's and ThreatObj's bound callbacks): what an attack object passes to its callbacks.
 * The mapped members form a 0x58-byte view; the complete native extent remains unknown. */
class AttackCallbackParam {
  public:
    /* 0x00 */ boost::weak_ptr<CharaColliAttack> field_0x0;
    /* 0x08 */ boost::weak_ptr<CharaColliAttack> field_0x8;
    /* 0x10 */ u8 field_0x10[0x14 - 0x10];
    /* BkWormBase's slot 0x70 (0x80192F14) compares the word at 0x20 of this object with 0xBBF,
     * the frame id (name inferred) it also passes to its display's fn_8019448C. */
    /* 0x14 */ UnkAttackPart_80192F14 *field_0x14;
    /* 0x18 */ Fn8040Vector3 field_0x18;
    /* 0x24 */ u8 field_0x24[0x30 - 0x24];
    /* The TurnOver gadget's first callback stores 1 here when the attack object of field_0x8
     * and that object's owner both still exist and the owner's word at 0x5C is 1. */
    /* 0x30 */ u8 field_0x30;
    /* 0x31 */ u8 field_0x31[0x34 - 0x31];
    /* The stick stores field_0x18 minus its owner's position here; GiantLantern stores
     * field_0x18 minus its character info's position, normalised (inferred). */
    /* 0x34 */ Fn8040Vector3 field_0x34;
    /* 0x40 */ u8 field_0x40[0x4C - 0x40];
    /* Meteor stores its movement speed here (inferred); GiantLantern stores 10. */
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ float field_0x50; /* ThreatObj subtracts it from its health; GiantLantern stores 1 */
    /* Bit 0 makes the stick count a hit; ThreatObj stores 1, 3 or 8, the Chao 1 or 8,
     * FallBuilding 1 or 5, PoleHolder's pole and stones 1 or 3, BAGunShip 1, 3 or 5,
     * FootingMovable 1 or 5. */
    /* 0x54 */ s32 field_0x54; /* signed word: compared as a signed value by Meteors */
};
/* Checks only the recovered view; the complete native extent remains unknown. */
CHECK_SIZEOF(SonicteamUSA::System::CharaColli::AttackCallbackParam, 0x58);
CHECK_OFFSETOF(SonicteamUSA::System::CharaColli::AttackCallbackParam, field_0x50, 0x50);
CHECK_OFFSETOF(SonicteamUSA::System::CharaColli::AttackCallbackParam, field_0x54, 0x54);

} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

typedef boost::function1<void, SonicteamUSA::System::CharaColli::AttackCallbackParam &>
    AttackCallback;
CHECK_SIZEOF(AttackCallback, 0xC);

extern "C" {
/* The character info that made the attack described by param (behaviour inferred). */
boost::shared_ptr<CharInfo>
fn_8015D570(SonicteamUSA::System::CharaColli::AttackCallbackParam *param);
/* True when the attacker of the attack described by param has the value 1, 2 or 3 in its word at
 * 0x50 (behaviour inferred). */
bool fn_8015D4E4(SonicteamUSA::System::CharaColli::AttackCallbackParam *param);
}

/* The class of the attack object's callback members (placeholder name: the binary does not name
 * it). Its constructor (0x8005A98C) only runs function1's default constructor (0x8005A9BC), and
 * its destructor (0x8005A5C4) only destroys the function1 at offset 0, passing -1 as for a member
 * rather than 0 as for a base class: the function1 is its one member. */
class UnkAttackCallback_8005A98C {
  public:
    /* 0x0 */ AttackCallback field_0x0;
};
CHECK_SIZEOF(UnkAttackCallback_8005A98C, 0xC);

class UnkAttackListNode_800535A4;
class UnkNodePool_8005A91C;

/* The attack object's member at 0x44 (placeholder name): a list of weak pointers to attack objects
 * (inferred: fn_800532F8 passes an attack to it, and its nodes are destroyed through
 * weak_ptr<CharaColliAttack>'s destructor). Its constructor (0x8005A8C0) and destructor
 * (0x8005A610) are out of line. The 0x18-byte nodes come from a shared pool (fn_8005A91C) or,
 * when the pool is exhausted, from operator new. */
class UnkAttackList_8005A8C0 {
  public:
    UnkAttackList_8005A8C0();
    ~UnkAttackList_8005A8C0();

    /* Receives a copied weak pointer for registration in this list (behaviour inferred). */
    void fn_80053340(boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> attack);

    /* 0x0 */ UnkNodePool_8005A91C *field_0x0; /* the pool fn_8005A91C returns */
    /* 0x4 */ int field_0x4;                   /* zeroed whenever the list is emptied */
    /* 0x8 */ UnkAttackListNode_800535A4 *field_0x8;
    /* 0xC */ UnkAttackListNode_800535A4 *field_0xC; /* the node the destructor's walk stops at */
};
CHECK_SIZEOF(UnkAttackList_8005A8C0, 0x10);

namespace SonicteamUSA {
namespace System {
namespace CharaColli {

/* SonicteamUSA::System::CharaColli::CharaColliAttack (class name from the
 * control-block RTTI): an attack object registered with the collision system. The constructor
 * at 0x8005A840 and destructor at 0x8005A538 establish its 0x58-byte layout; it has no virtual
 * functions. */
class CharaColliAttack {
  public:
    CharaColliAttack();

    /* Stores arg0 at 0x38, arg1 at 0x40 and arg4 at 0x34, copies the two callbacks into the first
     * and third callback members and sets the byte at 0x54 (behaviour inferred; the arguments'
     * meanings are unknown). */
    void fn_80054DD0(int arg0, int arg1, AttackCallback callback0, AttackCallback callback1,
                     float arg4);
    /* Sets the byte at 0x55, stores arg0 at 0x3C and assigns callback to the second callback
     * member (behaviour inferred from its body). */
    void fn_80096B64(int arg0, const AttackCallback &callback);
    /* Passes the attack on to the member at 0x44 (behaviour inferred). */
    void fn_800532F8(boost::weak_ptr<CharaColliAttack> attack);

    /* 0x00 */ boost::weak_ptr<CharaColliBase> field_0x0;
    /* 0x08 */ boost::weak_ptr<CharInfo> m_owner; /* name inferred */
    /* 0x10 */ UnkAttackCallback_8005A98C field_0x10;
    /* 0x1C */ UnkAttackCallback_8005A98C field_0x1C;
    /* 0x28 */ UnkAttackCallback_8005A98C field_0x28;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ int field_0x38;
    /* 0x3C */ int field_0x3C;
    /* Unsigned (inferred): 0x80192F14 compares its masked bits with cmplwi, which a signed field
     * compared with an unsigned mask also gives. */
    /* 0x40 */ u32 field_0x40;
    /* 0x44 */ UnkAttackList_8005A8C0 field_0x44;
    /* 0x54 */ u8 field_0x54;
    /* 0x55 */ u8 field_0x55;
};
CHECK_SIZEOF(CharaColliAttack, 0x58);

} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

extern "C" {
/* Factory scope and scalar spellings are inferred; arg0 is unused. */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack>
fn_8005A300(int arg0, boost::weak_ptr<CharInfo> owner, AttackCallback callback0,
            AttackCallback callback1, float arg4);
}

#endif
