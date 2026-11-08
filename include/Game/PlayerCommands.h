#ifndef GAME_PLAYERCOMMANDS_H
#define GAME_PLAYERCOMMANDS_H

#include "Game/CharCommand.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class Target;

/* Commands that gadgets send to character infos (class names from RTTI, with CharCommand,
 * Player::PlayerCommand or Player::Npc::NpcCommand as their only base); WallJumpCommand is sent by
 * the player and received by gadgets. Each class declares the overrides of CharCommand's three
 * virtual functions that its original vtable holds, under CharCommand's placeholder names and with
 * the USA address of the override in a comment; the overrides are defined elsewhere. The
 * constructors and destructors are defined elsewhere too, together with the vtables, except the
 * constructors of CancelBehaviorCommand, AddWeaponCommand, SetVelocityCommand,
 * IgnoreInputCommand(int, float), InvincibleCommand, SwitchCommand and Player::HomingJumpCommand
 * (ItemBox.cpp), which gadget files define, and the constructors of
 * Player::StartExternalControlCommand and Player::TouchHangCommand, which are defined in their
 * classes here (see the classes); the argument meanings are inferred from the stores in the
 * constructors.
 * SetPositionCommand, SetVelocityCommand and LaunchCommand pass their last argument to the base
 * constructor as its second argument, and ClrVelocityCommand its only argument;
 * IgnoreInputCommand passes 0. Most callers pass 4 there (0 and 2 also occur). */

/* Moves the character info to a position. */
class SetPositionCommand : public CharCommand {
  public:
    SetPositionCommand(const Fn8040Vector3 *position, int arg1);
    virtual ~SetPositionCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x801DBD84 */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x801DBD10 */
    virtual const char *fn_801D9CA8();                            /* 0x801DBD04 */

    /* 0x14 */ Fn8040Vector3 m_position;
};
CHECK_SIZEOF(SetPositionCommand, 0x20);

/* Sets the velocity of the character info. Its constructor is defined in Tornado.cpp, which holds
 * the copy that the original keeps (see the note there). */
class SetVelocityCommand : public CharCommand {
  public:
    SetVelocityCommand(const Fn8040Vector3 *velocity, int arg1);
    virtual ~SetVelocityCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x801DBA50 */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x801DB9DC */
    virtual const char *fn_801D9CA8();                            /* 0x801DB9D0 */

    /* 0x14 */ Fn8040Vector3 m_velocity;
};
CHECK_SIZEOF(SetVelocityCommand, 0x20);

/* Clears the velocity of the character info. */
class ClrVelocityCommand : public CharCommand {
  public:
    ClrVelocityCommand(int arg1);
    virtual ~ClrVelocityCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x801DBAE4 */
    virtual const char *fn_801D9CA8();                            /* 0x801DBAD8 */
};
CHECK_SIZEOF(ClrVelocityCommand, 0x14);

/* The command with the id 1. Its constructor (0x801DBE84) receives a vector by address (the bytes
 * do not show whether as a pointer or a reference) and the base constructor's second argument,
 * and copies the vector to 0x14. The Coaster gadget scales the roll of its ride by the length of
 * the vector and its direction. */
class AccelerateInputCommand : public CharCommand {
  public:
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DBE10 */
    virtual const char *fn_801D9CA8();        /* 0x801DBE04 */

    /* 0x14 */ Fn8040Vector3 m_vector;
};
CHECK_SIZEOF(AccelerateInputCommand, 0x20);

/* The command with the id 0x17. Its constructor (0x801DAB3C) passes 0x17 and 4 to the base
 * constructor and stores a byte, a float and a word. The member names are inferred from the
 * FootingBreak gadget, which passes the float to its model's fn_8015FDD8 when the byte is set
 * and calls fn_8015FD10 otherwise; the word is not used there. The FootingRoll gadget makes the
 * same choice by testing whether the float is below 1 and reads neither the byte nor the word. */
class SetTransparentCommand : public CharCommand {
  public:
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DAAB8 */
    virtual const char *fn_801D9CA8();        /* 0x801DAAAC */

    /* 0x14 */ u8 m_enable;
    /* 0x18 */ float m_alpha;
    /* 0x1C */ int field_0x1C;
};
CHECK_SIZEOF(SetTransparentCommand, 0x20);

