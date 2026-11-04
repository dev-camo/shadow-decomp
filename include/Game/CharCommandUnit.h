#ifndef GAME_CHARCOMMANDUNIT_H
#define GAME_CHARCOMMANDUNIT_H

#include "Game/CharCommand.h"
#include "boost/function.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The handler of the commands sent to a character info (name inferred). */
typedef boost::function1<void, CharCommand &> CharCommandHandler;
CHECK_SIZEOF(CharCommandHandler, 0xC);

/* CharCommandUnit (class name from the RTTI of the vtable at 0x8053D118): the 0x10-byte object
 * that a GadgetCharInfo holds in a shared pointer at 0x64. Its constructor at 0x801DA780 sets the
 * vtable and initialises the handler at 0x4. The whole object is mapped; the last word of the
 * vtable (after the destructor) is not explained. */
class CharCommandUnit {
  public:
    virtual ~CharCommandUnit();

    /* The command manager's dispatch loop (fn_801DA204) calls it with each queued command; the
     * Coaster, FootingBreak and BombServer gadgets bind their command handlers to it (name
     * inferred). */
    /* 0x04 */ CharCommandHandler m_handler;
};
CHECK_SIZEOF(CharCommandUnit, 0x10);

#endif
