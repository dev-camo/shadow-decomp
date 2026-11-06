#ifndef GAME_RAILSWITCH_H
#define GAME_RAILSWITCH_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkModelAtomicState.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace RailSwitch {

/* The gadget parameters in the order of the descriptor's table (the member name from the table's
 * string; the struct name is inferred). */
struct SetRailSwitchParams {
    SetRailSwitchParams() : route(0) {}

    /* The index into lbl_8054B8E0 (Game/UnkRailRoute_8054B8E0.h). The constructor does not
     * range-check it; the command handler accepts 0 to 10. */
    /* 0x0 */ s32 route;
};
CHECK_SIZEOF(SetRailSwitchParams, 0x4);

/* RailSwitch::SetRailSwitch (class name from RTTI; the gadget's descriptor is "RAIL SWITCH"): a
 * switch with the model RAILSWITCH.BIN. At construction, when the word at 0x8 of its route's
 * record in lbl_8054B8E0 is 1, it shows itself on (0 and 2 passed to the model's fn_80160A84, the
 * second of the file's two colours on the material "mat"), sets bit 1 of m_flags and sets m_on to
 * 1; otherwise it gives the material the first colour without calling fn_80160A84, clears bit 1
 * and sets m_on to 0. An attack that OnAttack accepts toggles m_on. When m_on becomes 1, it passes
 * 0 and 2 to fn_80160A84, gives the material the second colour and sets bit 1; when m_on becomes
 * 0, it passes 0 and 1, gives the material the first colour and clears bit 1. Either way it then
 * attempts to create an UnkObject_80169BB0 of kind 9 with fn_80169B80 at its character info's
 * position (result discarded) and to play sound 0x7835 there. Last, when fn_801DCB14 finds an
 * entry for 0x80 whose character info still exists, it allocates a SwitchCommand with the route
 * and passes it to that character info's fn_801DC980; when the allocation fails, the constructor
 * is skipped and fn_801DC980 receives a null pointer (behaviour inferred). Method and member names
 * are inferred. */
class SetRailSwitch : public Gadget {
  public:
    SetRailSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetRailSwitch();
    virtual void UpdateBody(float dt);
    /* Calls ReadOwner (which clears m_on and keeps m_flags), updates the model matrix, and calls
     * the model's fn_80162EF4 when there is a model. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, copies the parameters
     * when the owner's parameter block is 4 bytes (fn_800C9DBC), reads the com id, and clears
     * m_on and the hit delay; it leaves m_flags alone (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, passes it the character info's matrix from fn_8015CEA0 (name
     * inferred). */
    void UpdateModelMatrix();
    /* The callback of the model's attack object (name inferred). It stores 5 in param's word at
     * 0x54, and returns while the hit delay is above 0, when the attack object of param's first
     * weak pointer no longer exists, or when that object has none of the bits 0x3D1 set in its
     * word at 0x40. When param's value at 0x50 is below 0.9, it stores 5 in param's word at 0x54
     * again (the original repeats the store) and sets the hit delay to 1. Otherwise (the value is
     * not below 0.9, which includes NaN) it toggles the switch (see the class), stores 1 in
     * param's word at 0x54, sets m_soundTime to 0.2 and bit 0 of m_flags, and sets the hit delay
     * to 1. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* Counts down by dt while bit 0 of m_flags is set; when it falls to 0 or below, the update
     * attempts to play sound 0x7836 and clears bit 0 (name inferred). */
    /* 0x40 */ float m_soundTime;
    /* Counts down by dt to 0; OnAttack ignores attacks while it is above 0 (name inferred). */
    /* 0x44 */ float m_hitDelay;
    /* 0x48 */ s32 m_comId; /* the owner's com id (fn_800C9E50); not used here */
    /* Set to 1 or 0 at construction from the route's record and toggled by accepted attacks;
     * ReadOwner, which ResetFromOwner also calls, clears it (name inferred). */
    /* 0x4C */ s32 m_on;
    /* 0x50 */ SetRailSwitchParams m_params;
    /* 0x54 */ UnkModelAtomicState *m_material; /* the model's "mat" (fn_801621F0) */
    /* Bit 0: m_soundTime is counting down. Bit 1: set or cleared together with m_on at
     * construction and when an attack toggles the switch; while it is set, each update passes
     * sound 0x7837 to fn_801D53C4 with the gadget as its owner. ResetFromOwner clears m_on but
     * keeps both bits, so after a reset of a switch that was on, bit 1 stays set (and the sound
     * keeps being passed) while m_on is 0, and the next accepted attack sets m_on to 1 again
     * (inferred). Hides Task::m_flags. */
    /* 0x58 */ UnkBitFlags_80014E04 m_flags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetRailSwitch, 0x60);

} // namespace RailSwitch

#endif
