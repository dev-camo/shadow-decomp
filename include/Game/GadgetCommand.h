#ifndef GAME_GADGETCOMMAND_H
#define GAME_GADGETCOMMAND_H

#include "Game/CharCommand.h"
#include "layout_check.h"

/* Commands of the GadgetCommand namespace (class names from RTTI). Only the classes that the
 * reconstructed code reads are declared; their constructors and destructors are defined
 * elsewhere, together with the vtables. GadgetCommand::AttackEndCommand (id 0x304, constructor
 * 0x80273948) is not declared because nothing reconstructed reads its members. */
namespace GadgetCommand {

/* GadgetCommand::GadgetCommand (class name from RTTI, with CharCommand as its only base). Its
 * constructor (0x8025E758) truncates id to 16 bits (clrlwi), passes both arguments on to
 * CharCommand's and sets its vtable; the type of arg1 is inferred. */
class GadgetCommand : public CharCommand {
  public:
    GadgetCommand(int id, int arg1);
    virtual ~GadgetCommand();
};
CHECK_SIZEOF(GadgetCommand, 0x14);

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