/* Accelerates the character info (the constructor at 0x8006E354 passes 6 as the command id and
 * copies the vector to 0x14). The ElecFan gadget sends it to push the character info it blows.
 * The vector is taken by const reference (inferred): where ElecFan scales a vector for it, the
 * original builds the scaled vector into an argument temporary after the allocation, as a
 * reference bound to a by-value expression would. */
class AccelerateCommand : public CharCommand {
  public:
    AccelerateCommand(const Fn8040Vector3 &acceleration, int arg1);
    virtual ~AccelerateCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x801DB948 */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x801DB8D4 */
    virtual const char *fn_801D9CA8();                            /* 0x801DB8C8 */

    /* 0x14 */ Fn8040Vector3 m_acceleration;
};
CHECK_SIZEOF(AccelerateCommand, 0x20);

/* The command with the id 0xD: its constructors (0x801DB41C and 0x801DB480) pass 0xD and 4 to the
 * base constructor and copy a weak pointer to a character info to 0x14; the first copies its
 * second argument to the weak pointer at 0x1C, the second leaves that one empty. The creators
 * allocate 0x24 bytes. The SET_WEAPON gadget's weapon hands itself to that character info (name
 * inferred: the character that uses the target). */
class UseCommand : public CharCommand {
  public:
    /* The second constructor (0x801DB480; the const reference is inferred). The ROCKET gadget's
     * rocket sends it to its base's character info with its own character info as user. */
    UseCommand(const boost::weak_ptr<CharInfo> &user);
    /* Defined elsewhere (0x801DC484): destroys the weak pointers at 0x1C and 0x14 and releases
     * the command through fn_801D9C64 with the size 0x24 (behaviour inferred from the bytes). */
    virtual ~UseCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB414 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_user;
    /* The RING gadget locks it with the lock at 0x80057778, whose shared pointers are Target
     * pointers (it constructs them with Target's shared_ptr constructors), and compares the result
     * with its rings' targets. */
    /* 0x1C */ boost::weak_ptr<Target> field_0x1C;
};
CHECK_SIZEOF(UseCommand, 0x24);

/* Sets the rotation of the character info (inferred from the name): the constructor at
 * 0x800B81F8 passes 3 as the command id and its last argument to the base constructor, copies the
 * vector to 0x14 and stores order at 0x20. The vector is taken by const reference (inferred): the
 * Spring gadget builds it into an argument temporary after the allocation, from fn_8040F364's
 * result. The DashPanel and DashRing gadgets pass the angles that fn_8040F5F4 returns for order 2,
 * and 2, and the Spring gadget passes 2 as well. */
class SetAngleCommand : public CharCommand {
  public:
    SetAngleCommand(const UnkRotation3 &angles, int order, int arg1);
    virtual ~SetAngleCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x801DBBFC */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x801DBB70 */
    virtual const char *fn_801D9CA8();                            /* 0x801DBB64 */

    /* 0x14 */ UnkRotation3 m_angles;
    /* 0x20 */ int m_order;
};
CHECK_SIZEOF(SetAngleCommand, 0x24);

/* The command with the id 0x1D. Its constructor (0x801DA868) takes no arguments and passes 0x1D and
 * 4 to the base constructor. The LongSpring and Spring gadgets send it first to a character they
 * launch. */
class SpringCommand : public CharCommand {
  public:
    SpringCommand();
    virtual ~SpringCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DA804 */
};
CHECK_SIZEOF(SpringCommand, 0x14);

/* The command with the id 8. Its constructor (0x8009EE8C) only passes 8 and its argument to the
 * base constructor and sets the vtable; the objects are 0x14 bytes. The PARK_BALLOON balloons send
 * it, with 4, to their generator's character info when they burst, and the generator counts the
 * commands with this id. */
class DeadCommand : public CharCommand {
  public:
    DeadCommand(int arg1);
    virtual ~DeadCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB7F4 */
};
CHECK_SIZEOF(DeadCommand, 0x14);

/* The command with the id 10. Its constructor (0x801DB71C) passes 10 and 4 to the base
 * constructor, copies a weak pointer to a character info to 0x14 and a vector to 0x1C. The
 * player's wall hang behaviour (0x80091F58) sends it with a weak pointer that fn_800920CC builds
 * from the member at 0x260 of the object its behaviour keeps at 0x4; the BreakWall gadget breaks
 * when it receives it once cracked and keeps that character info. The vector is not read there. */
