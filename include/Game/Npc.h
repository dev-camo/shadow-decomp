#ifndef GAME_NPC_H
#define GAME_NPC_H

#include "Game/PlayerBase.h"
#include "layout_check.h"

namespace Player {
namespace Npc {

/* Player::Npc::Npc (class name from RTTI, with CharInfo, Player::PlayerObject and
 * Player::PlayerBase as its bases). This is a view: only the methods that the SET_CHARACTOR
 * gadget and the HintController file call are declared. They are defined with the class
 * elsewhere and named after their addresses; each one reaches the object that the pointer at
 * 0x290 refers to. That member, its type and the class's full size are not recovered. */
class Npc : public PlayerBase {
  public:
    /* Returns UnkBitFlags_80014E04::Test(0), called with the pointer at 0x290 as the flags
     * object (behaviour inferred from its body; bool inferred from the caller, which tests the
     * low byte). */
    bool fn_80073708();
    /* Stores value in the float at 0x10 of the object at 0x290; when the word at 0x64 is 0xD it
     * stores 0.0f instead (behaviour inferred from its body). */
    void fn_80073770(float value);
    /* Stores value in the float at 0xC of the object at 0x290 (behaviour inferred from its
     * body). */
    void fn_80073838(float value);
    /* Return the word at 0x4 (fn_80073A08) and at 0x18 (fn_800736FC) of the object that the
     * pointer at 0x290 refers to (behaviour from their bodies; the int returns are inferred: the
     * HintController file passes the first to fn_801FAAC4 and compares the second as signed). */
    int fn_80073A08();
    int fn_800736FC();
};
/* Checks only the declared view (the PlayerBase part); the full size is unknown. */
CHECK_SIZEOF(Npc, 0x290);

} // namespace Npc
} // namespace Player

#endif
