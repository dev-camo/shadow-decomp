#ifndef GAME_GRAVITYCHANGESWITCH_H
#define GAME_GRAVITYCHANGESWITCH_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. */
struct GravityChangeSwitchParams {
    GravityChangeSwitchParams() : gravity(0) {}

    /* 0x0 */ s32 gravity; /* GRAVITY: the index of the gravity direction to set */
};
CHECK_SIZEOF(GravityChangeSwitchParams, 0x4);

/* GravityChangeSwitch (class name from RTTI): a gadget with a switch model (GRAVITYCHANGESWITCH.BIN).
 * An update that finds bit 0 of m_hitFlags set (OnCollision sets it when fn_8015D2EC accepts the
 * model's collision hits) tries to start effect 0xE at the gadget (only when the previous effect
 * has expired), passes the index in GRAVITY to fn_80293928 (which sets the stage's gravity
 * direction when its state object exists), tries to play sound 0x773B, passes 5 and -2 to
 * fn_8034DF48 and, when the gadget has a com id, sets that id's state to 1; that update leaves the
 * state as it is. Every other update puts the gadget in state 2 while fn_80293AC8 returns the index
 * in GRAVITY and in state 1 while it does not (behaviour inferred). Method and member names are
 * inferred. */
class GravityChangeSwitch : public Gadget {
  public:
    GravityChangeSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GravityChangeSwitch();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Allocates the gadget and, when the allocation succeeds, constructs it under task layer 11
     * (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget and clears m_state. The
     * parameters are copied only when the owner's parameter size equals
     * sizeof(GravityChangeSwitchParams), 4 (fn_800C9DBC); otherwise they keep their values (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position with its rotation and moves the target
     * object to that position (name inferred). */
    void UpdateModelMatrix();
    /* Enters state 2 when fn_80293AC8 (the current gravity index, 0 without its state object)
     * returns the index in GRAVITY and state 1 otherwise, unless the gadget is already in that
     * state (name inferred). */
    void UpdateState();
    /* Calls the model's fn_80162804; then, for state 1 or 2, passes 0 or 1 to the model's
     * fn_80162A68, places the model, calls the model's fn_801612B8 or fn_801612E0 and sets or
     * clears bit 0 of the target object's flags. Stores the state in every case (name inferred). */
    void SetState(int state);
    /* The model's collision callback: sets bit 0 of m_hitFlags when fn_8015D2EC accepts the hits
     * (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ GravityChangeSwitchParams m_params;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ boost::shared_ptr<GadgetModelInfo> m_model; /* GRAVITYCHANGESWITCH.BIN */
    /* 0x44 */ boost::shared_ptr<Target> m_target;
    /* 0x4C */ s32 m_state; /* 0 after CopyFromOwner, then 1 or 2 (see SetState) */
    /* Bit 0: set by OnCollision, cleared by the update that acts on it (inferred: a character
     * touched the model). The constructor initialises the flags to 0. */
    /* 0x50 */ UnkBitFlags_80014E04 m_hitFlags;
    /* 0x54 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x5C */ u8 m_comId; /* the owner's com id (fn_800C9E60) */
};
CHECK_SIZEOF(GravityChangeSwitch, 0x60);

#endif
