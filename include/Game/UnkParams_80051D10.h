#ifndef GAME_UNKPARAMS_80051D10_H
#define GAME_UNKPARAMS_80051D10_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
namespace Weapon {
class LockonTarget;
class WeaponBase;
} // namespace Weapon

/* The 0x44-byte block that the constructors at 0x80051D10 and 0x800693AC build (placeholder name;
 * the original class is unknown). The first copies the owner and the lock-on target, stores the
 * translation of the matrix at 0x10, the matrix's rotation applied to a constant vector at 0x1C, a
 * copy of arg4 at 0x28 and arg2 at 0x34; the second copies the owner, leaves the lock-on target
 * empty, copies position, direction and arg4 to 0x10, 0x1C and 0x28 and stores arg2 at 0x34
 * (behaviour inferred). The destructor is at 0x8005C410. Taking the vectors of the second
 * constructor by reference is inferred from the temporary the BA GUNSHIP gadget passes. */
class UnkParams_80051D10 {
  public:
    UnkParams_80051D10(const boost::weak_ptr<CharInfo> &owner,
                       const boost::shared_ptr<Weapon::LockonTarget> &target, int arg2,
                       const Fn8040Matrix34 *matrix, const Fn8040Vector3 *arg4);
    UnkParams_80051D10(const boost::weak_ptr<CharInfo> &owner, int arg2,
                       const Fn8040Vector3 &position, const Fn8040Vector3 &direction,
                       const Fn8040Vector3 &arg4);
    /* The constructor at 0x80051FC8: copies the owner, leaves the lock-on target empty, copies
     * position and direction to 0x10 and 0x1C and stores arg2 at 0x34; when the owner still
     * locks, it copies the locked character info's field_0x18 to 0x28, and otherwise sets 0x28
     * to the clear value (fn_8040CE60) (behaviour from its body; the reference parameters are
     * inferred). */
    UnkParams_80051D10(const boost::weak_ptr<CharInfo> &owner, int arg2,
                       const Fn8040Vector3 &position, const Fn8040Vector3 &direction);
    /* The constructor at 0x80069218: as the one at 0x800693AC, but it also copies target to 0x8
     * (behaviour from its body; the reference parameters are inferred). */
    UnkParams_80051D10(const boost::weak_ptr<CharInfo> &owner,
                       const boost::shared_ptr<Weapon::LockonTarget> &target, int arg2,
                       const Fn8040Vector3 &position, const Fn8040Vector3 &direction,
                       const Fn8040Vector3 &arg4);
    ~UnkParams_80051D10();

    /* 0x00 */ boost::weak_ptr<CharInfo> m_owner;
    /* 0x08 */ boost::shared_ptr<Weapon::LockonTarget> m_target;
    /* 0x10 */ Fn8040Vector3 field_0x10;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
    /* 0x28 */ Fn8040Vector3 field_0x28;
    /* 0x34 */ int field_0x34;
    /* An object that fn_80051EB4 constructs (its constructor, 0x80051EE4, clears three words) and
     * fn_80051DBC destroys (type unknown). BkWormBase's stack frame at 0x80192608 leaves 0x44
     * bytes for the block (the other users' code is the same with 0x44 and 0x48). */
    /* 0x38 */ u8 field_0x38[0xC];
};
CHECK_SIZEOF(UnkParams_80051D10, 0x44);

/* The object that the callback at 0xC of UnkFactory_8051DDC8 returns (placeholder name; the
 * original class is unknown). fn_80051B58 constructs it when there is no callback. */
class UnkResult_80051B58;

/* A 0x14-byte record of the table that fn_80051C34 indexes, whose function at 0x8 creates an object
 * from the parameters (placeholder names; behaviour inferred from fn_80056B8C, the function
 * lbl_8051DDC8 holds, which allocates a 0x6C-byte object; lbl_8051DDDC holds fn_80056B3C). These
 * two records hold null at 0x0, 0x4 and 0xC; other records in the same table hold functions there,
 * which fn_80051CC8, fn_80051C80 and fn_80051AF0 call when they are set. */
struct UnkFactory_8051DDC8 {
    /* fn_80051CC8 calls it without setting arguments; the known callees take none. */
    /* 0x00 */ void (*field_0x0)();
    /* fn_80051C80 calls it without setting arguments; the known callees take none. */
    /* 0x04 */ void (*field_0x4)();
    /* 0x08 */ void (*create)(const UnkParams_80051D10 *params);
    /* Called by fn_80051AF0 with the parameters and a float; its result is returned by value. */
    /* 0x0C */ UnkResult_80051B58 (*field_0xC)(const UnkParams_80051D10 *params, float arg);
    /* 0x10 */ float field_0x10;
};
CHECK_SIZEOF(UnkFactory_8051DDC8, 0x14);

/* The ArkCannon gadget's shots go through the first record, the EggLastMech boss's through the
 * second. */
extern UnkFactory_8051DDC8 lbl_8051DDC8;
extern UnkFactory_8051DDC8 lbl_8051DDDC;

extern "C" {
/* Passes params to the function at 0x8 of the 0x14-byte record that the pointer at index kind of
 * the table lbl_8051D720 addresses, when the pointer and that function are set (behaviour inferred
 * from its body; the BA GUNSHIP gadget passes 0x1D for its shots). The bytes show only an address:
 * Weapon::HeavyShot builds the block that fn_80069424 or fn_80069298 returns at r1+0x8, passes
 * that address and destroys the block right after the call (USA (GUPE8P) 0x802F1A50-0x802F1A68).
 * The const reference spelling is inferred; a pointer to a named or bound temporary gives the
 * same code. */
void fn_80051C34(int kind, const UnkParams_80051D10 &params);
/* Call the function at 0x4 (fn_80051C80) or at 0x0 (fn_80051CC8) of the same record for kind when
 * the pointer and that function are set (behaviour from their bodies). Weapon::HeavyShot's unload
 * and load hooks pass 0xB. */
void fn_80051C80(int kind);
void fn_80051CC8(int kind);
/* Build the parameters for a bullet of weapon from the position of matrix with the second
 * constructor above, arg as arg4 and arg2 0 when the weapon's owner locks and its field_0x5C is 1,
 * otherwise 1 (behaviour from their bodies). The direction is matrix applied to a constant vector
 * (fn_8040B618) for fn_80069424, and the normalised difference target - position for
 * fn_80069298. The by-value return, the pointer, const and reference spellings and the parameter
 * names are inferred. Weapon::ChaosSpear::ChaosSpear::fn_8005C0CC
 * computes arg2 with the same rule in its own body, so the three may share an unidentified inline
 * helper. */
UnkParams_80051D10 fn_80069424(const Weapon::WeaponBase *weapon, Fn8040Matrix34 *matrix,
                               const Fn8040Vector3 &arg);
UnkParams_80051D10 fn_80069298(const Weapon::WeaponBase *weapon, Fn8040Matrix34 *matrix,
                               const Fn8040Vector3 &arg, const Fn8040Vector3 &target);
/* As fn_80069424 without arg: the same position, direction and arg2, passed to the constructor at
 * 0x80051FC8 (the third above), which fills 0x28 itself (behaviour from its body; the by-value
 * return and the pointer and const spellings are inferred, as for fn_80069424). */
UnkParams_80051D10 fn_80069540(const Weapon::WeaponBase *weapon, Fn8040Matrix34 *matrix);
}

#endif