class WallJumpCommand : public CharCommand {
  public:
    /* Defined elsewhere (USA 0x801DC5D4): destroys the weak pointer at 0x14 and releases the
     * command through fn_801D9C64 with size 0x28 (behaviour inferred from the bytes). */
    virtual ~WallJumpCommand();
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DB6A8 */
    virtual const char *fn_801D9CA8();        /* 0x801DB69C */

    /* 0x14 */ boost::weak_ptr<CharInfo> field_0x14;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
};
CHECK_SIZEOF(WallJumpCommand, 0x28);

/* VacuumCommand (class name from RTTI, with CharCommand as its only base): the command with the id
 * 0x14. Its constructor (0x801DB0D8) passes 0x14 and 4 to the base constructor, copies its first
 * two arguments to the weak pointers at 0x14 and 0x1C, default-constructs the weak pointer at 0x24
 * and stores its third argument at 0x2C; the creators allocate 0x30 bytes. The other constructor
 * (0x801DB004, which fn_8034085C calls at 0x80340984) also copies a weak pointer to 0x14, but
 * leaves the weak pointer at 0x1C empty, copies a weak pointer to a character info to 0x24 and
 * stores 1 at 0x2C. The ITEM_BOX gadget and VacuumObject's constructor (0x80238424), which reads
 * the members at 0x1C, 0x24 and 0x2C, take it; the RING gadget locks the weak pointer at 0x14 and
 * collects the first of its rings with bit 1 set whose target equals the locked pointer. */
class VacuumCommand : public CharCommand {
  public:
    /* Defined elsewhere (USA 0x801DC178). */
    virtual ~VacuumCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DAFFC */

    /* Copied with 0x8005FDB4, which copies UseCommand's field_0x1C. Holds a Target: it is locked
     * with the lock at 0x80057778 (see UseCommand::field_0x1C). Name inferred: the target being
     * vacuumed. */
    /* 0x14 */ boost::weak_ptr<Target> m_target;
    /* A weak pointer (copied with 0x801DB14C) whose pointee type is not established. */
    /* 0x1C */ u8 field_0x1C[0x24 - 0x1C];
    /* 0x24 */ boost::weak_ptr<CharInfo> field_0x24;
    /* 0x2C */ s32 field_0x2C;

  private:
    /* Copying is disabled: field_0x1C holds a reference-counted weak pointer as bytes. */
    VacuumCommand(const VacuumCommand &);
    VacuumCommand &operator=(const VacuumCommand &);
};
CHECK_SIZEOF(VacuumCommand, 0x30);

namespace Player {

/* Makes the player ignore input for a time (the first member holds the time; the gadgets that
 * send it pass a time in seconds). The constructor that takes the time sets the word at 0x18 to 0
 * and stores its integer at 0x1C; it is defined in Tornado.cpp, which holds the copy that the
 * original keeps (see the note there). */
class IgnoreInputCommand : public PlayerCommand {
  public:
    IgnoreInputCommand(int arg0, float seconds);
    /* The second constructor (0x800774C4) stores arg0 at 0x18 and arg1 at 0x1C and leaves the time
     * unset (the callers pass 1 to 3 and 0 or 1; meanings unknown). */
    IgnoreInputCommand(int arg0, int arg1);
    virtual ~IgnoreInputCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009A77C */

    /* 0x14 */ float m_seconds;
    /* 0x18 */ u32 field_0x18;
    /* 0x1C */ int m_arg0;
};
CHECK_SIZEOF(IgnoreInputCommand, 0x20);

/* Launches the player (the first argument is stored at 0x14; its meaning is unknown). */
class LaunchCommand : public PlayerCommand {
  public:
    LaunchCommand(int arg0, int arg1);
    virtual ~LaunchCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009ADD8 */

    /* 0x14 */ int m_arg0;
};
CHECK_SIZEOF(LaunchCommand, 0x18);

/* The constructor at 0x8009AD8C passes 0x122 as the command id and stores arg0 at 0x14, widened
 * to a word. The ElecFan gadget sends it with true when it starts to blow a character info and
 * with false when it lets go of it (meaning inferred from that use). */
class StreamCommand : public PlayerCommand {
  public:
    StreamCommand(bool arg0, int arg1);
    virtual ~StreamCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009ACD8 */

