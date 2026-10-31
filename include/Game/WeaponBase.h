#ifndef GAME_WEAPONBASE_H
#define GAME_WEAPONBASE_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The model object a weapon may carry at 0x1C (placeholder name: the original class is unknown).
 * fn_800631FC stores a vector in the entry index of the table the model's first member points to
 * (behaviour inferred); the weapon stick stores its stretch scale there. */
class UnkWeaponModel_800631FC;

extern "C" void fn_800631FC(UnkWeaponModel_800631FC *model, int index, const Fn8040Vector3 *value);

/* The object the weak pointer at 0x4 refers to (placeholder name: the original class is unknown;
 * only the pointer's constructor and destructor are seen, in Weapon::WeaponBase's). */
class UnkWeaponTarget_8005D16C;

namespace Weapon {

/* Weapon::WeaponBase (class name from RTTI): the base class of the weapons a character carries
 * (its RTTI derivatives are the gun, laser, lock-on, cannon, vacuum and stick bases). The layout
 * comes from its constructor and destructor, which are defined elsewhere, together with the base
 * implementations of its virtual functions. Member and method names marked inferred are not
 * original; the virtual functions that only the base implements keep placeholder names. */
class WeaponBase {
  public:
    /* Default-constructs field_0x4, copies the owner into 0xC and stores the two integers at 0x14
     * and 0x18; field_0x1C and the three floats at 0x2C are cleared and m_position is left unset
     * (behaviour inferred). Every weapon stick passes 5 as the third argument. */
    WeaponBase(const boost::weak_ptr<CharInfo> &owner, int arg1, int arg2);
    virtual ~WeaponBase();
    /* Copies position into m_position and three floats from arg1 into 0x2C to 0x34 (behaviour
     * inferred; the type of arg1 is unknown). */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const void *arg1);
    /* Per-frame entry point (name inferred); the base does nothing. */
    virtual void Update(float dt);
    /* The next two pass the model at 0x1C to fn_80063130 and fn_80063154 when it exists. */
    virtual void fn_8005CF64();
    virtual void fn_8005CF38();
    /* Passes field_0x14 to fn_80051AC8. */
    virtual void fn_8005CF14();
    /* Returns three constants (0, -1, -2.703125) as a vector. */
    virtual Fn8040Vector3 fn_8005CEF8();
    /* Pass field_0x14 to fn_8005B904 and fn_8005B8DC. */
    virtual void fn_8005CED4();
    virtual void fn_8005CEB0();
    /* Returns 0. */
    virtual int fn_8005C0DC();

    /* The character info that carries the weapon (name inferred). The out-of-line copy the linker
     * kept is at 0x80065034. */
    boost::weak_ptr<CharInfo> GetOwner() const {
        return m_owner;
    }

    /* 0x04 */ boost::weak_ptr<UnkWeaponTarget_8005D16C> field_0x4;
    /* 0x0C */ boost::weak_ptr<CharInfo> m_owner;   /* name inferred */
    /* 0x14 */ int field_0x14;                      /* the constructor's arg1 */
    /* 0x18 */ int field_0x18;                      /* the constructor's arg2 */
    /* 0x1C */ UnkWeaponModel_800631FC *field_0x1C; /* cleared by the constructor */
    /* 0x20 */ Fn8040Vector3 m_position; /* the weapon's sounds play here (name inferred) */
    /* 0x2C */ float field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
};
CHECK_SIZEOF(Weapon::WeaponBase, 0x38);

} // namespace Weapon

#endif
