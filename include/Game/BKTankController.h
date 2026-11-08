#ifndef GAME_BKTANKCONTROLLER_H
#define GAME_BKTANKCONTROLLER_H

#include "Game/PathObject.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetCharInfo;
/* The 0x70-byte object that BKTankController::Controller owns at 0x38 (placeholder name; the
 * original class is unknown). Its constructor (0x80322450) constructs a
 * SonicteamUSA::System::PathCtrl at its start and sets the words at 0x58 to 0x6C. */
class UnkPathControl_80322450;

namespace BKTankController {

/* BKTankController::Controller (class name from RTTI, with Task as its only base): the task that
 * the BK_TANK file creates under task layer 9 for its BKTank::SetBKTank. Its file (vtable
 * 0x8055C920) is not recovered: only what the BK_TANK file uses is declared. The constructor
 * (0x80321E44) builds a character info (fn_8015CD90(0x92, 4)), installs a command handler on it,
 * keeps the object at 0x30, clears the pointer at 0x38 and the word at 0x3C, sets the float at
 * 0x40 and clears the flags at 0x44 (behaviour inferred). */
class Controller : public Task {
  public:
    /* Passing object by value is inferred from the caller's copy, which the caller destroys after
     * the call. */
    Controller(Task *parent, boost::shared_ptr<PathObject> object);
    virtual ~Controller();

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<PathObject> m_object;
    /* Owned. Null after the constructor. The controller's function at 0x80321690 allocates 0x70
     * bytes with operator new, constructs the object there with 0x80322450 when the allocation
     * succeeded, and stores the pointer, null when it failed (0x8032174C-0x80321768). When the
     * pointer is not null, the destructor calls PathCtrl's destructor (0x80018770) on it, frees it
     * with operator delete and clears the pointer (0x80321A84-0x80321AAC). */
    /* 0x38 */ UnkPathControl_80322450 *field_0x38;
    /* used by the controller's own functions, which are not recovered */
    /* 0x3C */ u8 field_0x3C[0x44 - 0x3C];
    /* 0x44 */ UnkBitFlags_80014E04 m_flags;

  private:
    /* Copying is unrecovered: an implicit copy would duplicate the task links and share the
     * object that field_0x38 owns (declared only, as in C++03). */
    Controller(const Controller &);
    Controller &operator=(const Controller &);
};
CHECK_SIZEOF(Controller, 0x48);

} // namespace BKTankController

#endif
