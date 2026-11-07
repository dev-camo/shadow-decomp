#ifndef GAME_SUPERCOMPUTER_H
#define GAME_SUPERCOMPUTER_H

#include "Game/CharaColliAttack.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkResource_80012700;

namespace SuperComputer {

/* SuperComputer::SetSuperComputer (class name from RTTI): a gadget, in the namespace
 * SuperComputer, that shows the SUPERCOMPUTER.BIN model with a second object (fn_802B4254) and a
 * Target above it. An attack subtracts its value from a health of 50. Below 37.5, 25 and 12.5
 * the gadget enters damage stages 0 to 2; at 0 or below it enters stage 3, which passes
 * fn_8015D93C a parameter block built from the crash information's table entry and selects the
 * model entry that the crash information names. On the next update it calls fn_8016B224(1, 0) on
 * the mission that fn_800740E4 returns for 0 when it locks to a non-null pointer, passes the
 * owner's id to fn_8016907C, passes its owner on through fn_8015A768 and hands the Target to
 * fn_801E7108 (behaviour inferred). Method and member names are inferred. */
class SetSuperComputer : public Gadget {
  public:
    SetSuperComputer(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetSuperComputer();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget and restores the full health
     * (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation, and the second object and the Target above it (name inferred). */
    void UpdateModelMatrix();
    /* The model attack object's callback (name inferred). In states 1 to 4 and while
     * m_recoveryTime is not positive, an attack whose attack object has one of the bits 0x3D0
     * set in its word at 0x40 subtracts the attack's value (the word at 0x50 of the parameter)
     * from the health; the callback then reports 1, or 3 once the health is used up, and 5
     * otherwise. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* True in states 1 to 4, the states in which attacks can damage the gadget (name inferred). */
    bool IsDamageable() const;
    /* Stage 0 creates the part target (m_target) for the model; stages 1 and 2 pass it the
     * previous stage's value (25, 50) through fn_80166E20 and every stage passes the next one
     * (25, 50, 75, -1 for stage 3) through fn_80167180. Stage 3 first calls StartCrashEffect,
     * selects the model entry named by the crash information and creates a new part target.
     * Every stage plays sound 0x7843 and sets flag bit 1 (name inferred). */
    void EnterDamageStage(int stage);
    /* Passes fn_8015D93C a parameter block with the clump that the crash information's table
     * entry names, at the character info's position plus the entry's offset (name inferred). */
    void StartCrashEffect();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x50 */ boost::shared_ptr<Target> m_object;
    /* 0x58 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* 0x5C */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0 after construction or a reset, 1 to 4 while attacks can damage the gadget (one more for
     * each damage stage entered), 5 once the health is used up, 6 when finished (inferred). */
    /* 0x64 */ s32 m_state;
    /* 0x68 */ float m_health;
    /* 0.2 after a hit, counted down by dt; attacks are ignored while it is positive. */
    /* 0x6C */ float m_recoveryTime;
    /* Bit 0: set once the gadget is destroyed, bit 1: set by every damage stage and cleared on
     * destruction; each update plays sound 0x7845 while bit 1 is set and 0x785F while bit 0 is
     * clear (inferred). Hides Task::m_flags. */
    /* 0x70 */ UnkBitFlags_80014E04 m_flags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetSuperComputer, 0x78);

} // namespace SuperComputer

#endif
