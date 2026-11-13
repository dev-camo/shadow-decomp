#ifndef GAME_PICKUPCHARINFO_H
#define GAME_PICKUPCHARINFO_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;
class CharCommandUnit;
class Target;

/* PickupCharInfo (class name from RTTI, with CharInfo as its only base): the 0x80-byte character
 * info of an object that the player can pick up (inferred from the commands its owners handle). Its
 * constructor (0x801E4368) passes arg0 to the CharInfo constructor, sets the vtable, constructs the
 * members and stores a new CharCommandUnit in the pointer at 0x64 (an empty pointer when the
 * allocation fails). The HEALING_UNIT gadget keeps one in a shared pointer. */
class PickupCharInfo : public CharInfo {
  public:
    /* PickupCharInfo::SetGrabPositionArgs (name from the RTTI string of a handler bound to
     * it): what the grab position handler at 0x74 receives. This view declares only the word that
     * the HEALING_UNIT gadget's handler reads, a pointer to the matrix it copies (the rest of the
     * record, if any, is not recovered, so the size check below covers this view only). */
    struct SetGrabPositionArgs {
        /* 0x0 */ const Fn8040Matrix34 *m_matrix;
    };

    /* The handler at 0x74 (name inferred). */
    typedef boost::function1<void, SetGrabPositionArgs &> SetGrabPositionHandler;

    PickupCharInfo(int arg0);
    /* Defined elsewhere (0x801E429C). */
    virtual ~PickupCharInfo();
    /* The override at 0x801E424C. */
    virtual void fn_801DC980(CharCommand *command);

    /* Passes the target's address (the pointer at 0x6C) and position to fn_800CCE38 (behaviour
     * inferred from its body; the pointer spelling is inferred). */
    void fn_801E413C(const Fn8040Vector3 *position);
    /* Stores in the target pointer at 0x6C the target that fn_801E7128 creates from owner and
     * flags | 0x2020, then stores value in the target's float at 0x20 (behaviour inferred from its
     * body; the parameter names and the const reference are inferred). */
    void fn_801E4160(const boost::weak_ptr<CharInfo> &owner, int flags, float value);
    /* Calls the grab position handler at 0x74 with matrix when the handler is set (behaviour
     * inferred from its body; the const is inferred). */
    void fn_801E41C4(const Fn8040Matrix34 *matrix);

    /* 0x64 */ boost::shared_ptr<CharCommandUnit> m_commandUnit;
    /* 0x6C */ boost::shared_ptr<Target> m_target;
    /* 0x74 */ SetGrabPositionHandler m_grabHandler;
};
CHECK_SIZEOF(PickupCharInfo, 0x80);
CHECK_SIZEOF(PickupCharInfo::SetGrabPositionArgs, 0x4);

#endif
