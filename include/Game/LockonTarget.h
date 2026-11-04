#ifndef GAME_LOCKONTARGET_H
#define GAME_LOCKONTARGET_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The object that a lock-on target's weak pointer refers to (placeholder name: the original class
 * is unknown). */
class UnkLockonObject_8005FDF4;

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

    /* Assigns object to field_0x0 (behaviour inferred from its body). */
    void fn_80061054(const boost::weak_ptr<UnkLockonObject_8005FDF4> &object);
    /* Creates the Weapon::LockonMark task (fn_80061E68), which stores itself in field_0x8
     * (behaviour inferred). */
    void fn_80060F70();

    /* 0x0 */ boost::weak_ptr<UnkLockonObject_8005FDF4> field_0x0;
    /* The mark that fn_80060F70 creates stores itself here; the constructor clears it. */
    /* 0x8 */ LockonMark *field_0x8;
};
CHECK_SIZEOF(Weapon::LockonTarget, 0xC);

} // namespace Weapon

#endif
