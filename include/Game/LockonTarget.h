#ifndef GAME_LOCKONTARGET_H
#define GAME_LOCKONTARGET_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class Target;

namespace Weapon {

/* Weapon::LockonMark (class name from RTTI): the task that marks the target; defined elsewhere. */
class LockonMark;

/* Weapon::LockonTarget (class name from the RTTI of its shared_ptr control block). The constructor
 * (0x8006118C) and the destructor (0x80061130) are defined elsewhere; the layout comes from them.
 * Method names are placeholders. */
class LockonTarget {
  public:
    LockonTarget();
    ~LockonTarget();

    /* Returns field_0x0: the body at 0x80061050 only returns, leaving the object's address as
     * the result, and BulletSatelliteLaser::SatelliteLaser's constructor locks that result as a
     * weak pointer to a Target. The const qualifiers are inferred. */
    const boost::weak_ptr<Target> &fn_80061050() const;
    /* Assigns object to field_0x0 (behaviour inferred from its body; the const reference is
     * inferred, as the bytes do not distinguish it from a pointer). */
    void fn_80061054(const boost::weak_ptr<Target> &object);
    /* Creates the Weapon::LockonMark task (fn_80061E68), which stores itself in field_0x8
     * (behaviour inferred). */
    void fn_80060F70();

    /* 0x0 */ boost::weak_ptr<Target> field_0x0;
    /* The mark that fn_80060F70 creates stores itself here; the constructor clears it. */
    /* 0x8 */ LockonMark *field_0x8;
};
CHECK_SIZEOF(Weapon::LockonTarget, 0xC);

} // namespace Weapon

#endif
