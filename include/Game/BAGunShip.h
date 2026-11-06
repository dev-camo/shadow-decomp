#ifndef GAME_BAGUNSHIP_H
#define GAME_BAGUNSHIP_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_8027C3C4.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace BAGunShip {

/* The gadget parameters in the order of the descriptor's table (names from the table's labels). */
struct BAGunShipParams {
    BAGunShipParams()
        : radius(0.0f), height(0.0f), second(0.0f), attack(0), field_0x10(0.0f), comId(0), num(0),
          delay(0.0f), interval(0.0f) {}

    /* 0x00 */ float radius;
    /* 0x04 */ float height;
    /* 0x08 */ float second;
    /* 0x0C */ s32 attack;       /* "attack OFF" when 0, "attack ON" otherwise */
    /* 0x10 */ float field_0x10; /* labelled "N/A"; nothing here reads it */
    /* For the UnkObject_8027C3C4, not the switch the gadget waits for (m_comId). */
    /* 0x14 */ s32 comId;
    /* 0x18 */ s32 num;
    /* 0x1C */ float delay;
    /* 0x20 */ float interval;
};
CHECK_SIZEOF(BAGunShipParams, 0x24);

/* BAGunShip::BAGunShip (class name from RTTI; the gadget's descriptor is "BA GUNSHIP"): a gadget
 * that shows the BAGUNSHIP.BIN model. Once the switch of its com id reports 1 (at once when the com
 * id is not positive) it waits for a hit on its probe for which fn_8015AC84 is true, then starts
 * the opening motion. When that motion ends it enters state 4 if bit 2 of m_gunFlags is set
 * ("second" <= 0) and state 3 otherwise, with m_timer set to "second". Each update in state 3 first
 * subtracts the time from m_timer; once m_timer <= 0 it starts the closing motion (state 5) and
 * skips the shot check in that update (a NaN "second" never closes). Otherwise in state 3, and on
 * every update in state 4, when "attack" is set it calls Shoot once the shot timer is <= 0 and
 * restarts the timer at the "interval" that Shoot returns; a shot needs the model's muzzle frame
 * (see Shoot). ReadOwner sets the shot timer to "interval"; in every state the update subtracts the
 * time from it and sets it to 0 only when the result is < 0 (a NaN stays). Some attacks take health
 * from it in states 3 and 4 (see OnAttack). When the closing motion ends it only sets state 7 and
 * clears bit 3 of m_gunFlags; the next UpdateBody passes the gadget to fn_8015A794 and sets the
 * finished flag (bit 0 of m_flags). After OnAttack has set state 6, the state switch of UpdateBody
 * calls StartCrashEffect, passes item type 0x10 to fn_80221BA8 and kind 3 to fn_80169B80, passes
 * the gadget to fn_8015A794 and sets the finished flag. It also drives an UnkObject_8027C3C4 built
 * from "ComId", "num" and "delay", rebuilt on every update in state 0; the calls on it are skipped
 * while its pointer is null. Behaviour inferred; method names marked "(name inferred)" and the
 * parameters' pointer and reference spellings are not original. */
