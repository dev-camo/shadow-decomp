#ifndef GAME_GRAVITYCHANGEZONE_H
#define GAME_GRAVITYCHANGEZONE_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. */
struct GravityChangeZoneParams {
    GravityChangeZoneParams() : gravity(0) {}

    /* 0x0 */ s32 gravity; /* GRAVITY: the index of the gravity direction to set */
};
CHECK_SIZEOF(GravityChangeZoneParams, 0x4);

/* GravityChangeZone (class name from RTTI): a gadget with a switch model (GRAVITYCHANGESWITCH.BIN)
 * and a zone model (GRAVITYCHANGEZONE.BIN). When a character touches the zone model, it starts an
 * effect at the gadget, sets the stage's gravity direction to the index in GRAVITY, sets the state
 * of its com id and plays a sound; otherwise it puts itself in state 2 while the stage's gravity
 * direction is the one in GRAVITY and in state 1 while it is not (behaviour inferred). Method and
 * member names are inferred. */
class GravityChangeZone : public Gadget {
  public:
    GravityChangeZone(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GravityChangeZone();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters and com id into the gadget and clears
     * m_state (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places both models at the character info's position with its rotation (name inferred). */
    void UpdateModelMatrix();
    /* Enters state 2 when the stage's gravity direction is the one in GRAVITY and state 1
     * otherwise, unless the gadget is already in that state (name inferred). */
    void UpdateState();
    /* For state 1 or 2, passes 0 or 1 to the switch model's fn_80162A68, places the models and
     * clears or sets bit 1 of the zone model's flags; stores the state (name inferred). */
    void SetState(int state);
    /* The zone model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ GravityChangeZoneParams m_params;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ boost::shared_ptr<GadgetModelInfo> m_switchModel; /* GRAVITYCHANGESWITCH.BIN */
    /* 0x44 */ boost::shared_ptr<GadgetModelInfo> m_zoneModel;   /* GRAVITYCHANGEZONE.BIN */
    /* 0x4C */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x54 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x5C */ s32 m_state; /* 0 after CopyFromOwner, then 1 or 2 (see SetState) */
    /* 0x60 */ UnkBitFlags_80014E04 m_hitFlags; /* bit 0: a character touched the zone (inferred) */
    /* 0x64 */ u8 m_comId;                      /* the owner's com id (fn_800C9E60) */
};
CHECK_SIZEOF(GravityChangeZone, 0x68);

#endif