    /* 0x14 */ int m_arg0;
};
CHECK_SIZEOF(StreamCommand, 0x18);

/* The command that the Coaster gadget sends to the character it carries. The constructor
 * passes 0x112 as the command id and arg4 as the base constructor's second argument, stores a
 * copy of target at 0x14, the two vector pointers at 0x1C and 0x20 and arg3 at 0x24; the meaning
 * of the vectors and numbers is not known (the Coaster gadget passes its position and the angles
 * of its turned matrix). The constructor is defined in the class: its only copy (0x80115B70) is
 * emitted after the ROCKET gadget's collision callback, the last function in that file to
 * construct one; the Coaster and UD_REEL gadgets, which come later in the link, call that copy.
 * Passing target by value is inferred: the constructor only copies it and the caller destroys the
 * temporary, which a const reference would also produce. */
class TouchHangCommand : public PlayerCommand {
  public:
    TouchHangCommand(boost::weak_ptr<CharInfo> target, const Fn8040Vector3 *position,
                     const UnkRotation3 *angles, int arg3, int arg4)
        : PlayerCommand(0x112, arg4), m_target(target), field_0x1C(position), field_0x20(angles),
          field_0x24(arg3) {}
    virtual ~TouchHangCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009BD80 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_target;
    /* 0x1C */ const Fn8040Vector3 *field_0x1C;
    /* 0x20 */ const UnkRotation3 *field_0x20;
    /* 0x24 */ int field_0x24;
};
CHECK_SIZEOF(TouchHangCommand, 0x28);

/* The command that the SET_WEAPON gadget sends to the character that takes its weapon (id 0x11A).
 * The constructor (0x8011F478, in the SET_WEAPON gadget's file) passes 0x11A and arg2 to the base
 * constructor and stores the weapon type and arg1 (the weapon item's word at 0xC, meaning unknown);
 * the destructor and the vtable are elsewhere. */
class AddWeaponCommand : public PlayerCommand {
  public:
    AddWeaponCommand(int weaponType, int arg1, int arg2);
    virtual ~AddWeaponCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009B70C */

    /* 0x14 */ int m_weaponType;
    /* 0x18 */ int field_0x18;
};
CHECK_SIZEOF(AddWeaponCommand, 0x1C);

/* The constructor at 0x80099BFC passes 0x134 as the command id and 4 to the base constructor,
 * stores the float at 0x14 and copy-constructs the vector at 0x18. The vector is passed
 * by address (the bytes do not show whether as a pointer or a reference; a pointer is declared).
 * The DashPanel and JumpBoard gadgets send it with a direction scaled by a value of their level
 * and their "nocontrol time (sec)" parameter (meanings inferred from that use). */
class ActingAccelerateCommand : public PlayerCommand {
  public:
    ActingAccelerateCommand(const Fn8040Vector3 *vector, float seconds);
    virtual ~ActingAccelerateCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x80099B18 */

    /* 0x14 */ float m_seconds;
    /* 0x18 */ Fn8040Vector3 m_vector;
};
CHECK_SIZEOF(ActingAccelerateCommand, 0x24);

/* The constructor at 0x8009A1E0 passes 0x12E as the command id and its argument to the base
 * constructor; it stores nothing else. */
class PressGroundCommand : public PlayerCommand {
  public:
    PressGroundCommand(int arg1);
    virtual ~PressGroundCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009A11C */
};
CHECK_SIZEOF(PressGroundCommand, 0x14);

/* The command with the id 0x101. Its destructor (0x8009DBB4) and vtable are defined elsewhere; the
 * constructor passes 0x101 and its argument to the base constructor and is defined in
 * DashPanel.cpp, which holds the copy that the original keeps (see the note there). The Spring
 * gadget sends it with 4 to the character it launches. */
class CancelBehaviorCommand : public PlayerCommand {
  public:
    CancelBehaviorCommand(int arg1);
    virtual ~CancelBehaviorCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009CB90 */
};
CHECK_SIZEOF(CancelBehaviorCommand, 0x14);

/* The command with the id 0x12B (class name from the RTTI of its vtable at 0x80520F74); the
 * destructor and the vtable are elsewhere. The first constructor below passes 0x12B and arg4 to the
 * base constructor, copies target to 0x14 and stores the two vector pointers and arg3. It is
 * defined in the class: its only copy (0x8014D730) sits among the functions that the NOBI_KUSA
 * gadget's file instantiates where its hit callback first uses them. That gadget sends it with the
 * point and the angles it carries the character at (meaning inferred from that use). Passing target
 * by value is inferred, as for TouchHangCommand. The second constructor (0x8009E8D8) passes 0x12B
 * and arg1 to the base constructor, default-constructs the weak pointer at 0x14, zeroes the words
 * at 0x1C and 0x20 and stores arg0 at 0x24. The ELEC_LIGHT gadget sends it with 0 and 2 to the
 * character info it catches and Player::EndExternalControlCommand when it lets it go (meanings
 * inferred from the names). */
class StartExternalControlCommand : public PlayerCommand {
  public:
    StartExternalControlCommand(boost::weak_ptr<CharInfo> target, const Fn8040Vector3 *position,
                                const UnkRotation3 *angles, int arg3, int arg4)
        : PlayerCommand(0x12B, arg4), m_target(target), m_position(position), m_angles(angles),
          field_0x24(arg3) {}
    StartExternalControlCommand(int arg0, int arg1);
    virtual ~StartExternalControlCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009A414 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_target;
    /* 0x1C */ const Fn8040Vector3 *m_position;
    /* 0x20 */ const UnkRotation3 *m_angles;
    /* 0x24 */ int field_0x24;
};
CHECK_SIZEOF(StartExternalControlCommand, 0x28);

/* The command with the id 0x129 (class name from the RTTI of its vtable at 0x80520FCC). The
 * constructor at 0x8009E880 passes 0x129 and arg2 to the base constructor and stores arg0 at 0x14
 * and the float at 0x18; the bytes do not show where the float stands in the parameter list (it
 * arrives in f1). The NOBI_KUSA gadget sends it with 3, 0.0f and 4 and the ELEC_LIGHT gadget with
 * 0x26, 0.0f and 2 (meanings unknown). */
class SetMotionCommand : public PlayerCommand {
  public:
    SetMotionCommand(int arg0, float arg1, int arg2);
    virtual ~SetMotionCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009A6EC */

