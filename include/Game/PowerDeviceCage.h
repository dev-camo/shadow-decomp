#ifndef GAME_POWERDEVICECAGE_H
#define GAME_POWERDEVICECAGE_H

#include "Game/Gadget.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
class UnkObject_802B4254;
class UnkTarget_80166AF4;
class ColliDetectArgs;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class AttackCallbackParam;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

/* The gadget parameters in the order of the descriptor's table ("MODEL 0:Out 1:In"). */
struct PowerDeviceCageParams {
    /* 0x0 */ u32 model; /* 0 Out or 1 In; passed to the model's fn_80162A68 */
};
CHECK_SIZEOF(PowerDeviceCageParams, 0x4);

/* PowerDeviceCage (class name from RTTI; the gadget's descriptor is "POWER_DEVICE_CAGE"): the
 * gadget of the POWERDEVICECAGE.BIN model, with two states, 0 and 1. In state 0 the model's
 * collision callback passes the hits to fn_8015AA84, which calls the hit callback only for the hits
 * whose collision object and character info still lock and whose character info's fn_80078B4C
 * (vtable offset 0x28) returns true; the hit callback sets bit 0 of m_flags. The next update that
 * finds the bit in state 0 switches the gadget to state 1 and sets m_triggered in the owner's
 * PowerDeviceCageSetState; the constructor starts a gadget in state 1 when that flag is set. The
 * attack callback's result is 5 on every path. Member names, and the method names marked "(name
 * inferred)", are not original; the parameters' pointer, reference and const spellings are
 * inferred. */
class PowerDeviceCage : public Gadget {
  public:
    PowerDeviceCage(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~PowerDeviceCage();
    /* In state 0, when the hit callback has set bit 0 of m_flags: switches to state 1, sets
     * m_triggered in the owner's PowerDeviceCageSetState and clears the bit. Then, in every
     * state, places the model and passes dt to its fn_80162FB0 and, when there is one, to
     * m_target's fn_8016761C. */
    virtual void UpdateBody(float dt);
    /* Reads the owner again. When the model parameter has changed, passes the new value to the
     * model and leaves and enters the current state again. Then places the model and calls its
     * fn_8016108C and fn_80162EF4. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 when its allocation succeeds (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation, the parameters and the tag from the owner (name
     * inferred). The parameters are copied into m_params only when the owner's parameter block is
     * 4 bytes (fn_800C9DBC compares the sizes and skips the copy otherwise). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, places it at the character info's position and y rotation (name
     * inferred). */
    void UpdateModelMatrix();
    /* Enters state 0 or 1; other values do nothing (name inferred). State 1 sets up the model with
     * "CoreMtn" and the model parameter, creates m_target, passes fn_80169D00 and a weak pointer to
     * m_target to the model's fn_8015FA78, requests sound 0x733A for m_state1Sound and releases
     * m_object. State 0 requests sound 0x7338 for m_state0Sound and creates m_object, placed
     * s_objectHeight (58) units above the character info. */
    void EnterState(int state);
    /* Leaves state 0 or 1; other values do nothing (name inferred). State 1 calls the model's
     * fn_801604FC, releases m_target and stops m_state1Sound; state 0 stops m_state0Sound. */
    void ExitState(int state);
    /* The model's collision callback: in state 0, passes the hits to fn_8015AA84 with the hit
     * callback and the gadget as its context (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* The attack object's callback (name inferred): when fn_8015D4E4 returns true, the attack
     * object can be locked and bit 0x2 of its word at 0x40 is clear, passes sound 0x733C and the
     * character info's position to fn_801D54F4. The result (param's word at 0x54) is 5 on every
     * path. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* The names of the members below are inferred. */
    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* Created when state 1 is entered, released when it is left. */
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* Created when state 0 is entered, released when state 1 is entered. */
    /* 0x48 */ boost::shared_ptr<UnkObject_802B4254> m_object;
    /* 0x50 */ s32 m_state; /* 0 or 1 */
    /* Bit 0: set by the hit callback, cleared by the update that switches to state 1. */
    /* 0x54 */ UnkBitFlags_80014E04 m_flags;
    /* 0x58 */ PowerDeviceCageParams m_params;
    /* 0x5C */ u8 m_tag; /* copied from the owner; passed to fn_8016907C */
    /* 0x60 */ UnkSoundHandle_801D3DC8 m_state0Sound;
    /* 0x7C */ UnkSoundHandle_801D3DC8 m_state1Sound;
};
CHECK_SIZEOF(PowerDeviceCage, 0x98);

/* The state that the owner keeps between the creations of the gadget (class name from RTTI,
 * derived from SetAdapterState; the member name is inferred). m_triggered is false when the
 * constructor of a gadget creates the object for an owner that has none, and is set when an
 * update switches the gadget to state 1. */
class PowerDeviceCageSetState : public SetAdapterState {
  public:
    PowerDeviceCageSetState() : m_triggered(false) {}

    virtual ~PowerDeviceCageSetState() {}

    /* 0x4 */ bool m_triggered;
};
CHECK_SIZEOF(PowerDeviceCageSetState, 0x8);

#endif
