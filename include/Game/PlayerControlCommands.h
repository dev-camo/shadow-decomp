#ifndef GAME_PLAYERCONTROLCOMMANDS_H
#define GAME_PLAYERCONTROLCOMMANDS_H

#include "Game/CharCommand.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class Target;

/* Commands that the player's controls (Player::Control) send to their own player; the commands
 * that gadgets send are in Game/PlayerCommands.h. Class names are from RTTI, with
 * Player::PlayerCommand as their base. As in that header, each class declares the overrides that
 * its original vtable holds, under CharCommand's placeholder names, and the destructors and
 * vtables are defined elsewhere. The constructors are defined in their classes: the original has
 * one copy of each after the first function in the link that constructs the command, and none is
 * expanded at its uses, because each calls the inline constructor of its base class. The
 * addresses below are those of USA (GUPE8P). */

namespace Player {

/* Player::TurnOverCommand (class name from the RTTI of its vtable at 0x8052112C); the destructor
 * (0x8009D1FC) and the vtable are elsewhere. The constructor passes 0x11F and arg1 to the base
 * constructor and copies the weak pointer to 0x14. It is defined in the class: its only copy
 * (0x8009E0C4) follows Player::Control::Approach's Update, its only caller, which passes its own
 * weak pointer member and 2. When fn_8009CC30 gives a player for the receiver, the handler
 * (0x8009AF40) passes a Player::Behavior::InitializeArgsTurnOver that it builds from the weak
 * pointer to fn_800A7F14 of the player's object at 0x22C and sets bit 1 of the command's flags.
 * The member's name is inferred. */
class TurnOverCommand : public PlayerCommand {
  public:
    TurnOverCommand(const boost::weak_ptr<Target> &target, int arg1)
        : PlayerCommand(0x11F, arg1), m_target(target) {}
    virtual ~TurnOverCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009AF40 */

    /* 0x14 */ boost::weak_ptr<Target> m_target;
};
CHECK_SIZEOF(TurnOverCommand, 0x1C);

/* Player::PickupObjectCommand (class name from the RTTI of its vtable at 0x80521184); the
 * destructor (0x8009D2D4) and the vtable are elsewhere. The constructor passes 0x11D and arg1 to
 * the base constructor and copies the weak pointer to 0x14. It is defined in the class: its only
 * copy (0x8009E114) follows TurnOverCommand's after Player::Control::Approach's Update, its only
 * caller. Its handler (0x8009B200) does what TurnOverCommand's does with a
 * Player::Behavior::InitializeArgsPickupObject. The member's name is inferred. */
class PickupObjectCommand : public PlayerCommand {
  public:
    PickupObjectCommand(const boost::weak_ptr<Target> &target, int arg1)
        : PlayerCommand(0x11D, arg1), m_target(target) {}
    virtual ~PickupObjectCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009B200 */

    /* 0x14 */ boost::weak_ptr<Target> m_target;
};
CHECK_SIZEOF(PickupObjectCommand, 0x1C);

} // namespace Player

#endif
