#ifndef GAME_PLAYEROPTION_H
#define GAME_PLAYEROPTION_H

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

namespace Player {
class PlayerBase;
} // namespace Player

/* The 0x18-byte object that the pointer at 0x254 of a Player::PlayerBase refers to (placeholder
 * name; it has no vtable and so no name in the type records). It holds a shared pointer to the
 * player and two character infos that its constructor creates with it: a Player::InternalOption
 * at index 0 and a Player::ExternalOption at index 1 (class names from RTTI; both are defined in
 * PlayerOption.cpp). PlayerBase's fn_80079AB0 creates it (USA (GUPE8P) 0x80079AE8-0x80079B1C)
 * and the function at 0x80078B54 deletes it (0x80078BF0-0x80078C08). The member functions keep
 * placeholder names. */
class UnkPlayerOptions_800AAF64 {
  public:
    UnkPlayerOptions_800AAF64(const boost::shared_ptr<Player::PlayerBase> &player);
    ~UnkPlayerOptions_800AAF64();

    /* Copies the player's position, rotation, the vectors at 0x18 and 0x24, the word at 0x30 and
     * the pointer at 0x60 to both options. */
    void fn_800AAE14();
    /* Returns a weak pointer to the option at index (not range-checked). */
    boost::weak_ptr<CharInfo> fn_800AADA4(int index);
    /* Calls the virtual fn_8007A4CC of both options with arg0; neither option class overrides
     * CharInfo's, which stores it at 0x54. The one caller is 0x80079904. */
    void fn_800AAD44(int arg0);

    /* 0x00 */ boost::shared_ptr<Player::PlayerBase> m_player; /* name inferred */
    /* 0x08 */ boost::shared_ptr<CharInfo> m_options[2];       /* name inferred */
};
CHECK_SIZEOF(UnkPlayerOptions_800AAF64, 0x18);

#endif
