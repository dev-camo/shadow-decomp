#ifndef GAME_DEFENSESHIELD_H
#define GAME_DEFENSESHIELD_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The state that the owner keeps for a defense shield (class name from RTTI, derived from
 * SetAdapterState): the health that attacks use up, 25 when new (member name inferred). It can
 * survive from one creation of the shield to the next, but not always: when bit 0x40000000 of the
 * owner's record is set, fn_800C9BE8 (called by Gadget's destructor) destroys and clears it. */
class DefenseShieldSetState : public SetAdapterState {
  public:
    DefenseShieldSetState() : m_health(25.0f) {}
    virtual ~DefenseShieldSetState() {}

    /* 0x04 */ float m_health;
};
CHECK_SIZEOF(DefenseShieldSetState, 0x8);

/* DefenseShield (class name from RTTI; the gadget's descriptor is "DEFENSE_SHIELD"): the gadget
 * of a shield (model file DEFENSESHIELD.BIN) whose health is kept in a DefenseShieldSetState that
 * the owner keeps. Attacks take their value off the health in states 1 and 2. A health below 0
 * or within 0.0001 of 0, or an entry of 1 for its com id in the fn_800CB69C manager, requests
 * state 3. An attack that leaves the health above that requests state 2 only when the shield
 * is in state 1, no request for state 3 is pending and the health is below 5 or within 0.0001 of
 * it. UpdateBody carries out the requests, state 3 first. The constructor makes the same tests to
 * choose the first state. States 1, 2 and 3 pass 0, 1 and 2 to the model's fn_80162A68
 * (behaviour inferred). Method and member names marked "(name inferred)" are not original. */
class DefenseShield : public Gadget {
  public:
    DefenseShield(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DefenseShield();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the com id from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model and the object at 0x40 at the character info (name inferred). */
    void UpdateModelMatrix();
    /* For previousState 1 to 3, releases the target and calls the model's fn_80162804 and
     * fn_80161880; then sets up state and stores it in m_state (name inferred). */
    void ChangeState(int previousState, int state);
    /* The attack object's callback (behaviour inferred): sets the result at 0x54 to 1 and, in
     * states 1 and 2, takes the attack's value at 0x50 off the kept health. When the health is
     * below 0 or within 0.0001 of 0, the result becomes 3, the attacker's character info (if any)
     * goes to fn_8020642C with 0x48 and the callback requests state 3. Otherwise, in state 1 with
     * no request for state 3 pending, a health below 5 or within 0.0001 of it requests state 2,
     * and a value of 1 or more plays the sound 0x7733. UpdateBody carries out the requests when
     * it runs; a request for state 3 overrides one for state 2. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* The names of the members below are inferred. */
    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_object;
    /* Present in states 1 and 2. */
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* The owner's state object, read back with dynamic_cast by the constructor. */
    /* 0x50 */ DefenseShieldSetState *m_setState;
    /* 1 intact, 2 damaged, 3 broken (meanings inferred); 0 until the constructor sets one. */
    /* 0x54 */ s32 m_state;
    /* Requests that UpdateBody carries out when it runs (Gadget::Update skips it while
     * IsFinished returns true): bit 0 for state 2, bit 1 for state 3. Bit 1 takes priority and
     * clears both. */
    /* 0x58 */ UnkBitFlags_80014E04 m_flags;
    /* 0x5C */ u8 m_comId; /* the owner's com id (fn_800C9E60) */
};
CHECK_SIZEOF(DefenseShield, 0x60);

#endif
