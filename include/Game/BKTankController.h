#ifndef GAME_BKTANKCONTROLLER_H
#define GAME_BKTANKCONTROLLER_H

#include "Game/PathObject.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;
class GadgetCharInfo;
/* The 0x70-byte object that BKTankController::Controller owns at 0x38 (placeholder name; the
 * original class is unknown), defined in BKTankController.cpp. Its constructor (0x80322450)
 * constructs a SonicteamUSA::System::PathCtrl at its start and sets the words at 0x58 to 0x6C. */
class UnkPathControl_80322450;

namespace BKTankController {

/* BKTankController::Controller (class name from RTTI, with Task as its only base): the task that
 * the BK_TANK file creates under task layer 9 for its BKTank::SetBKTank (vtable 0x8055C920). The
 * constructor (0x80321E44) builds a character info (fn_8015CD90(0x92, 4)), installs OnCommand as
 * its command handler, keeps the object at 0x30, clears the pointer at 0x38 and the state at
 * 0x3C, sets the wait time at 0x40 to 0 and clears the flags at 0x44. Update and OnCommand are
 * defined in BKTankController.cpp. Member, method and state names are inferred from that code;
 * Update is the name of the Task slot it overrides. */
class Controller : public Task {
  public:
    /* Passing object by value is inferred from the caller's copy, which the caller destroys after
     * the call. */
    Controller(Task *parent, boost::shared_ptr<PathObject> object);
    virtual ~Controller();
    /* Subtracts dt from the wait time, not below 0, and returns while the byte at 0xC8 of
     * lbl_8057D6A0 is set. Otherwise, by state:
     * - loading: once fn_80012408 returns 3 for key 1 and fn_80177CF8 returns 1, initialises the
     *   object, creates the path control, updates it once with the object's character info,
     *   places the model and goes on to moving;
     * - moving, while the object pointer is not null and the wait time is not above 0: updates
     *   the path control unless flag 0 is set, then places the model and updates the object. When
     *   the path control reported the end of its path and its NextRoute returns false, the
     *   mission that fn_800740E4 returns for 2 is passed to fn_8016B270 if it exists, and the
     *   state becomes done. When the object's IsFinished returns true, the state becomes finished
     *   (also right after it became done);
     * - finished: updates the object if the pointer is not null, calls fn_8016B224(1, 0) on that
     *   mission if it exists and goes on to done;
     * - done: updates the object if the pointer is not null. */
    virtual void Update(float dt);

    /* The command handler of m_charInfo, by command id:
     * - 0x301 (GadgetCommand::MoveStartCommand): clears the wait time and flag 0;
     * - 0x302 (GadgetCommand::MoveEndCommand): takes the command's wait time and sets flag 0
     *   when it is <= 0 (an ordered comparison, so a NaN does not set it);
     * - 9 (SwitchCommand): when the path control exists, toggles the switch word of the route
     *   that the command names (0 to 3);
     * - 6 (AccelerateCommand): when the path control exists, makes x of the command's vector its
     *   target speed; then, when z of the vector is <= 0, makes the speed the target speed and the
     *   acceleration 0, and otherwise makes the acceleration (target speed - speed) / z; clears
     *   flag 0 in either case;
     * - 2 (SetPositionCommand): does nothing when the mission that fn_800740E4 returns for 2
     *   exists and its word at 0xC is 2. Otherwise, when the path control exists, resets its
     *   speeds and selects the route x of the command's position (converted to an integer), if
     *   that route is not before the current one, with y as the fraction for SetRoute, which
     *   writes the distance only when 0 <= y && y <= 1; else the current route with the fraction
     *   0 (the last route when the routes were used up, going back from done to moving); clears
     *   flag 0 in either case.
     * Each case converts the command with dynamic_cast and does nothing when that fails. */
    void OnCommand(CharCommand &command);

    enum State { STATE_LOADING = 0, STATE_MOVING = 1, STATE_FINISHED = 2, STATE_DONE = 3 };

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<PathObject> m_object;
    /* Owned. Null after the constructor. Update allocates 0x70 bytes with operator new,
     * constructs the object there with 0x80322450 when the allocation succeeded, and stores the
     * pointer, null when it failed (0x8032174C-0x80321768). When the pointer is not null, the
     * destructor calls PathCtrl's destructor (0x80018770) on it, frees it with operator delete and
     * clears the pointer (0x80321A84-0x80321AAC). */
    /* 0x38 */ UnkPathControl_80322450 *m_pathControl;
    /* 0x3C */ State m_state;
    /* 0x40 */ float m_waitTime;
    /* 0x44 */ UnkBitFlags_80014E04 m_flags; /* bit 0: Update does not advance the path control */

  private:
    /* Copying is unrecovered: an implicit copy would duplicate the task links and share the
     * object that m_pathControl owns (declared only, as in C++03). */
    Controller(const Controller &);
    Controller &operator=(const Controller &);
};
CHECK_SIZEOF(Controller, 0x48);

} // namespace BKTankController

#endif
