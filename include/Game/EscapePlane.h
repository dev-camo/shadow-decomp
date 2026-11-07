#ifndef GAME_ESCAPEPLANE_H
#define GAME_ESCAPEPLANE_H

#include "Game/PathObject.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "rw/rwcore.h"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
class UnkObject_802B4254;
class UnkTarget_80166AF4;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class AttackCallbackParam;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

namespace EscapePlane {

/* EscapePlane::SetEscapePlane (class name from RTTI, with PathObject as its only base): the object
 * of ESCAPEPLANE.BIN that this file creates together with its EscapePlaneController::Controller
 * (fn_80255074); the ESCAPE_PLANE descriptor has no create function. It keeps two models of the
 * file and three frames of the first with a Target at each. Hits lower its health (800 at
 * construction); below 600, 400 and 200 and at 0 or less it moves to the next stage, and
 * IsFinished becomes true once a 4 second countdown after the last stage has run out (see Update,
 * OnAttack and SetStage). Behaviour inferred; method, member and constant names are inferred
 * unless PathObject gives them, and so are the parameters' reference and const spellings. */
class SetEscapePlane : public PathObject {
  public:
    /* The values of m_state (names inferred from what each state does). Nothing stores 1. */
    enum State {
        STATE_START = 0,
        STATE_STAGE0 = 2, /* until m_health is below 600 */
        STATE_STAGE1 = 3, /* until m_health is below 400 */
        STATE_STAGE2 = 4, /* until m_health is below 200 */
        STATE_STAGE3 = 5, /* until m_health is 0 or less */
        STATE_DEFEATED = 6,
        STATE_COUNTDOWN = 7, /* the 4 second m_timer */
        STATE_FINISHED = 8
    };

    /* The bits of m_flags (names inferred from where they are set and tested). The constructor
     * clears them (the flags constructor) and Initialize clears them again (Clear); nothing else
     * in this file clears them. */
    enum Flag {
        FLAG_FINISHED = 0,   /* set on entering STATE_FINISHED; IsFinished returns it */
        FLAG_STAGE_SOUND = 1 /* set by SetStage; Update then passes sound 0x7834 on every update */
    };

    /* Stores 800 in m_health and constructs the members, which clears m_flags; the timers, the
     * state and the frame pointers are first set by Initialize and FindFrames, and Initialize
     * clears m_flags again. */
    SetEscapePlane();
    /* Clears the frame pointers and releases the targets (ReleaseFrames) before the members are
     * destroyed. */
    virtual ~SetEscapePlane();

    /* Advances both models and field_0x1C, when each pointer is not null, by dt and counts
     * m_hitTimer down to no less than 0. Then it runs the state:
     * - STATE_START enters STATE_STAGE0.
     * - STATE_STAGE0 to STATE_STAGE3 call SetStage with 0 to 3 once m_health is below 600, 400 or
     *   200, or is 0 or less, and enter the next state; the first three also pass 0x4F2, 0x4F3 or
     *   0x4F4 and fn_801FAAC4's result to fn_801FAF00 unless that result is -1.
     * - STATE_DEFEATED sets m_timer to 4; once STATE_COUNTDOWN has counted it down to 0 or less,
     *   it sets FLAG_FINISHED and enters STATE_FINISHED.
     * Last, while FLAG_STAGE_SOUND is set, it passes sound 0x7834 to fn_801D53C4 at the character
     * info's position with itself as owner. */
    virtual void Update(float dt);
    /* Builds the character info and both models, requests the attachment of the character info
     * to the first model's part 0 (fn_80161FC8) without testing the result (nothing is attached
     * when the character info has expired or the model has no part 0) and, when the attack
     * object of that part still exists, sets its bytes at 0x54 and 0x55 to 0 and 1 and installs
     * OnAttack as its callback. Then it finds the frames, creates field_0x24 (fn_802B4254(0, 1))
     * and passes it 1 (fn_802B4510), places the models, resets the timers, the state, field_0x3C
     * and the flags, and, when there is a first model, creates field_0x1C and passes it to the
     * model's parts (fn_80169D00); when field_0x1C is not null it passes it -1 (fn_80167010). */
    virtual void Initialize();
    virtual void UnkSlot3();
    /* When there is a second model, passes it a matrix built from the identity, translated by
     * the character info's position and by the negated s_modelOffset (fn_8040B59C) and turned by
     * the character info's y angle (fn_8040B848); when there is a first model, passes it the same
     * matrix without the offset. For each frame pointer that is not null, moves the matching
     * target (the target pointer is not tested) to the position of the frame's local-to-world
     * matrix. Moves field_0x24, when the pointer is not null, to the character info's position
     * with s_objectHeight added to y. */
    virtual void UpdateModelMatrix();
    /* Whether FLAG_FINISHED is set. */
    virtual bool IsFinished();
    /* A weak pointer to the character info. */
    virtual boost::weak_ptr<CharInfo> GetCharInfo();
    virtual void UnkSlot7();
    /* When field_0x1C is not null, passes it 0x4B, 0x32 or 0x19 (fn_80167010) when m_health is
     * below 200, 400 or 600, and nothing from 600 up. */
    virtual void UnkSlot8();

