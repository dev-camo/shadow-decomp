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
} // namespace Weapon

/* The 0x48-byte block that the constructors at 0x80051D10 and 0x800693AC build (placeholder name;
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
    ~UnkParams_80051D10();

    /* 0x00 */ boost::weak_ptr<CharInfo> m_owner;
    /* 0x08 */ boost::shared_ptr<Weapon::LockonTarget> m_target;
    /* 0x10 */ Fn8040Vector3 field_0x10;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
    /* 0x28 */ Fn8040Vector3 field_0x28;
    /* 0x34 */ int field_0x34;
    /* An object that fn_80051EB4 constructs and fn_80051DBC destroys (type unknown). */
    /* 0x38 */ u8 field_0x38[0x10];
};
CHECK_SIZEOF(UnkParams_80051D10, 0x48);

extern "C" {
/* Passes params to the function at 0x8 of the 0x14-byte record that the pointer at index kind of
 * the table lbl_8051D720 addresses, when the pointer and that function are set (behaviour inferred
 * from its body; the BA GUNSHIP gadget passes 0x1D for its shots). */
void fn_80051C34(int kind, const UnkParams_80051D10 *params);
}

#endif
