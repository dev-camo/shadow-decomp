#ifndef GAME_CHARCOMMAND_H
#define GAME_CHARCOMMAND_H

#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class UnkString_8004F888;

namespace boost {
template <class T> class weak_ptr;
} // namespace boost

/* CharCommand (class name from RTTI): the base of the commands that are sent to character infos.
 * The layout comes from its constructor (0x8006E454): it stores the id at 0x4 and its second
 * argument at 0x8, constructs the member at 0xC (fn_8006E4A8) and clears the bytes at 0x6 and 0x7;
 * only the id and the byte at 0x7 are named. Its operator new only calls fn_80049B90 with the
 * size and 2 (purpose unknown). The deleting destructors call its class operator delete(void *,
 * size_t), fn_801D9C64, which is not declared here. */
class CharCommand {
  public:
    CharCommand(u16 id, int arg1);
    virtual ~CharCommand();
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

    /* 0x04 */ u16 m_type; /* the id (handlers switch on it) */
    /* 0x06 */ u8 field_0x6;
    /* 0x07 */ u8 m_flags; /* handlers set bit 1 (meaning unknown) */
    /* 0x08 */ u8 field_0x8[0x14 - 0x8];
};
CHECK_SIZEOF(CharCommand, 0x14);

namespace Player {

/* Player::PlayerCommand (class name from RTTI): a command with an id, which sets its vtable. */
class PlayerCommand : public CharCommand {
  public:
    PlayerCommand(int id, int arg1);
    virtual ~PlayerCommand();
    virtual const char *fn_801D9CA8(); /* 0x8009CCE4 */
};
CHECK_SIZEOF(PlayerCommand, 0x14);

} // namespace Player

#endif
