#ifndef GAME_CHARCOMMAND_H
#define GAME_CHARCOMMAND_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommandUnit;
class CharInfo;
class UnkString_8004F888;

/* CharCommand (class name from RTTI): the base of the commands that are sent to character infos.
 * Its operator new only calls fn_80049B90 with the size and 2 (purpose unknown). The deleting
 * destructors call its class operator delete(void *, size_t) (0x801D9C64) with the size of their
 * class.
 * The constructor and the destructor are defined in the class. The original has one copy of each
 * (0x8006E454 and 0x8006E3A4), emitted where an inline function's out-of-line copy goes: after
 * AccelerateCommand's constructor (0x8006E354), the first function in the link that calls the
 * constructor, the destructor's copy first, each followed by the copy of the same function of the
 * weak pointer at 0xC (0x8006E404 and 0x8006E4A8). Neither is expanded at its uses: each calls
 * the weak pointer's inline constructor or destructor. */
class CharCommand {
  public:
    CharCommand(u16 id, int arg1) : m_type(id), field_0x8(arg1) {
        m_flags = 0;
        field_0x6 = 0;
    }
    virtual ~CharCommand() {}
    /* The three slots after the destructor, named after CharCommand's implementations (their
     * names and meanings are not known; the derived command classes declare the overrides that
     * their original vtables hold). The first is empty here (0x801D9CFC); the EggLastMech boss's
     * command handler calls it with a weak pointer to the boss when bit 1 of m_flags is clear. The
     * second (0x801D9CB0) returns a UnkString_8004F888 built from the third's result. The third
     * (0x801D9CA8) returns a string from .sdata2; the boss's handler prints it with the command id
     * when bit 1 is still clear after that call. The parameter and return types are inferred from
     * those uses. */
    virtual void fn_801D9CFC(boost::weak_ptr<CharInfo> receiver);
    virtual UnkString_8004F888 fn_801D9CB0();
    virtual const char *fn_801D9CA8();

    static void *operator new(size_t size);
    static void operator delete(void *command, size_t size);

    /* 0x04 */ u16 m_type; /* the id (handlers switch on it) */
    /* 0x06 */ u8 field_0x6;
    /* 0x07 */ u8 m_flags; /* handlers set bit 1 (meaning unknown) */
    /* The constructor's second argument. The command manager's fn_801D9F3C uses it as the index
     * of the vector of commands that it appends the command to; fn_801DA204 goes through seven
     * such vectors. */
    /* 0x08 */ int field_0x8;
    /* The command unit that receives the command (name inferred). fn_801D9F3C assigns it from the
     * shared pointer that it is given with the command (Player::PlayerBase's fn_801DC980 passes
     * the unit at its 0x21C through fn_801D9C0C). For a command with bit 2 of m_flags set and
     * bit 0 clear, fn_801DA204 locks it and, if the unit still exists, passes the command to the
     * unit's handler wrapper, which calls the callback only when one is set (0x8019FF04-0x8019FF14;
     * a new unit's handler is empty). */
    /* 0x0C */ boost::weak_ptr<CharCommandUnit> m_unit;
};
CHECK_SIZEOF(CharCommand, 0x14);

namespace Player {

/* Player::PlayerCommand (class name from RTTI): a command with an id, which sets its vtable. The
 * constructor and the destructor are defined in the class, as CharCommand's are: the original's
 * only copies (0x8006EE30 and 0x8006EDD4, the destructor's first) follow the command constructor
 * at 0x8006ED90, the first function in the link that calls the constructor. id is an int here: the
 * constructor narrows it for CharCommand's (a clrlwi). */
class PlayerCommand : public CharCommand {
  public:
    PlayerCommand(int id, int arg1) : CharCommand(id, arg1) {}
    virtual ~PlayerCommand() {}
    virtual const char *fn_801D9CA8(); /* 0x8009CCE4 */
};
CHECK_SIZEOF(PlayerCommand, 0x14);

} // namespace Player

#endif