    /* 0x14 */ int field_0x14;
    /* 0x18 */ float field_0x18;
};
CHECK_SIZEOF(SetMotionCommand, 0x1C);

namespace Npc {

/* Player::Npc::NpcCommand (class name from RTTI, with CharCommand as its only base). Its
 * constructor (0x800734E4) passes both arguments on to CharCommand's and sets its vtable. It
 * passes id on without the clrlwi that an int parameter compiles to (Player::PlayerCommand's
 * constructor at 0x8006EE30 has one), so id is u16 like CharCommand's; the type of arg1 is
 * inferred. */
class NpcCommand : public CharCommand {
  public:
    NpcCommand(u16 id, int arg1);
    virtual ~NpcCommand();
};
CHECK_SIZEOF(NpcCommand, 0x14);

/* Player::Npc::EnemyDeadByPlayerCommand (class name from RTTI, with Player::Npc::NpcCommand as its
 * base). Its constructor (0x80072FF4) passes 0x403 and 4 to the base constructor and stores arg0
 * at 0x14 and arg1 at 0x18 (parameter types inferred; argument meanings inferred from the
 * callers). */
class EnemyDeadByPlayerCommand : public NpcCommand {
  public:
    EnemyDeadByPlayerCommand(int arg0, int arg1);
    virtual ~EnemyDeadByPlayerCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x80072EE4 */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x80072E68 */
    virtual const char *fn_801D9CA8();                            /* 0x80072ED8 */
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
};
CHECK_SIZEOF(EnemyDeadByPlayerCommand, 0x1C);

/* Player::Npc::ShowCommand (class name from RTTI, with Player::Npc::NpcCommand as its base). Its
 * constructor (0x800732D0) passes 0x402 and arg1 to the base constructor and stores arg0 at 0x14
 * (parameter types inferred). The SET_CHARACTOR gadget sends it with 0 and 4 (meanings
 * unknown). */
class ShowCommand : public NpcCommand {
  public:
    ShowCommand(int arg0, int arg1);
    virtual ~ShowCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x80073050 */
    virtual const char *fn_801D9CA8();                            /* 0x80073048 */

    /* 0x14 */ int field_0x14;
};
CHECK_SIZEOF(ShowCommand, 0x18);

} // namespace Npc

/* The constructor at 0x8009E614 passes 0x12C as the command id and its argument to the base
 * constructor; it stores nothing else. */
class EndExternalControlCommand : public PlayerCommand {
  public:
    EndExternalControlCommand(int arg1);
    virtual ~EndExternalControlCommand();
};
CHECK_SIZEOF(EndExternalControlCommand, 0x14);

/* The constructor at 0x8010DFD0 passes 0x10B as the command id and arg1 to the base constructor and
 * copy-constructs the vector at 0x14. The vector is taken by const reference (inferred):
 * the ELEC_LIGHT gadget builds it into an argument temporary after the allocation. */
class HomingJumpCommand : public PlayerCommand {
  public:
    HomingJumpCommand(const Fn8040Vector3 &vector, int arg1);
    virtual ~HomingJumpCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009C4FC */

