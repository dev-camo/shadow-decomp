#ifndef GAME_PLAYERCOMMANDS_H
#define GAME_PLAYERCOMMANDS_H

#include "Game/CharCommand.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* Commands that gadgets send to character infos (class names from RTTI, with CharCommand or
 * Player::PlayerCommand as their only base). The constructors and destructors are defined
 * elsewhere, together with the vtables; the argument meanings are inferred from the stores in
 * the constructors. SetPositionCommand, SetVelocityCommand and LaunchCommand pass their last
 * argument to the base constructor as its second argument, and ClrVelocityCommand its only
 * argument; IgnoreInputCommand passes 0. Most callers pass 4 there (0 and 2 also occur). */

/* Moves the character info to a position. */
class SetPositionCommand : public CharCommand {
  public:
    SetPositionCommand(const Fn8040Vector3 *position, int arg1);
    virtual ~SetPositionCommand();

    /* 0x14 */ Fn8040Vector3 m_position;
};
CHECK_SIZEOF(SetPositionCommand, 0x20);

/* Sets the velocity of the character info. */
class SetVelocityCommand : public CharCommand {
  public:
    SetVelocityCommand(const Fn8040Vector3 *velocity, int arg1);
    virtual ~SetVelocityCommand();

    /* 0x14 */ Fn8040Vector3 m_velocity;
};
CHECK_SIZEOF(SetVelocityCommand, 0x20);

/* Clears the velocity of the character info. */
class ClrVelocityCommand : public CharCommand {
  public:
    ClrVelocityCommand(int arg1);
    virtual ~ClrVelocityCommand();
};
CHECK_SIZEOF(ClrVelocityCommand, 0x14);

/* The command with the id 1. Its constructor (0x801DBE84) receives a vector by address (the bytes
 * do not show whether as a pointer or a reference) and the base constructor's second argument,
 * and copies the vector to 0x14. The Coaster gadget scales the roll of its ride by the length of
 * the vector and its direction. */
class AccelerateInputCommand : public CharCommand {
  public:
    /* 0x14 */ Fn8040Vector3 m_vector;
};
CHECK_SIZEOF(AccelerateInputCommand, 0x20);

/* The command with the id 0x17. Its constructor (0x801DAB3C) passes 0x17 and 4 to the base
 * constructor and stores a byte, a float and a word. The member names are inferred from the
 * FootingBreak gadget, which passes the float to its model's fn_8015FDD8 when the byte is set
 * and calls fn_8015FD10 otherwise; the word is not used there. */
class SetTransparentCommand : public CharCommand {
  public:
    /* 0x14 */ u8 m_enable;
    /* 0x18 */ float m_alpha;
    /* 0x1C */ int field_0x1C;
};
CHECK_SIZEOF(SetTransparentCommand, 0x20);

namespace Player {

/* Makes the player ignore input for a time (the first member holds the time; the gadgets that
 * send it pass a time in seconds. The other two are a word the constructor sets to 0 and the
 * constructor's first argument). */
class IgnoreInputCommand : public PlayerCommand {
  public:
    IgnoreInputCommand(int arg0, float seconds);
    virtual ~IgnoreInputCommand();

    /* 0x14 */ float m_seconds;
    /* 0x18 */ u32 field_0x18;
    /* 0x1C */ int m_arg0;
};
CHECK_SIZEOF(Player::IgnoreInputCommand, 0x20);

/* Launches the player (the first argument is stored at 0x14; its meaning is unknown). */
class LaunchCommand : public PlayerCommand {
  public:
    LaunchCommand(int arg0, int arg1);
    virtual ~LaunchCommand();

    /* 0x14 */ int m_arg0;
};
CHECK_SIZEOF(Player::LaunchCommand, 0x18);

/* The command that the Coaster gadget sends to the character it carries. The constructor at
 * 0x80115B70 passes 0x112 as the command id, stores a copy of target at 0x14, the two vector
 * pointers at 0x1C and 0x20 and arg3 at 0x24; the meaning of the vectors and numbers is not
 * known (the Coaster gadget passes its position and the angles of its turned matrix). Passing
 * target by value is inferred: the constructor only copies it and the caller destroys the
 * temporary, which a const reference would also produce. */
class TouchHangCommand : public PlayerCommand {
  public:
    TouchHangCommand(boost::weak_ptr<CharInfo> target, const Fn8040Vector3 *position,
                     const Fn8040Vector3 *angles, int arg3, int arg4);
    virtual ~TouchHangCommand();

    /* 0x14 */ boost::weak_ptr<CharInfo> m_target;
    /* 0x1C */ const Fn8040Vector3 *field_0x1C;
    /* 0x20 */ const Fn8040Vector3 *field_0x20;
    /* 0x24 */ int field_0x24;
};
CHECK_SIZEOF(Player::TouchHangCommand, 0x28);

namespace Npc {

/* Class and base names come from RTTI; argument meanings are inferred by callers. */
class EnemyDeadByPlayerCommand : public PlayerCommand {
  public:
    EnemyDeadByPlayerCommand(int arg0, int arg1);
    virtual ~EnemyDeadByPlayerCommand();
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
};
CHECK_SIZEOF(EnemyDeadByPlayerCommand, 0x1C);

} // namespace Npc

} // namespace Player

#endif
