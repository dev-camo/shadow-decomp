#ifndef GAME_POISONGAS_H
#define GAME_POISONGAS_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/Target.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace PoisonGas {

/* PoisonGas::PoisonGas (class name from RTTI; the gadget's descriptor is "POISON_GAS"): a gas
 * cloud without a model. It starts effect 0x8B at the owner's position and keeps a sphere probe of
 * radius 10 there with a handle target, and a Target. The constructor also asks fn_800C4374 for an
 * attack object on the probe; only when the returned weak pointer locks does it set the attack's
 * bytes at 0x54 and 0x55 to 0 and 1 and install OnAttack as its callback. Each update first runs
 * the handle target (fn_8015BE9C), whose callback (OnCollision) can move the gadget to state 2,
 * and then acts on the state: state 0 becomes waiting (state 1); in state 2 it starts effect 0x29F
 * at its position and finishes. A waiting gadget also moves to state 2 on command id 0x14
 * (OnCommand) or an attack with bit 0x10 (OnAttack). Behaviour inferred; method names marked
 * "(name inferred)" are not original. */
class PoisonGas : public Gadget {
  public:
    PoisonGas(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~PoisonGas();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget, and its scale parameters
     * when the owner's parameter block has their size (fn_800C9DBC), and returns to state 0 (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the character info's position to the effect, while it exists, and to the Target, when
     * there is one (name inferred). */
    void UpdatePositions();
    /* The handle target's callback for the probe's hits (name inferred): passes them to
     * fn_8015B004 with the hit callback, and moves the gadget to state 2, whatever its state, when
     * the hit callback ran for at least one character info. */
    void OnCollision(ColliDetectArgs &args);
    /* Clears the effect's active flag while the effect exists (name inferred). */
    void StopEffect();
    /* The character info's command handler: command id 0x14 moves a waiting gadget to state 2
     * (name inferred). */
    void OnCommand(CharCommand &command);
    /* The attack object's second callback (name inferred). It reports 5 unless the gadget is
     * waiting and the attack object still exists with bit 0x10 set in its word at 0x40: then it
     * reports 3 and moves the gadget to state 2. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the sphere probe */
    /* 0x40 */ boost::shared_ptr<Target> m_target;
    /* 0x48 */ boost::weak_ptr<Effect::System::EffectElement> m_effect; /* effect 0x8B */
    /* 0 after construction or a reset, 1 waiting, 2 finishing (inferred). */
    /* 0x50 */ s32 m_state;
    /* The parameters "scale X", "scale Y" and "scale Z"; nothing in this file reads them. */
    /* 0x54 */ Fn8040Vector3 m_scale;
    /* The owner's com id (fn_800C9E60); nothing in this file reads it. */
    /* 0x60 */ u8 m_comId;
};
/* The last member ends at 0x61; the size is rounded up to Task's 8-byte alignment. */
CHECK_SIZEOF(PoisonGas, 0x68);

} // namespace PoisonGas

#endif
