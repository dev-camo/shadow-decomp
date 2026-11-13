#ifndef GAME_PLAYERCONTROLAPPROACH_H
#define GAME_PLAYERCONTROLAPPROACH_H

#include "Game/PlayerControl.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class Target;

namespace Player {
namespace Control {

/* Player::Control::InitializeArgsApproach (class name from RTTI, with
 * Player::Control::InitializeArgs as its only base): the arguments for the control with id 1,
 * Player::Control::Approach, 0x14 bytes. The constructor is defined in the class: its only copy
 * (0x8009B470 in USA (GUPE8P)) is in the file of the command handler at 0x8009B360, its only
 * caller, which passes the word at 0x14 and the weak pointer at 0x18 of its command and gives the
 * result to fn_8009EBA4. The destructor is the implicit one; its copy (0x8009B400, between the
 * handler and the constructor) and the vtable (0x80520D4C) are in that file as well. The const
 * reference and the member names are inferred (see Approach's members). */
class InitializeArgsApproach : public InitializeArgs {
  public:
    InitializeArgsApproach(int action, const boost::weak_ptr<Target> &target)
        : InitializeArgs(1), m_action(action), m_target(target) {}

    /* 0x08 */ int m_action;
    /* 0x0C */ boost::weak_ptr<Target> m_target;
};
CHECK_SIZEOF(InitializeArgsApproach, 0x14);

/* Player::Control::Approach (class name from RTTI, with Player::Control::ControlBase as its only
 * base): the control with id 1, which steers the player towards a target and then sends the
 * player a command for it (see Update). Member names are inferred from that. */
class Approach : public ControlBase {
  public:
    Approach(const boost::shared_ptr<PlayerBase> &player);
    virtual ~Approach();
    /* Takes the action and the target from args, which must be an InitializeArgsApproach (a
     * reference cast). When the target still exists, copies the target's weak pointer to its
     * character info to m_targetInfo and passes it to fn_80095A64 of the player's object at
     * 0x248. Clears m_time. */
    virtual void fn_8009E1A4(int previousId, const InitializeArgs &args);
    /* Calls fn_80095A40 for the player's object at 0x248. */
    virtual void fn_8009E17C(int nextId);
    virtual void Update(float dt);

    /* Allocates an Approach for the player and constructs it only when the allocation succeeds;
     * returns the result, which is null when it fails (name inferred). The result type is that
     * of the base: the code at 0x800AED14-0x800AED80 passes this function's result and those of
     * the functions at 0x8009F9FC and 0x8009E324, which create the UserInput and Bukuu controls,
     * to the same shared pointer constructor (0x800AEDC8). */
    static ControlBase *Create(const boost::shared_ptr<PlayerBase> &player);

    /* "Approach", which the constructor passes to the base (name inferred). */
    static const char *s_name;

    /* What Update sends once the player has reached the target: 1 a
     * Player::PickupObjectCommand, 2 a Player::TurnOverCommand, anything else nothing. */
    /* 0x14 */ int m_action;
    /* 0x18 */ boost::weak_ptr<Target> m_target;
    /* 0x20 */ boost::weak_ptr<CharInfo> m_targetInfo;
    /* The sum of Update's arguments since fn_8009E1A4, counting only the updates that get past the
     * target, character info and bit 2 checks (the others end the control before the addition). */
    /* 0x28 */ float m_time;
};
CHECK_SIZEOF(Approach, 0x2C);

} // namespace Control
} // namespace Player

#endif
