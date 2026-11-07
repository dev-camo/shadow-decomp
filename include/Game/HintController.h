#ifndef GAME_HINTCONTROLLER_H
#define GAME_HINTCONTROLLER_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Player {
class PlayerBase;
namespace Npc {
class Npc;
}
} // namespace Player

/* A 0xC-byte entry of the stage's hint file (placeholder name): the descriptor's first hook loads
 * stg<stage>/stg<stage>_hint.bin and keeps the first array of the file's table, and
 * HintController steps through its entries until one whose message id is -1. */
struct UnkHintEntry_802F2F5C {
    /* 0x0 */ s32 messageId;
    /* A negative value takes the branch from the word at 0x20 of fn_8006D6A8's manager. */
    /* 0x4 */ s32 branch;
    /* The time HintController waits before requesting the message, in the units of dt. */
    /* 0x8 */ float delay;
};
CHECK_SIZEOF(UnkHintEntry_802F2F5C, 0xC);

namespace HintController {

/* HintController::HintController (class name from RTTI, with Task and Gadget as its bases; the
 * gadget's descriptor is "HINT CONTROLLER"): requests the messages of the stage's hint file in
 * turn. In state 0 it takes the next entry: one with message id -1 moves it to state 2, any other
 * starts the entry's delay in state 1. In state 1 it counts the delay down by dt; when the
 * remaining time is 0 or less (not when it is NaN) it requests the entry's message through
 * fn_801FAF00 and returns to state 0. When the branch is negative it passes the branch 0, 1 or 2
 * equal to the word at 0x20 of fn_8006D6A8's manager; for any other value of that word it makes no
 * request but still returns to state 0. State 2 finishes the gadget (fn_8015A794 and Task flag
 * 0x01). Member names and the names of the non-virtual methods are inferred. */
class HintController : public Gadget {
  public:
    HintController(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~HintController();
    virtual void UpdateBody(float dt);
    /* Empty. */
    virtual void fn_800CBC1C(float dt);
    /* Empty. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, then sets the state,
     * the timer, the entry and the entry index to 0 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ int m_state;
    /* The index of the next entry. */
    /* 0x3C */ int m_index;
    /* 0x40 */ const UnkHintEntry_802F2F5C *m_entry;
    /* The remaining delay of m_entry. */
    /* 0x44 */ float m_timer;
};
CHECK_SIZEOF(HintController, 0x48);

} // namespace HintController

/* Hint requests made by player code (names unknown; signatures, parameter names and the reference
 * spellings inferred). Each one makes at most one request: it passes a message id and a branch to
 * a file-local function that calls fn_801FAF00 with them and 1 only when the id is not negative.
 * The rows are indexed by the word at 0xAC of lbl_8057D6A0; the npc is what
 * boost::dynamic_pointer_cast to Player::Npc::Npc gives for the locked player, and the branch is
 * fn_801FAAC4 of its fn_80073A08 unless stated otherwise. None of them checks the result of the
 * cast. */
extern "C" {
/* Only when (count - 1) % 3 is 0: requests the npc type's first id plus ((count - 1) / 3) % 3
 * from a table of 20 ids indexed by the npc's word at 0x64, or nothing when that entry is
 * negative. For type 9 the offset grows by 3 for kind 0 when the stage (the word at 0xA8 of
 * lbl_8057D6A0) is 504 and by 6 for kind 2; for the other types by 6 for kind 1 and by 3 for
 * kind 2. */
void fn_802F28A8(const boost::shared_ptr<Player::Npc::Npc> &npc, int kind, int count);
/* Unless the weak pointer has expired: requests message 441 when fn_8040EE90's random value is at
 * most chance, otherwise the id at 0xC of the row with the index that fn_8040EA90(4) returns. */
void fn_802F299C(const boost::weak_ptr<Player::PlayerBase> &player, float chance);
/* As fn_802F299C without a player: the branch is fn_801FAAC4(1), and the random index
 * comes from fn_8040EA90(2) instead of fn_8040EA90(4) when the byte at 0xB1 of lbl_8057D6A0 is
 * set. */
void fn_802F2A9C(float chance);
/* Unless the weak pointer has expired: requests the row's id at 0x1C. */
void fn_802F2B40(const boost::weak_ptr<Player::PlayerBase> &player);
/* Unless the weak pointer has expired: requests the row's id at index n of the first three, where
 * n is the npc's fn_800736FC result, replaced by 2 unless it is less than 2 (no lower limit). */
void fn_802F2C1C(const boost::weak_ptr<Player::PlayerBase> &player);
}

#endif
