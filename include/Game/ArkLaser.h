#ifndef GAME_ARKLASER_H
#define GAME_ARKLASER_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (member names inferred from the
 * table's labels "Length point", "ShotSec" and "WaitSec"). */
struct ArkLaserParams {
    /* The length of the beam: the model is scaled along y by it divided by 100, and the probe
     * gets half of it. */
    /* 0x0 */ float length;
    /* The timer value at which state 3 ends. */
    /* 0x4 */ float shotSeconds;
    /* The timer value at which state 5 ends. */
    /* 0x8 */ float waitSeconds;
};
CHECK_SIZEOF(ArkLaserParams, 0xC);

/* ArkLaser (class name from RTTI, with Gadget as its base): the gadget "ArkLaser", a laser beam
 * that goes through a cycle of states. Behaviour inferred; the method names marked "(name
 * inferred)" and the member names are not original, and the pointer and reference spellings of
 * the parameters are inferred.
 *
 * UpdateBody steps the states (m_state) and then passes dt to the model's fn_80162FB0. State 1
 * ends after one second; state 2 ends when stMtnControlWorkFlag(work, 0, 3, 1) returns non-zero,
 * where work is the motion control of the model's part 0 (fn_801602B4(0), locked and passed on
 * without a null test); state 3 ends when the timer reaches shotSeconds; state 4 ends on the same
 * motion test as state 2; state 5 ends when the timer reaches waitSeconds and leads back to
 * state 1. State 6, which ResetFromOwner sets, leads to state 1 on the next update. The three
 * timed tests add dt to the timer first and are ordered comparisons (timer >= limit), so they are
 * false while either value is a NaN. SetState describes what each change does. */
class ArkLaser : public Gadget {
  public:
    /* Clears m_state, stores the class name, resets m_effect, creates the character info
     * (fn_8015CD90(0x6E, 4)), reads the owner (CopyFromOwner), loads the model ARKLASER.BIN
     * (fn_8015F800) and creates the probe: fn_800C7A84 with half the length and s_radius as the
     * floats, 0x140000, 7 and 0, the zero vector lbl_805E07A0, a rotation of 90 degrees about x
     * and 0. It then creates the probe's attack object (fn_800C4374), gives it the two callbacks
     * (fn_80054DD0(4, 0x100, fn_8026DBB8, fn_8026DBB4, 1.0f)), clears its byte at 0x54, calls the
     * probe's UnkSlot2(-1) and sets state 1. The locked attack pointer is used without a null
     * test. */
    ArkLaser(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    /* When m_effect has not expired: clears ELEMENT_ACTIVE of the effect if it locks, and resets
     * m_effect. Then stops m_sound. */
    virtual ~ArkLaser();
    virtual void UpdateBody(float dt);
    /* Reads the owner again (CopyFromOwner), sets state 6, places the model (UpdateModelMatrix),
     * calls the probe's SetSize(s_radius, -2, 0.0f, half the length), the model's fn_80162EF4 and
     * the probe's UnkSlot26. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* The descriptor's create function (name inferred): allocates an ArkLaser and, when the
     * allocation succeeds, constructs it with the task that fn_801EB304 returns for index 11 as
     * its parent. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* The first callback of the probe's attack object (placeholder name). It builds the beam's axis:
     * the segment from the character info's position plus h to that position minus h, where h is
     * (0, half the length, 0) turned by the character info's rotation (about y, x and z). It
     * stores field_0x18 minus the segment point that fn_8040FC3C returns for field_0x18 in
     * field_0x34 (the closest point, except that for a segment with squared length below 0.0001
     * fn_8040FC78 takes the fraction 1, the second endpoint), normalises it in place
     * (fn_8040CE74, which makes it the zero vector when it is too short to normalise), and stores
     * s_callbackValue (100) in field_0x4C and 1 in field_0x50. */
    void fn_8026DBB8(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The second callback of the probe's attack object (placeholder name); it does nothing. */
    void fn_8026DBB4(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Changes the state (name inferred). When the current state is 3, it first clears the byte at
     * 0x54 of the probe's attack object (fn_800C42D0), calls the probe's UnkSlot2(-1) and stops
     * m_sound. Then, for the new state 1, it clears the timer and requests sound 0x7737 at the
     * character info's position (fn_801D54F4 returns without it when its sound setup returns
     * null); for 5, it clears the timer; for 3, it sets the attack object's
     * byte at 0x54 to 1, calls the probe's UnkSlot1(-1) and starts sound 0x7738 at the character
     * info's position in m_sound. Last it calls OnStateChange and stores the state. Both locked
     * attack pointers are used without a null test. The timer is not cleared for the new state
     * 3, so state 3 goes on counting from the value that state 1 left. */
    void SetState(int state);
    /* What the model and the effect do when the state changes (name inferred). For the previous
     * state 1, it calls RequestStop on m_effect if it locks, and resets m_effect either way; for
     * the previous state 4, it calls the model's fn_801604FC and fn_80162804. Then, for the next
     * state 1, only when m_effect has expired, it creates effect 0x27 at the character info's
     * position with the direction (0, 0, -1) (fn_80168C48) and, if the new effect locks, passes
     * it the beam's matrix (fn_80425E68); for 2, it calls the model's fn_80162A68(0, 0),
     * fn_80160C28(0) and fn_80160A84(0, 1) and places the model (UpdateModelMatrix); for 4, it
     * calls the model's fn_80160A84(0, 2); for 6, it calls the model's fn_801604FC and
     * fn_80162804. */
    void OnStateChange(int previous, int next);
    /* Passes the beam's matrix (see MakeBeamMatrix in ArkLaser.cpp) to the model's fn_801633DC
     * (name inferred). */
    void UpdateModelMatrix();
    /* Copies the owner's position and rotation to the character info (fn_800C9F14, fn_800C9ED4)
     * and its parameter block to m_params (fn_800C9DBC with size 0xC, which copies only when the
     * owner's block has that size) (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* The class name the constructor stores in m_name (name inferred). */
    static const char *s_name;

    /* 0x30 */ ArkLaserParams m_params;
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The probe that the constructor creates with fn_800C7A84. */
    /* 0x4C */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* Effect 0x27: created for the new state 1 and asked to stop when state 1 is left (see
     * OnStateChange). */
    /* 0x54 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x5C */ int m_state;
    /* The seconds that the states 1, 3 and 5 count. The constructor does not initialise it;
     * SetState(1), which the constructor calls last, clears it. */
    /* 0x60 */ float m_timer;
    /* Sound 0x7738: started for the new state 3 and stopped when state 3 is left and by the
     * destructor. */
    /* 0x64 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(ArkLaser, 0x80);

#endif
