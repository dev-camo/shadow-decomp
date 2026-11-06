#ifndef GAME_NPC_H
#define GAME_NPC_H

#include "Game/PlayerBase.h"
#include "layout_check.h"

namespace Player {
namespace Npc {

/* Player::Npc::Npc (class name from RTTI, with CharInfo, Player::PlayerObject and
 * Player::PlayerBase as its bases). This is a view: only the methods that the SET_CHARACTOR
 * gadget calls are declared. They are defined with the class elsewhere and named after their
 * addresses; each one reaches the object that the pointer at 0x290 refers to. That member, its
 * type and the class's full size are not recovered. */
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
};
/* Checks only the declared view (the PlayerBase part); the full size is unknown. */
CHECK_SIZEOF(Npc, 0x290);

} // namespace Npc
} // namespace Player

#endif
