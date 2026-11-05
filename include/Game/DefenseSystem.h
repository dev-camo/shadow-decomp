#ifndef GAME_DEFENSESYSTEM_H
#define GAME_DEFENSESYSTEM_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

/* The state that the owner stores for the gadget (class name from RTTI, derived from
 * SetAdapterState; member names inferred). It can survive from one creation of the gadget to the
 * next, but not always: when bit 0x40000000 of the owner's record is set, fn_800C9BE8 (called by
 * Gadget's destructor) destroys and clears it. */
class DefenseSystemSetState : public SetAdapterState {
  public:
    /* m_state values (names inferred from what each state does). The constructor of DefenseSystem
     * passes STATE_NONE as the state it leaves. */
    enum { STATE_NONE, STATE_INTACT, STATE_DAMAGED, STATE_DESTROYED };

    DefenseSystemSetState() {
        Reset();
    }

    virtual ~DefenseSystemSetState() {}

    /* Restores the intact state and the full health (name inferred). */
    void Reset() {
        m_state = STATE_INTACT;
        m_health = 50.0f;
    }

    /* 0x4 */ s32 m_state; /* the gadget's state, a STATE_ value */
    /* 0x8 */ float m_health;
};
CHECK_SIZEOF(DefenseSystemSetState, 0xC);

/* DefenseSystem (class name from RTTI; the gadget's descriptor is "DEFENSE_SYSTEM"): a gadget that
 * shows the DEFENSESYSTEM.BIN model turning about its z axis, with a Target at its position, four
 * more Targets around it and a second object (fn_802B4254) that the constructor places above the
 * gadget's position (ResetFromOwner does not move it). Its state and health are stored in a
 * DefenseSystemSetState that the owner holds. Outside the destroyed state, attacks subtract their
 * value from the health, 50 at first, and OnAttack queues the transitions in m_attackFlags: the
 * destruction flag when the health is below zero or nearly zero (fn_8000A950); otherwise, in
 * the intact state with no destruction queued, the damage flag when it is below 10 or nearly
 * equal to 10 (fn_801C455C). UpdateBody acts on them when it next runs (Gadget::Update skips it
 * once IsFinished is true). It tests the destruction flag first and clears both flags there, so a
 * hit that uses up the health before the body runs skips the damaged stage. For damage it shows
 * the first break model and enters the damaged state. For destruction it shows the second break
 * model, passes its com id to fn_8016907C, calls UnkSlot7(1, 0) on the actor only when the weak
 * handle that fn_800740E4 returns locks, passes its owner to fn_800C9B04, enters the destroyed
 * state and finishes (behaviour inferred). Method names marked "(name inferred)" are not original;
 * all member names are inferred. */
class DefenseSystem : public Gadget {
  public:
    DefenseSystem(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DefenseSystem();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, its rotation converted from degrees to radians (fn_800C9ED4)
     * and its com id (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation, turned by m_angle about the
     * z axis, the Target at the position and the four Targets around it (name inferred). */
    void UpdateModelMatrix();
    /* Leaves state previous and enters state next (DefenseSystemSetState's STATE_ values), then
     * stores next in the set state. Only STATE_INTACT and STATE_DAMAGED select a model entry (0 and
     * 1) and create the part target; for them OnAttack is registered with the entry's attack object
     * only when that object's weak handle locks. Entering another state only stores it, after the
     * work for the state it leaves (name inferred). */
    void ChangeState(int previous, int next);
    /* The attack object's callback (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ std::vector<boost::shared_ptr<Target> > m_objects; /* around the gadget */
    /* 0x4C */ boost::shared_ptr<Target> m_object;                /* at the gadget's position */
    /* 0x54 */ boost::shared_ptr<UnkObject_802B4254> m_object2;   /* placed above it at first */
    /* 0x5C */ boost::shared_ptr<UnkTarget_80166AF4> m_target;    /* the model's part target */
    /* 0x64 */ DefenseSystemSetState *m_setState;                 /* the owner's state object */
    /* Transitions that OnAttack queues for UpdateBody (inferred): bit 0 damage (health below or
     * nearly equal to 10), bit 1 destruction (health below or nearly equal to zero). */
    /* 0x68 */ UnkBitFlags_80014E04 m_attackFlags;
    /* 0x6C */ u8 m_comId;    /* the owner's com id (fn_800C9E60) */
    /* 0x70 */ float m_angle; /* the model's turn about the z axis, 12 degrees a second */
    /* 0x74 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(DefenseSystem, 0x90);

#endif