    /* 0x14 */ Fn8040Vector3 m_vector;
};
CHECK_SIZEOF(HomingJumpCommand, 0x20);

} // namespace Player

/* RideCommand (class name from RTTI, with CharCommand as its only base). Its constructor
 * (0x801DB638) passes id 11 and 4 to the base constructor, copies rider to 0x14 and
 * copy-constructs the vector at 0x1C. The ELEC_BAR gadget's bars keep the character info at 0x14
 * of the last one they receive as their rider (meanings inferred from that use). Passing rider by
 * const reference is inferred. */
class RideCommand : public CharCommand {
  public:
    RideCommand(const boost::weak_ptr<CharInfo> &rider, const Fn8040Vector3 *vector);
    virtual ~RideCommand();
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DB5C4 */
    virtual const char *fn_801D9CA8();        /* 0x801DB5BC */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_rider;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
};
CHECK_SIZEOF(RideCommand, 0x28);

namespace Player {

/* The command that the ELEC_BAR gadget's bars and the CATCHSTICK gadget send to a player whose
 * movement comes near their line (meaning inferred from those uses). The constructor at 0x800F618C
 * (defined in CatchStick.cpp) passes 0x111 as the command id and arg2 to the base constructor and
 * copies the two weak pointers to 0x14 and 0x1C. Passing them by value follows the bars' call,
 * which constructs a temporary for each. */
class TouchLineCommand : public PlayerCommand {
  public:
    TouchLineCommand(boost::weak_ptr<CharInfo> charInfo, boost::weak_ptr<UnkSegment_80020A20> line,
                     int arg2);
    virtual ~TouchLineCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009BF04 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_charInfo;
    /* 0x1C */ boost::weak_ptr<UnkSegment_80020A20> m_line;
};
CHECK_SIZEOF(TouchLineCommand, 0x24);

/* The constructor at 0x80079484 passes 0x106 as the command id and its argument to the base
 * constructor; it stores nothing else. The ELEC_BAR gadget's bars send it with 4 to their rider
 * when they hide. */
class StartActionCommand : public PlayerCommand {
  public:
    StartActionCommand(int arg1);
    virtual ~StartActionCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009C7CC */
};
CHECK_SIZEOF(StartActionCommand, 0x14);

/* The command with the id 0x126 (class name from the RTTI of its vtable at 0x80521024); the
 * destructor and the vtable are elsewhere. The constructor, defined in Tornado.cpp, passes 0x126
 * and arg1 to the base constructor and stores the bool at 0x14. The Tornado gadget's towing task
 * sends it with true when it takes a character and with false when it lets it go (meaning
 * inferred from that use and the class name). */
class InvincibleCommand : public PlayerCommand {
  public:
    InvincibleCommand(bool enable, int arg1);
    virtual ~InvincibleCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009A8F8 */

    /* 0x14 */ bool m_enable;
};
CHECK_SIZEOF(InvincibleCommand, 0x18);

/* Player::StartJumpCommand (class name from the RTTI of its vtable at 0x805214F4). The constructor
 * at 0x8009C994 passes 0x104 as the command id and its argument to the base constructor and clears
 * the vector at 0x14 with fn_8040CE60. The GunLift vehicle sends it with 4 to the character info
 * at 0x80 of the vehicle. */
class StartJumpCommand : public PlayerCommand {
  public:
    StartJumpCommand(int arg1);
    virtual ~StartJumpCommand();
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver); /* 0x8009C850 */
    virtual UnkString_8004F888 fn_801D9CB0();                     /* 0x8009C7DC */

    /* 0x14 */ Fn8040Vector3 field_0x14;
};
CHECK_SIZEOF(StartJumpCommand, 0x20);

} // namespace Player

/* SwitchCommand (class name from the RTTI of its vtable at 0x8053D404, with CharCommand as its
 * only base); the destructor and the vtable are elsewhere. The constructor, defined in
 * RailSwitch.cpp, passes 9 and 4 to the base constructor and stores its argument at 0x14. The RAIL
 * SWITCH gadget allocates one with its "route" parameter and passes it to a character info (a null
 * pointer when the allocation fails); the command handler fn_8025E9C0 uses the word at 0x14 as an
 * index into the table at 0x8054B8E0 under the conditions given in Game/UnkRailRoute_8054B8E0.h
 * (the member name is inferred from these uses). */
