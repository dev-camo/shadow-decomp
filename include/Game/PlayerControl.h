#ifndef GAME_PLAYERCONTROL_H
#define GAME_PLAYERCONTROL_H

#include "Game/PlayerBase.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Player {

class Controller;

namespace Control {

/* Player::Control::InitializeArgs (class name from RTTI): what the player's controller is given
 * to change its control, 8 bytes. Its vtable (0x8051FC94 in USA (GUPE8P)) holds only the
 * destructor. The constructor and the destructor are defined in the class (inferred): the callers
 * store the vtable and the id themselves, and the destructor's only copy (0x8006D318) is in the
 * first file in the link that makes one. m_id is the id of the control to change to (name
 * inferred): the controller's fn_8009F398 passes it to slot 1 of its factory, which returns the
 * new control, and to fn_8009E17C of the control it replaces. InitializeArgsApproach's
 * constructor passes 1, the id that Player::Control::Approach gives its base, and Approach
 * passes 0 when it is done. */
class InitializeArgs {
  public:
    InitializeArgs(int id) : m_id(id) {}
    virtual ~InitializeArgs() {}

    /* 0x4 */ int m_id;
};
CHECK_SIZEOF(InitializeArgs, 0x8);

/* Player::Control::ControlBase (class name from RTTI, no bases): the base of the player's
 * controls (Approach, Bukuu and UserInput in Player::Control and six other classes whose
 * constructors call this one), 0x14 bytes. The constructor (0x8009DCC0) is in PlayerControl.cpp.
 * The destructor and fn_8006D14C are defined in the class (inferred): their only copies,
 * 0x8006D1A8 and 0x8006D14C, and the vtable (0x8051FC40) are among the code and data of the
 * NpcNoControl class (in an unnamed namespace; its functions start at 0x8006D150), the first
 * class in the link to use ControlBase. The member names are inferred.
 *
 * The vtable leaves slots 1 to 3 pure. Slots 1 and 2 are named after
 * Player::Control::Approach's implementations and their purpose is unknown. The controller's
 * fn_8009F398 calls slot 2 of the control it replaces with the id of the new arguments, and then
 * slot 1 of the new control with the m_id of the replaced control (0 when there was none) and
 * the arguments; the parameter types are inferred from those two calls, and of the nine
 * implementations of each only Approach's slot 1 reads a parameter (the arguments). Update's name
 * is inferred: fn_8009F5D8, in the controller's file, calls it for the current control with the
 * float that the player's pointer at 0x60 refers to, and Approach adds that float to a time. No
 * override of fn_8006D14C and no call to it were found, so its parameters and result are unknown
 * (declared with none). */
class ControlBase {
  public:
    /* Copies the shared pointer to 0x4 and stores the name and the id at 0xC and 0x10. Approach
     * passes its name, "Approach", and 1. */
    ControlBase(const boost::shared_ptr<PlayerBase> &player, const char *name, int id);
    virtual ~ControlBase() {}
    virtual void fn_8009E1A4(int previousId, const InitializeArgs &args) = 0;
    virtual void fn_8009E17C(int nextId) = 0;
    virtual void Update(float dt) = 0;
    virtual void fn_8006D14C() {}

    /* 0x04 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x0C */ const char *m_name;
    /* 0x10 */ int m_id;
};
CHECK_SIZEOF(ControlBase, 0x14);

} // namespace Control
} // namespace Player

extern "C" {
/* A function of the Player::Controller task's file (placeholder name): passes args to
 * fn_8009F398 of the object that the controller's pointer at 0x28 refers to, which replaces the
 * current control with the one its factory makes for the id of args, and returns that function's
 * result, which is always 1. None of its 30 calls uses the result, so its type is inferred. */
bool fn_8009EBA4(Player::Controller *controller, const Player::Control::InitializeArgs &args);
}

#endif
