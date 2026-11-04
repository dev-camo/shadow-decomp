#ifndef GAME_BASHIELD_H
#define GAME_BASHIELD_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace BAShield {

/* BAShield::BAShield (class name from RTTI): a gadget that shows the BASHIELD.BIN model and turns
 * it on and off with the switch of its com id. With the parameter "type" 0 it is on while the
 * switch is off, with any other type while the switch is on (behaviour inferred). Method names
 * marked "(name inferred)" are not original. */
class BAShield : public Gadget {
  public:
    BAShield(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BAShield();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameter and com id into the gadget and clears its
     * state and flags (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by the character
     * info's rotation (name inferred). */
    void UpdateModelMatrix();
    /* Replaces the target object, if there is one, and calls fn_80167448 on the new one; calls
     * fn_80161230 on the model and plays sound 0x793C (name inferred). */
    void TurnOff();
    /* Replaces the target object, if there is one, and calls fn_80167010 on the new one; calls
     * fn_8016111C on the model and plays sound 0x793B (name inferred). */
    void TurnOn();
    /* Creates the target object and hands it to the model's parts (name inferred). */
    void CreateTarget();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ s32 m_state; /* 0 until the first update after construction or a reset (inferred) */
    /* 0x4C */ s32 m_type;  /* the parameter "type" */
    /* 0x50 */ s32 m_modelType; /* m_type when the model's parts were last set up (inferred) */
    /* 0x54 */ s32 m_comId;
    /* 0x58 */ UnkBitFlags_80014E04 m_flags; /* bit 0: turned off (inferred) */
    /* 0x5C */ u32 field_0x5C; /* in the allocation size (0x60); no code here touches it */
};
CHECK_SIZEOF(BAShield, 0x60);

} // namespace BAShield

#endif
