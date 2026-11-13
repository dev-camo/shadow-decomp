#ifndef GAME_GADGETCOMMAND_H
#define GAME_GADGETCOMMAND_H

#include "Game/CharCommand.h"
#include "layout_check.h"

/* Commands of the GadgetCommand namespace (class names from RTTI). Only the classes that the
 * reconstructed code uses are declared. Unless a class says otherwise, its constructor, destructor
 * and overrides of CharCommand's virtual functions are defined elsewhere, together with the vtable
 * (the comment after an override gives its USA address). GadgetCommand::AttackEndCommand (id
 * 0x304, constructor 0x80273948) is not declared because nothing reconstructed reads its
 * members. */
namespace GadgetCommand {

/* GadgetCommand::GadgetCommand (class name from RTTI, with CharCommand as its only base). Its
 * constructor truncates id to 16 bits (clrlwi), passes both arguments on to CharCommand's and
 * sets its vtable; the type of arg1 is inferred. The constructor is defined in the class
 * (inferred): the original's one copy (0x8025E758) lies among the functions of the E_P_COLI
 * gadget's file, between the constructors of MoveEndCommand and MoveStartCommand, which call it,
 * far from the destructor (0x8015D6AC) and fn_801D9CA8, which are defined out of line with the
 * vtable. It is not expanded in them, because it calls CharCommand's inline constructor (see
 * MoveStartCommand for where those callers are emitted). */
class GadgetCommand : public CharCommand {
  public:
    GadgetCommand(int id, int arg1) : CharCommand(id, arg1) {}
    virtual ~GadgetCommand();
    virtual const char *fn_801D9CA8(); /* 0x8015D708 */
};
CHECK_SIZEOF(GadgetCommand, 0x14);

/* GadgetCommand::MoveStartCommand and GadgetCommand::MoveEndCommand (class names from RTTI, with
 * GadgetCommand::GadgetCommand as their base): the commands with the ids 0x301 and 0x302. Each
 * constructor (0x8025E798 and 0x8025E704) passes its id and arg1 to the base constructor and
 * stores the float at 0x14; the order of the float and integer parameters is not visible in the
 * bytes and is inferred. The destructors (0x8025E8C8 and 0x8025E924) only run the base's, and the
 * deleting forms free 0x18 bytes through CharCommand's operator delete. The vtables (0x8054B880
 * and 0x8054B8AC) hold no other functions of their own. The E_P_COLI and BK_TANK_COLI gadgets
 * send the two commands with the fifth of their six parameters as the float, for their types
 * "Move" and "Stop".
 *
 * The constructors are defined in the classes and the destructors are implicit (both inferred):
 * the original has one copy of each constructor, destructor and vtable, among the functions and
 * data of the E_P_COLI gadget's file, and the BK_TANK_COLI file holds a place for the two vtables
 * in its .data. A unit that creates the commands therefore emits weak copies of the
 * constructors, destructors and vtables, which the linker discards in favour of the E_P_COLI
 * file's. A unit that only casts to the classes keeps its own RTTI records of them, as the file
 * of the BK_TANK controller's command handler (0x80321388) does (0x805ED2F4 and 0x805ED2FC).
 *
 * MoveEndCommand's member name is inferred from that handler, which copies the float into the
 * controller's float at 0x40 (0x80321438), which the controller's update function (0x80321690)
 * counts down to 0, and sets bit 0 of the controller's flags when the float is <= 0 (an ordered
 * comparison, so a NaN does not set it). The BK_TANK_COLI gadget labels the parameter it sends
 * "sec" when it is > 0 and "no timer" otherwise. The handler does not read MoveStartCommand's
 * float, and no other receiver is reconstructed: its name is a placeholder. */
class MoveStartCommand : public GadgetCommand {
  public:
    MoveStartCommand(float value, int arg1) : GadgetCommand(0x301, arg1), field_0x14(value) {}

    /* 0x14 */ float field_0x14;
};
CHECK_SIZEOF(MoveStartCommand, 0x18);

class MoveEndCommand : public GadgetCommand {
  public:
    MoveEndCommand(float waitTime, int arg1) : GadgetCommand(0x302, arg1), m_waitTime(waitTime) {}

    /* 0x14 */ float m_waitTime;
};
CHECK_SIZEOF(MoveEndCommand, 0x18);

/* GadgetCommand::AttackStartCommand (class name from RTTI, with GadgetCommand::GadgetCommand as its
 * base). Its constructor (0x8027399C) passes 0x303 and arg1 to the base constructor and stores
 * kind at 0x14 and the two floats at 0x18 and 0x1C. The order of the integer and float parameters
 * is not visible in the bytes and is inferred, and so are the member names (from the EGG_BALLOON
 * gadget's OnCommand, which selects its firing by the kind, adds or stores the time and stores the
 * interval). */
class AttackStartCommand : public GadgetCommand {
  public:
    AttackStartCommand(int kind, float time, float interval, int arg1);
    virtual ~AttackStartCommand();

    /* 0x14 */ int m_kind;
    /* 0x18 */ float m_time;
    /* 0x1C */ float m_interval;
};
CHECK_SIZEOF(AttackStartCommand, 0x20);

} // namespace GadgetCommand

#endif
