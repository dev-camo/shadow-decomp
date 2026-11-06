#ifndef GAME_BLASTBREAK_H
#define GAME_BLASTBREAK_H

#include "Game/Gadget.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class AttackCallbackParam;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

/* BlastBreak (class name from RTTI; the gadget's descriptor is "BREAK_BLAST"): a breakable object
 * of the BLASTBREAK.BIN model. The constructor registers OnAttack with the attack object of the
 * model's part "col" when attaching the character info to that part (fn_80161F08) returns true and
 * the attack object still exists. ReadOwner sets the state to STATE_RESET; the first update after
 * that switches it to STATE_WAITING. OnAttack switches an accepted attack's STATE_WAITING to
 * STATE_HIT, and the first update whose state switch then reads STATE_HIT passes the owner to
 * fn_800C9B34 when there is one (fn_8015A794) and sets Task flag 0x01 (behaviour inferred). Each
 * update advances the model when there is one (fn_80162FB0) before that switch. Member names, and
 * the method names marked "(name inferred)", are not original; the parameters' pointer, reference
 * and const spellings are inferred. */
class BlastBreak : public Gadget {
  public:
    BlastBreak(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BlastBreak();
    /* Advances the model when there is one (fn_80162FB0), then reads the state: STATE_RESET
     * switches to STATE_WAITING, STATE_HIT passes the owner to fn_800C9B34 when there is one and
     * sets Task flag 0x01, and the other states do nothing. The model update runs before the
     * state switch. */
    virtual void UpdateBody(float dt);
    /* Calls ReadOwner (which sets STATE_RESET) and UpdateModelMatrix, then the model's fn_8016108C;
     * the model pointer is not checked. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 when its allocation succeeds (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info (whose pointer is not
     * checked) and sets the state to STATE_RESET (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, passes it the character info's matrix from fn_8015CEA0 (name
     * inferred). */
    void UpdateModelMatrix();
    /* The callback in the second callback member (0x1C) of the attack object of the part "col"
     * (name inferred). In STATE_WAITING, an attack whose attack object still exists and has bit
     * 0x10 of its word at 0x40 set passes that object's owner to fn_8020642C with 0x17 when the
     * owner still exists, then, with or without that call, gets the result 3 (param's word at
     * 0x54) and switches the state to STATE_HIT; when that bit is clear, the result is 5. Without
     * an attack object, or outside STATE_WAITING, the result is 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* The values of m_state (names inferred, as in BreakObj, whose state machine is the same). */
    enum State { STATE_RESET = 0, STATE_WAITING = 1, STATE_HIT = 2 };

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* STATE_RESET after ReadOwner, STATE_WAITING from the next update, STATE_HIT after an
     * accepted attack. */
    /* 0x40 */ s32 m_state;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BlastBreak, 0x48);

#endif
