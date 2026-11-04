#ifndef GAME_MAGMAGUARD_H
#define GAME_MAGMAGUARD_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace MagmaGuard {

/* MagmaGuard::SetMagmaGuard (class name from RTTI; the gadget's descriptor is "MAGMA GUARD"): a
 * gadget, in the namespace MagmaGuard, that shows the MAGMAGUARD.BIN model with a second object
 * (fn_802B4254) above it. An attack subtracts its value from a health of 12; when the health
 * falls to 10.8, 6 and 0 or below, the part target (m_target) is passed the selectors 1, 2 and 3
 * through fn_80167010 (the last two after the previous selector through fn_80166E20). Once the
 * health is used up, the next update sets the state of the gadget's com id to 1 (fn_801690D0),
 * calls UnkSlot7(1, 0) on the actor that fn_800740E4 returns for 0 and sets bit 0x400000 of the
 * owner's record; a gadget whose owner has that bit set starts destroyed (behaviour inferred).
 * Method and member names are inferred. */
class SetMagmaGuard : public Gadget {
  public:
    SetMagmaGuard(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetMagmaGuard();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget, restores the full health
     * and clears the flags (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's matrix and the second object above the character
     * info's position (name inferred). */
    void UpdateModelMatrix();
    /* The model attack object's callback (name inferred). In state 1 and while m_recoveryTime is
     * not positive, an attack whose attack object has one of the bits 0x3D1 set in its word at
     * 0x40 subtracts the attack's value (the float at 0x50 of the parameter) from the health; the
     * callback then reports 1, and 5 otherwise. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0 after construction or a reset, 1 while attacks can damage the gadget, 2 once the health
     * is used up, 3 when destroyed (inferred). */
    /* 0x48 */ s32 m_state;
    /* 0x4C */ s32 m_comId; /* the owner's com id (fn_800C9E50) */
    /* 0x50 */ float m_health;
    /* 0.5 after a hit that leaves health, counted down by dt; attacks are ignored while it is
     * positive. */
    /* 0x54 */ float m_recoveryTime;
    /* 0x58 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* Bits 0 to 2: set when the health falls to 10.8, 6 and 0 or below; bit 3: set when the
     * health is used up or the gadget starts destroyed, after which each update plays sound
     * 0x784C (inferred). Hides Task::m_flags. */
    /* 0x60 */ UnkBitFlags_80014E04 m_flags;
    /* 0x64 */ u8 field_0x64[0x4]; /* in the allocation size (0x68); no code here touches it */
};
CHECK_SIZEOF(SetMagmaGuard, 0x68);

} // namespace MagmaGuard

#endif
