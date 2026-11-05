#ifndef GAME_FIRE_H
#define GAME_FIRE_H

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

namespace Fire {

/* Fire::Fire (class name from RTTI; the gadget's descriptor is "FIRE"): a fire without a model. It
 * starts effect 0xA6 at the owner's position and keeps a sphere probe of radius 10 there, with a
 * handle target and an attack object, and a Target. While it waits (state 1), command id 0x14, a
 * call of the attack object's first callback or an attack whose attack object has one of the bits
 * 0x95 in its word at 0x40 moves it to state 2: on the next update it starts effect 0x29C at its
 * position and finishes. The first update after construction or a reset moves it from state 0
 * to waiting. Behaviour inferred; method names marked "(name inferred)" are not original. */
class Fire : public Gadget {
  public:
    Fire(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Fire();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and scale parameters into the gadget and returns to
     * state 0 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the character info's position to the effect, while it exists, and to the Target, when
     * there is one (name inferred). */
    void UpdatePositions();
    /* Clears the effect's active flag while the effect exists (name inferred). */
    void StopEffect();
    /* The character info's command handler: command id 0x14 moves a waiting fire to state 2
     * (name inferred). */
    void OnCommand(CharCommand &command);
    /* The attack object's second callback (name inferred). An attack whose attack object has one
     * of the bits 0x95 in its word at 0x40 moves a waiting fire to state 2: it passes the attack
     * object's owner, if any, to fn_8020642C with 0x1B and reports 3. Any other attack, or any
     * attack outside that state, is reported with 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The attack object's first callback (placeholder name): moves a waiting fire to state 2. */
    void fn_80104040(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the sphere probe */
    /* 0x40 */ boost::shared_ptr<Target> m_target;
    /* 0x48 */ boost::weak_ptr<Effect::System::EffectElement> m_effect; /* effect 0xA6 */
    /* 0 after construction or a reset, 1 waiting, 2 finishing (inferred). */
    /* 0x50 */ s32 m_state;
    /* The parameters "scale X", "scale Y" and "scale Z"; nothing in this file reads them. */
    /* 0x54 */ Fn8040Vector3 m_scale;
};
CHECK_SIZEOF(Fire, 0x60);

} // namespace Fire

#endif