    /* The attack object's callback (name inferred). It first sets the result (param's word at
     * 0x54) to 5. When m_hitTimer is above 0, or the attack object no longer exists, or none of
     * the bits 0x3D0 of its word at 0x40 is set, nothing else happens. Otherwise it subtracts the
     * parameter's float at 0x50 from m_health, sets m_hitTimer to 0.5 and keeps the word at 0x20
     * of the parameter's object at 0x14 in field_0x3C (0 when that pointer is null); while
     * m_health stays above 0 the result becomes 1 and it plays the sound 0x7830 at the character
     * info's position, otherwise the result becomes 3. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Stage 0 releases field_0x1C, the frames and the targets, calls the first model's
     * fn_80162804 and fn_80162A68(1, 0) (the pointer is not tested), finds the frames again,
     * places the models and, when there is a first model, creates field_0x1C again. Each stage
     * then passes its values to field_0x1C (fn_80166E20, fn_80167010, fn_80167448) when it is not
     * null and plays the sound 0x7832 at the character info's position. Stages 0-2 also pass 2, 4
     * or 6 to field_0x1C's fn_80167448 when field_0x1C is not null and field_0x3C is 1, 3 or 5.
     * Every call, whatever the stage, sets FLAG_STAGE_SOUND. */
    void SetStage(int stage);
    /* Keeps the frames with the ids of s_frameIds of the first model (not tested for null) as
     * m_frames and replaces each target with a new one (fn_801E72DC(0x800)) whose float at 0x20
     * is s_targetField20. */
    void FindFrames();
    /* Clears the frame pointers and releases each target whose pointer is not null
     * (fn_801E7108). */
    void ReleaseFrames();

    /* 0x04 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* Passed 0 by Initialize and 1 by stage 0 (fn_80162A68). */
    /* 0x0C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* Passed 2 by Initialize (fn_80162A68). */
    /* 0x14 */ boost::shared_ptr<GadgetModelInfo> field_0x14;
    /* 0x1C */ boost::shared_ptr<UnkTarget_80166AF4> field_0x1C;
    /* 0x24 */ boost::shared_ptr<UnkObject_802B4254> field_0x24; /* from fn_802B4254(0, 1) */
    /* 0x2C */ float m_health;                                   /* 800 at construction */
    /* 0x30 */ float m_timer;    /* the STATE_DEFEATED/STATE_COUNTDOWN timer */
    /* 0x34 */ float m_hitTimer; /* 0.5 after an accepted hit; hits are ignored above 0 */
    /* 0x38 */ s32 m_state;      /* a State */
    /* 0x3C */ s32 field_0x3C;   /* from OnAttack; SetStage tests it for 1, 3 and 5 */
    /* 0x40 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
    /* 0x44 */ RwFrame *m_frames[3];
    /* 0x50 */ boost::shared_ptr<Target> m_targets[3]; /* at m_frames */
};
CHECK_SIZEOF(SetEscapePlane, 0x68);

} // namespace EscapePlane

extern "C" {
/* Destroys the controller when there is one and creates a new SetEscapePlane with its controller
 * (fn_80255074). */
void fn_80254FB4(void);
/* Calls the controller's fn_8025F054 when there is a controller. */
void fn_80255000(void);
/* Destroys the controller when there is one. */
void fn_8025502C(void);
/* Creates a SetEscapePlane and an EscapePlaneController::Controller for it under task layer 6,
 * which becomes the file's controller (null when its allocation fails). */
void fn_80255074(void);
/* Listed in the order 0x80254EFC, 0x80254ED8, 0x80254EB4, 0x80254E90 at 0x804C4FC4, in a record
 * of the table at 0x804C4A18 whose other records also hold four such functions (the meaning of
 * each slot is unknown). Each calls one function above and returns 1 (return type inferred). */
int fn_80254E90(void);
int fn_80254EB4(void);
int fn_80254ED8(void);
int fn_80254EFC(void);
}

#endif