class BAGunShip : public Gadget {
  public:
    BAGunShip(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BAGunShip();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Allocates the gadget under task layer 11 and constructs it when the allocation succeeds (name
     * inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians) into the character
     * info, the owner's parameters into m_params only when the owner's parameter size is 0x24
     * (fn_800C9DBC; otherwise m_params keeps its values) and the owner's com id; then clears the
     * state and the timers, sets the health to 40 and the shot timer to "interval", sets bit 0 of
     * m_gunFlags when "attack" is nonzero (clears it otherwise) and bit 2 when "second" <= 0
     * (clears it otherwise, also for a NaN); bits 1 and 3 keep their values (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null, places the model at the character info's position,
     * turned about the y axis by its rotation; when the m_object pointer is non-null, moves
     * m_object to that position (name inferred). */
    void UpdateModelMatrix();
    /* The callback of the attack object of the model's part with id 0 (name inferred). It sets the
     * result (param.field_0x54) to 5 and returns there when m_hitTimer > 0, when the state is
     * neither 3 nor 4, when the attack object cannot be locked or when its word at 0x40 has none
     * of the bits 0x3D0. Otherwise it subtracts param.field_0x50 from the health and sets
     * m_hitTimer to 0.5. When the health is then > 0 the result is 1 and sound 0x7931 is passed to
     * fn_801D54F4; otherwise (also for a NaN) it is 3, the state becomes 6, the attack object's
     * owner, if it can be locked, is passed to fn_8020642C with 0x4E, and sound 0x7932 is passed
     * to fn_801D54F4. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The collision callback of m_handle (name inferred): in state 2, a hit for which fn_8015AC84
     * is true sets m_timer to 1 and the state to 1 and starts the opening motion. */
    void OnHit(ColliDetectArgs &args);
    /* When the DM0000GUNSHIPBRK.DFF clump is loaded, fills an effect parameter block for the
     * character info and that clump, with the y of the effect's rotation taken from the angles
     * (fn_80415DF8) of the motion control of the model's part with id 0, or 0 without one, and
     * passes it to fn_8015D93C, which constructs the crash effect only when its allocation
     * succeeds (name inferred). */
    void StartCrashEffect();
    /* With a muzzle frame, passes kind 0x1D and a parameter block to fn_80051C34: the frame's
     * position and the vector from there to the character that fn_800A8B74 returns for
     * lbl_805EE588 (to the origin when that character cannot be locked), with its y set to 0 when
     * it is > 0 (a NaN y is left unchanged); then passes sound 0x7930 to fn_801D54F4. Without a
     * muzzle frame nothing happens. Returns "interval" either way (a shot and its interval; name
     * inferred). */
    float Shoot();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x50 */ boost::shared_ptr<Target> m_object;
    /* The state (names inferred from the code that sets them): 0 after construction or a reset,
     * 1 opening (set by OnHit), 2 waiting for a hit on the probe, 3 open with the time limit
     * "second", 4 open without a time limit (bit 2 of m_gunFlags set), 5 closing, 6 broken (set by
     * OnAttack; the next pass through UpdateBody's state switch runs the break), 7 closed, set when
     * the closing motion ends and awaiting the next UpdateBody, which passes the gadget to
     * fn_8015A794 and sets the finished flag (bit 0 of m_flags). */
    /* 0x58 */ s32 m_state;
    /* 0x5C */ BAGunShipParams m_params;
    /* 0x80 */ s32 m_comId;
    /* 0x84 */ boost::weak_ptr<_stMtnPlayWork> m_motion; /* of the model's part with id 0 */
    /* Cleared by ReadOwner; set to 1 by OnHit and counted down in state 1 while fn_801626F4 is
     * true; set to "second" when the opening motion ends and counted down in state 3. */
    /* 0x8C */ float m_timer;
    /* OnAttack ignores attacks while it is > 0; an attack it accepts sets it to 0.5. Every update
     * subtracts the time from it and sets it to 0 only when the result is < 0 (a NaN stays). */
    /* 0x90 */ float m_hitTimer;
    /* 0x94 */ float m_health;
    /* 0x98 */ boost::shared_ptr<UnkObject_8027C3C4> m_object8027C3C4;
    /* 0xA0 */ float m_shotTimer;
    /* The model's frame with id 7, where shots start (name inferred). */
    /* 0xA4 */ RwFrame *m_muzzleFrame;
    /* Bit 0: set by ReadOwner when "attack" is nonzero, cleared otherwise. Bit 1: where the shot
     * check runs (states 3 and 4 with bit 0 set), set when m_shotTimer <= 0 is false,
     * m_shotTimer <= 1 and the bit is clear (then m_target, when there is one, gets
     * fn_801672E4(1)); cleared after every Shoot call, also when Shoot finds no muzzle frame and
     * does not shoot. Bit 2: set by ReadOwner when "second" <= 0, cleared otherwise (also for a
     * NaN). Bit 3: set in state 1 on every update in which fn_801626F4 is true and m_timer <= 0
     * after the subtraction (with sound 0x792E), cleared when the closing motion ends (state 7);
     * while it is set, sound 0x792F is passed to fn_801D53C4 before the state switch of every
     * update. ReadOwner, and so a reset, changes only bits 0 and 2. */
    /* 0xA8 */ UnkBitFlags_80014E04 m_gunFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BAGunShip, 0xB0);

} // namespace BAGunShip

#endif