class SwitchCommand : public CharCommand {
  public:
    SwitchCommand(s32 route);
    virtual ~SwitchCommand();
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DB780 */
    virtual const char *fn_801D9CA8();        /* 0x801DB7EC */

    /* 0x14 */ s32 m_route;
};
CHECK_SIZEOF(SwitchCommand, 0x18);

/* ControlVehicleCommand (class name from the RTTI of its vtable at 0x8053D254, with CharCommand as
 * its only base); the constructor, the destructor and the vtable are elsewhere. The constructor
 * (0x801DADB4) passes 0x15 as the command id and its second argument to the base constructor and
 * stores its first argument at 0x14. The GunLift vehicle's command handler switches on that word
 * and acts on the values 1, 2, 3, 4, 6 and 7. */
class ControlVehicleCommand : public CharCommand {
  public:
    virtual UnkString_8004F888 fn_801D9CB0(); /* 0x801DAD4C */
    virtual const char *fn_801D9CA8();        /* 0x801DAD40 */

    /* 0x14 */ int m_control;
};
CHECK_SIZEOF(ControlVehicleCommand, 0x18);

/* DamageCommand (class name from the RTTI of its vtable at 0x8053D44C, with CharCommand as its
 * only base); the destructor and the vtable are elsewhere. The constructor at 0x801DB864 passes 7
 * as the command id and arg1 to the base constructor, stores the float at 0x14 and copy-constructs
 * the zero vector lbl_805E07A0 at 0x18. The float arrives in f1, so the bytes do not
 * show where it stands in the parameter list (order inferred). The WEIGHT gadget sends it with
 * 1.0f and 4 to the characters it catches (meanings inferred from that use and the class name). */
class DamageCommand : public CharCommand {
  public:
    DamageCommand(float arg0, int arg1);
    /* The constructor at 0x801DB804 does the same, but copy-constructs vector at 0x18 instead of
     * the zero vector. The GunLift vehicle sends it with 1.0f, (0, 1, 0) and 4. The order of the
     * float and the other parameters and the reference spelling are inferred. */
    DamageCommand(float arg0, const Fn8040Vector3 &vector, int arg1);
    virtual ~DamageCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB7FC */

    /* 0x14 */ float field_0x14;
    /* 0x18 */ Fn8040Vector3 field_0x18;
};
CHECK_SIZEOF(DamageCommand, 0x24);

/* ScoreCommand (class name from the RTTI of its vtable at 0x8053D158, with CharCommand as its only
 * base); the constructor, the destructor and the vtable are elsewhere. The constructor
 * (0x801DA8B4) passes 28 (0x1C) and 4 to the base constructor and stores its five arguments at
 * 0x14 to 0x24 in order. The TurnOver gadget's command handler passes a copy of each ScoreCommand
 * it receives to the character info of the user it recorded from a UseCommand (while that user is
 * recorded and still exists), with the five words in the same order; their meanings are unknown.
 * The argument types are inferred from those 32-bit copies. */
class ScoreCommand : public CharCommand {
  public:
    ScoreCommand(int arg0, int arg1, int arg2, int arg3, int arg4);
    virtual ~ScoreCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DA8AC */

    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ int field_0x20;
    /* 0x24 */ int field_0x24;
};
CHECK_SIZEOF(ScoreCommand, 0x28);

/* ReadyCommand (class name from the RTTI of its vtable at 0x8053D350, with CharCommand as its
 * only base). Its constructor (0x801DB3C0) passes id 0xE and 4 to the base constructor and copies
 * a weak pointer to a character info to 0x14; the objects are 0x1C bytes. The ROCKET gadget's
 * rocket sends it to its base's character info, with its own character info, when its flight
 * ends (a base in its launched state then builds the next rocket; meaning of the name inferred
 * from that use). The const reference is inferred. */
class ReadyCommand : public CharCommand {
  public:
    ReadyCommand(const boost::weak_ptr<CharInfo> &sender);
    /* Defined elsewhere (0x801DC418). */
    virtual ~ReadyCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB3B8 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_sender;
};
CHECK_SIZEOF(ReadyCommand, 0x1C);

