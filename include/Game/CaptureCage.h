#ifndef GAME_CAPTURECAGE_H
#define GAME_CAPTURECAGE_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

/* The gadget parameters in the order of the CAPTURE_CAGE descriptor's table, which labels all
 * three "N/A". CopyFromOwner copies them from the owner (only when the owner's parameter block is
 * 0xC bytes: fn_800C9DBC compares the size at 0x800C9DD4); nothing else in the file reads them. */
struct CaptureCageParams {
    CaptureCageParams() : field_0x0(1.0f), field_0x4(1.0f), field_0x8(1.0f) {}

    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
};
CHECK_SIZEOF(CaptureCageParams, 0xC);

namespace CaptureCage {

/* CaptureCage::CaptureCage (class name from RTTI; the gadget's descriptor is "CAPTURE_CAGE"): a
 * cage of the CAPTURECAGE.BIN model that selects the nearest eligible touching character and sends
 * it bind requests, and a release request when it breaks (behaviour inferred); names inferred.
 *
 * The model's collision callback does nothing while FLAG_BIT1 or FLAG_BIT0 of m_stateFlags is set.
 * Otherwise it passes fn_800F48A8 to fn_8015B70C with 3, so that the callback sees the character
 * infos of the hits whose collision object and character info still lock and whose character info's
 * word at 0x5C is 3 (as described at fn_8015B70C); of those it selects the one whose squared
 * distance from the cage's position is the smallest, counting only squared distances below 1e8
 * (each must be strictly smaller than the best so far; NaN never is). When it found one, it
 * replaces m_captured with it (a weak reference; any later call can replace it again) and, if
 * m_captured still locks, sends it a new BindCommand holding the cage's character info (a request;
 * a null command when the allocation fails).
 *
 * The attack callback of the attack object of model part 0 (set only when the constructor could
 * attach the character info to part 0 and the attack object locks) first sets the parameter's
 * word at 0x54 to 5, and leaves it so unless the state is STATE_INTACT and the parameter's attack
 * object still locks. For an attack object whose word at 0x40 has bit 0 or bit 1 set it stores 5
 * again; otherwise it subtracts the parameter's float at 0x50 from m_health and, when m_health <= 0
 * (false for NaN), stores 3, enters STATE_BREAKING and, if the attack object's owner (its character
 * info) locks, passes it to fn_8020642C with 0x1F; otherwise it stores 1.
 *
 * Each update first advances the model when there is one (fn_80162FB0, which can run the collision
 * callback), then by state: STATE_START enters STATE_INTACT; STATE_INTACT does nothing (the attack
 * callback acts in it); STATE_BREAKING (entered by the attack callback, handled on the next update)
 * calls the model's fn_80161880 and fn_80162804, starts the crash effect, passes 1 and 0 to the
 * model's fn_80162A68, attempts to attach the character info to part 1 (fn_80161FC8, result
 * ignored), places the model, sets m_timer to 10 and enters STATE_CRASHING; then, unless m_captured
 * has expired, sends the character info that m_captured still locks to (if any) a new
 * ReleaseCommand holding the cage's character info (a request, as for the BindCommand), resets
 * m_captured and clears FLAG_BIT0; in either case it sets FLAG_BIT1. STATE_CRASHING subtracts dt
 * from m_timer; then, while FLAG_BIT2 is clear, once m_timer <= 2 it passes -1 to the model's
 * fn_8016268C and sets FLAG_BIT2 (nothing in this file clears it); then, once m_timer <= 0, it
 * enters STATE_DONE (both tests are false for NaN). STATE_DONE passes the gadget to fn_8015A794 and
 * sets Task flag 0x01 on every update. The STATE_BREAKING and STATE_CRASHING code uses the model
 * without testing for one.
 *
 * CopyFromOwner enters STATE_START and sets m_health to 5 and m_timer to 0; it does not touch
 * m_stateFlags or m_captured. Nothing in this file sets FLAG_BIT0. */
class CaptureCage : public Gadget {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State { STATE_START, STATE_INTACT, STATE_BREAKING, STATE_CRASHING, STATE_DONE };

    /* The bits of m_stateFlags. The names only give the bit numbers: nothing in this file sets
     * FLAG_BIT0, and what FLAG_BIT2 marks depends on fn_8016268C. */
    enum Flag { FLAG_BIT0 = 0, FLAG_BIT1 = 1, FLAG_BIT2 = 2 };

    CaptureCage(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CaptureCage();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation and the parameters (see CaptureCageParams), then
     * enters STATE_START and sets m_health to 5 and m_timer to 0 (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when there is one, at the character info's position, turned by its y
     * rotation (name inferred). */
    void UpdateModelMatrix();
    /* The attack object's callback (name inferred; behaviour in the class comment). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The model's collision callback (name inferred; behaviour in the class comment). */
    void OnCollision(ColliDetectArgs &args);
    /* When fn_80012700 returns a clump for CAPTCAGECRSH01.DFF (key 0), passes fn_8015D93C a
     * parameter block built for the character info (kind 0) with that clump at 0x44, its position
     * 20 higher in y, 1.3 at 0x34, the character info's y position at 0x40, 1 at 0x4A and 0x54
     * and 150 at 0x4C; otherwise does nothing (name inferred). */
    void StartCrashEffect();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The nearest hit of the last collision callback that found one (see the class comment). */
    /* 0x40 */ boost::weak_ptr<CharInfo> m_captured;
    /* 0x48 */ CaptureCageParams m_params;
    /* A State value (see the class comment). */
    /* 0x54 */ s32 m_state;
    /* 0x58 */ float m_health; /* 5 after CopyFromOwner; lowered by the attack callback */
    /* 0x5C */ float m_timer;  /* 10 when the cage breaks, counted down by dt in STATE_CRASHING */
    /* 0x60 */ UnkBitFlags_80014E04 m_stateFlags; /* Flag bits */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(CaptureCage, 0x68);

} // namespace CaptureCage

#endif
