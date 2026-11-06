#ifndef GAME_BREAKOBJ_H
#define GAME_BREAKOBJ_H

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

/* BreakObj (class name from RTTI; the gadget's descriptor is "BRK_OBJ"): a breakable object of
 * the BREAKOBJ.BIN model whose "kind" parameter selects the model entry; OnAttack names kinds 0 and
 * 1. The constructor reads the kind (m_kind has no other initial value, so when the owner's
 * parameter size is not 4 it keeps what the allocation held), sets up the model entry of that
 * kind and, when attaching the character info to that entry's part (fn_80161FC8) returns true
 * and the part's attack object still exists, registers OnAttack with that attack object. The
 * constructor does not check the model pointer before its own calls: a failed GadgetModelInfo
 * allocation stores null and the constructor still calls the model's fn_80162A68. The first
 * update after construction switches the state from 0 to 1; OnAttack switches an accepted attack's
 * state 1 to 2, and the first update whose state switch then reads state 2 starts the break
 * effect, passes the owner to fn_800C9B34 and sets Task flag 0x01 (behaviour inferred). Each
 * update runs the model update before that switch. All member names, and the method names marked
 * "(name inferred)", are not original; the parameters' pointer, reference and const spellings are
 * inferred. */
class BreakObj : public Gadget {
  public:
    BreakObj(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BreakObj();
    /* Advances the model when there is one (fn_80162FB0), then reads the state: state 0 switches
     * to state 1; state 2 calls StartBreakEffect, passes the owner to fn_800C9B34 (the owner
     * pointer is not checked) and sets Task flag 0x01; the other states do nothing. The model
     * update runs before the state switch (name inferred). */
    virtual void UpdateBody(float dt);
    /* ReadOwner and UpdateModelMatrix, then nothing more without a model. When the kind read
     * differs from m_loadedKind and the table that fn_80162A68 searches has no entry of that kind
     * (fn_80162EA8), it returns without calling the model's fn_8016108C. When there is such an
     * entry, it stores the kind in m_loadedKind, calls fn_80161880 and fn_80162804 and sets up the
     * new kind's entry as the constructor does (registering OnAttack under the same conditions).
     * With an unchanged kind, or after that setup, it calls the model's fn_8016108C. The state is
     * left unchanged (name inferred). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 when its allocation succeeds (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info (whose pointer is not
     * checked), then copies the kind parameter into m_kind only when the owner's parameter size
     * is 4 (fn_800C9DBC; otherwise m_kind keeps its value) (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, places it at the character info's position, turned by the y angle
     * of its rotation (name inferred). */
    void UpdateModelMatrix();
    /* The callback in the second callback member (0x1C) of the attack object of the kind's part
     * (name inferred). In state 1, an attack whose attack object still exists and has bit 1 of
     * its word at 0x40 set gets the result 5. Any other attack whose attack object still exists
     * passes that object's owner, when it still exists, to fn_8020642C with 0x14 for kind 0 or
     * 0x15 for kind 1 (no call for another kind), gets the result 3 and switches the state to 2.
     * Without an attack object, or outside state 1, the result is 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Does nothing without a model or without a model entry 1000 plus the kind (fn_8015F7BC).
     * Otherwise passes fn_8015D93C a parameter block built from the character info with 1.3 at
     * 0x34 and, at 0x44, the clump that fn_80012700 returns for the file the entry names (not
     * checked). When that file name is BRKOBJGUNCRSH.DFF it then passes sound 0x101C and the
     * character info's position to fn_801D54F4; for any other name sound 0x101D, and then
     * fn_8015D794 gets the character info's position raised by 10 on the y axis and 0xB9 (an
     * effect, inferred; the returned weak pointer is discarded) (name inferred). */
    void StartBreakEffect();

    /* The values of m_state (names inferred). */
    enum State { STATE_RESET = 0, STATE_WAITING = 1, STATE_HIT = 2 };

    static const char *s_name;

    /* The kind whose model entry the constructor or ResetFromOwner last set up (name inferred). */
    /* 0x30 */ s32 m_loadedKind;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model;
    /* 0x40 */ s32 m_kind; /* the "kind" parameter */
    /* STATE_RESET after construction, STATE_WAITING from the first update, STATE_HIT after an
     * accepted attack. */
    /* 0x44 */ s32 m_state;
};
CHECK_SIZEOF(BreakObj, 0x48);

#endif