/* PickupCommand (class name from the RTTI of its vtable at 0x8053D2E4, with CharCommand as its only
 * base): the command with the id 0x11. Its constructor (0x801DB26C) passes 0x11 and 4 to the base
 * constructor, copies a vector to 0x14, stores a float at 0x20, copies a weak pointer to a
 * character info to 0x24 and a second vector to 0x2C. The HEALING_UNIT gadget's unit keeps the
 * weak pointer as the character that picked it up (role inferred). */
class PickupCommand : public CharCommand {
  public:
    /* Defined elsewhere (0x801DC2D4). */
    virtual ~PickupCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB264 */

    /* 0x14 */ Fn8040Vector3 field_0x14;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ boost::weak_ptr<CharInfo> m_picker;
    /* 0x2C */ Fn8040Vector3 field_0x2C;
};
CHECK_SIZEOF(PickupCommand, 0x38);

/* CastCommand (class name from the RTTI of its vtable at 0x8053D2C0, with CharCommand as its only
 * base): the command with the id 0x12. Its constructor (0x801DB200) passes 0x12 and 4 to the base
 * constructor and copies a weak pointer to a character info to 0x14 and a vector to 0x1C. */
class CastCommand : public CharCommand {
  public:
    /* Defined elsewhere (0x801DC268). */
    virtual ~CastCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB1F8 */

    /* 0x14 */ boost::weak_ptr<CharInfo> field_0x14;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
};
CHECK_SIZEOF(CastCommand, 0x28);

/* ThrowCommand (class name from the RTTI of its vtable at 0x8053D29C, with CharCommand as its only
 * base): the command with the id 0x13. Its constructor (0x801DB194) passes 0x13 and 4 to the base
 * constructor and copies a weak pointer to a character info to 0x14 and a vector to 0x1C. */
class ThrowCommand : public CharCommand {
  public:
    /* Defined elsewhere (0x801DC1FC). */
    virtual ~ThrowCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB18C */

    /* 0x14 */ boost::weak_ptr<CharInfo> field_0x14;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
};
CHECK_SIZEOF(ThrowCommand, 0x28);

/* RecoverCommand (class name from the RTTI of its vtable at 0x8053D308, with CharCommand as its
 * only base). Its constructor (0x801DB2F8) passes 0x10 and 4 to the base constructor, copies a
 * weak pointer to a character info to 0x14 and stores amount at 0x1C; the HEALING_UNIT gadget's
 * blast allocates 0x20 bytes for it. The parameter names and the const reference are inferred. */
class RecoverCommand : public CharCommand {
  public:
    RecoverCommand(const boost::weak_ptr<CharInfo> &sender, float amount);
    /* Defined elsewhere (0x801DC340). */
    virtual ~RecoverCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DB2F0 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_sender;
    /* 0x1C */ float m_amount;
};
CHECK_SIZEOF(RecoverCommand, 0x20);

/* BindCommand (class name from the RTTI of its vtable at 0x8053D1A0, with CharCommand as its only
 * base); the constructor, the destructor and the vtable are elsewhere. The constructor
 * (0x801DA984) passes id 0x1A and 4 to the base constructor and copies a weak pointer to a
 * character info to 0x14; the CAGE gadget allocates 0x1C bytes and passes its own character
 * info. The const reference and the member name are inferred. */
class BindCommand : public CharCommand {
  public:
    BindCommand(const boost::weak_ptr<CharInfo> &sender);
    /* Defined elsewhere (0x801DBF9C). */
    virtual ~BindCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DA97C */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_sender;
};
CHECK_SIZEOF(BindCommand, 0x1C);

/* ReleaseCommand (class name from the RTTI of its vtable at 0x8053D17C, with CharCommand as its
 * only base); the constructor, the destructor and the vtable are elsewhere. The constructor
 * (0x801DA928) passes id 0x1B and 4 to the base constructor and copies a weak pointer to a
 * character info to 0x14; the CAGE gadget allocates 0x1C bytes and passes its own character
 * info. The const reference and the member name are inferred. */
class ReleaseCommand : public CharCommand {
  public:
    ReleaseCommand(const boost::weak_ptr<CharInfo> &sender);
    /* Defined elsewhere (0x801DBF30). */
    virtual ~ReleaseCommand();
    virtual const char *fn_801D9CA8(); /* 0x801DA920 */

    /* 0x14 */ boost::weak_ptr<CharInfo> m_sender;
};
CHECK_SIZEOF(ReleaseCommand, 0x1C);

#endif
