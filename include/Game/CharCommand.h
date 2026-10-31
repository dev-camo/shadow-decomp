#ifndef GAME_CHARCOMMAND_H
#define GAME_CHARCOMMAND_H

#include <dolphin/types.h>
#include "layout_check.h"

/* CharCommand (class name from RTTI): the base of the commands that are sent to character infos.
 * The layout comes from its constructor (members not mapped). Its operator new only calls
 * fn_80049B90 with the size and 2 (purpose unknown). The deleting destructors call its class
 * operator delete(void *, size_t), fn_801D9C64, which is not declared here. The vtable has three
 * more virtual functions after the destructor, which are not declared here because nothing in the
 * reconstructed code calls them; Player::ElectricCircuitCommand overrides the first of them. */
class CharCommand {
  public:
    CharCommand(u16 id, int arg1);
    virtual ~CharCommand();

    static void *operator new(size_t size);

    /* 0x04 */ u8 field_0x4[0x14 - 0x4];
};
CHECK_SIZEOF(CharCommand, 0x14);

namespace Player {

/* Player::PlayerCommand (class name from RTTI): a command with an id, which sets its vtable. */
class PlayerCommand : public CharCommand {
  public:
    PlayerCommand(int id, int arg1);
    virtual ~PlayerCommand();
};
CHECK_SIZEOF(Player::PlayerCommand, 0x14);

} // namespace Player

#endif
