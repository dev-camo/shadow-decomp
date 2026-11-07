#ifndef GAME_EGGBALLOON_H
#define GAME_EGGBALLOON_H

#include "Game/PathObject.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "rw/rwcore.h"
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

/* The angles that SetEggBalloon turns each of its six aim frames by (placeholder name). The
 * original builds the array with __construct_array and a default constructor (0x80147830) that
 * zeroes the three angles, so the element type has a constructor of its own; fn_8040E970 takes it
 * as an UnkRotation3. Its original name is unknown. */
struct UnkAngles_80147830 : public UnkRotation3 {
    UnkAngles_80147830() : UnkRotation3(0.0f, 0.0f, 0.0f) {}
};
CHECK_SIZEOF(UnkAngles_80147830, 0xC);

namespace EggBalloon {

/* EggBalloon::SetEggBalloon (class name from RTTI, with PathObject as its only base): the object
 * of EGGBALLOON.BIN that this file creates together with its EggBalloonController::Controller
 * (fn_8014565C); the EGG_BALLOON descriptor has no create function. While a command keeps its
 * firing time running it passes shots (fn_80051C34) from three of six frames or drop requests
 * (fn_80209B58) from another frame; command kind 3 clears the other kinds' firing, aiming and drop
 * flags, and either sets FLAG_KIND3 with a new firing time or, when that flag is already set, adds
 * to the firing time; Fire does nothing for it. While either aiming flag is set it turns three of
 * its six aim frames towards the nearest character (towards the zero vector when there is none),
 * and without one it turns all six back towards zero. Update runs Aim and Fire only when
 * fn_80169CD0 returns false, whatever flags and firing time remain set. It switches to its second
 * model set once its health (400 at construction) falls below 300 (see Update, OnCommand, Aim and
 * OnAttack). Behaviour inferred; method, member and constant names are inferred unless PathObject
 * gives them, and so are the parameters' reference and const spellings. */
class SetEggBalloon : public PathObject {
  public:
    /* The values of m_state (names inferred from what each state does). Nothing stores 1. */
    enum State {
        STATE_START = 0,
        STATE_STAGE0 = 2, /* until m_health is below 300 */
        STATE_STAGE1 = 3, /* until m_health is below 200 */
        STATE_STAGE2 = 4, /* until m_health is below 100 */
        STATE_STAGE3 = 5, /* until m_health is 0 or less */
        STATE_DEFEATED = 6,
        STATE_COUNTDOWN = 7, /* the 2 second m_timer */
        STATE_FINISHED = 8
    };

    /* The bits of m_flags (names inferred from where they are set, cleared and tested). */
    enum Flag {
        FLAG_FINISHED = 0,  /* set on entering STATE_FINISHED; IsFinished returns it */
        FLAG_FIRE_3TO5 = 1, /* shots from m_shotFrames[3-5] (command kind 0) */
        FLAG_FIRE_0TO2 = 2, /* shots from m_shotFrames[0-2] (command kind 1) */
        FLAG_DROP = 3,      /* drop requests from m_dropFrames[0] (command kind 2) */
        FLAG_KIND3 = 4,     /* command kind 3, which can set m_fireTime; Fire does nothing for it */
        FLAG_AIM_3TO5 = 5,  /* aim m_angles[3-5] (set with FLAG_FIRE_3TO5) */
        FLAG_AIM_0TO2 = 6,  /* aim m_angles[0-2] (set with FLAG_FIRE_0TO2) */
        FLAG_HIT_SOUND = 7  /* toggled by each accepted hit that leaves m_health above 0 */
    };

    /* Stores 400 in m_health and constructs the members; the timers, the state, the flags and
     * the frame pointers are first set by Initialize and FindFrames. */
    SetEggBalloon();
    /* Clears the frame pointers and releases the targets (ReleaseFrames) before the members are
     * destroyed. */
    virtual ~SetEggBalloon();

    /* Advances the model and each UnkTarget_80166AF4 object that exists (each pointer is tested
     * for null) by dt and counts m_hitTimer down to no less than 0; unless fn_80169CD0 returns
     * true, it counts m_fireTime and m_shotTimer down the same way. Then it runs the state:
     * - STATE_START creates field_0x1C when there is a model and enters STATE_STAGE0.
     * - STATE_STAGE0 to STATE_STAGE3 call SetStage with 0 to 3 once m_health is below 300, 200 or
     *   100, or is 0 or less, and enter the next state; the first three also pass 0x430, 0x431
     *   or 0x432 and fn_801FAAC4's result to fn_801FAF00 unless that result is -1.
     * - STATE_DEFEATED sets m_timer to 2; once STATE_COUNTDOWN has counted it down to 0 or less,
     *   it enters STATE_FINISHED, sets FLAG_FINISHED and clears the four firing flags (bits
     *   1-4).
     * Last, when fn_80169CD0 returned false (the second count-down ran), it calls Aim and, while
     * m_fireTime is above 0, Fire; when it returned true neither runs, even if an aiming flag is
     * set and m_fireTime is above 0 (USA branch at 0x80146F1C). */
    virtual void Update(float dt);
    /* Builds the character info ("SetEggBalloon") and the model with its first set, attaches the
     * character info to the model's part 0 and, when the attack object of that part still
     * exists, sets its bytes at 0x54 and 0x55 to 0 and 1 and installs OnAttack as its callback.
     * Then it finds the frames (FindFrames(0)), creates field_0x24 (fn_802B4254(2, 0)) and passes
     * it 1 and 300, keeps the aim frames' matrices (FindMatrixFrames(0)), places the model, and
     * resets the timers, m_shotInterval (1), the state, the flags and the angles. */
    virtual void Initialize();
    virtual void UnkSlot3();
    /* When there is a model, passes it the character info's matrix (fn_8015CEA0). Turns each
     * frame of m_aimFrames by its angles (y with fn_800E9CEC, x with fn_800E9C64) and applies its
     * kept matrix after them (fn_804876E8 with 2); the frame pointers are not tested. Moves
     * field_0x24, when the pointer is not null, to 105 below the character info's position, and
     * each target whose pointer is not null to the position of the matching m_aimFrames frame. */
    virtual void UpdateModelMatrix();
    /* Whether FLAG_FINISHED is set. */
    virtual bool IsFinished();
    /* A weak pointer to the character info. */
    virtual boost::weak_ptr<CharInfo> GetCharInfo();
    virtual void UnkSlot7();
    /* When field_0x14 is not null, passes it 0x4B, 0x32 or 0x19 (fn_80167010) when m_health is
     * below 100, 200 or 300, and nothing from 300 up; passes 0x29A to field_0x1C's fn_80167010
     * when that pointer is not null. */
    virtual void UnkSlot8();
    /* Command 0x303 (GadgetCommand::AttackStartCommand) selects the firing of the command's kind
     * (0: FLAG_FIRE_3TO5 with FLAG_AIM_3TO5, 1: FLAG_FIRE_0TO2 with FLAG_AIM_0TO2, 2: FLAG_DROP,
     * 3: FLAG_KIND3, which sets a firing time but fires nothing): it clears the other kinds'
     * flags, adds the command's time to m_fireTime when the kind's firing flag is already set,
     * otherwise stores the time and sets the kind's flags, and, except for kind 3, stores the
     * command's interval in m_shotInterval. Command 0x304 (GadgetCommand::AttackEndCommand, whose
     * members are not read) clears the four firing flags and both aiming flags and zeroes
     * m_fireTime. Other commands and kinds are ignored. */
    virtual void OnCommand(CharCommand &command);

    /* The attack object's callback (name inferred). It first sets the result (param's word at
     * 0x54) to 5. When m_hitTimer is above 0, or the attack object no longer exists, or none of
     * the bits 0x3D0 of its word at 0x40 is set, nothing else happens. Otherwise it subtracts the
     * parameter's float at 0x50 from m_health and sets m_hitTimer to 0.5; while m_health stays
     * above 0 the result becomes 1 and it plays the sound 0x754F (FLAG_HIT_SOUND set, which it
     * clears) or 0x754E (FLAG_HIT_SOUND clear, which it sets) at the character info's position;
     * otherwise the result becomes 3. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* For the shot frames 3-5 (FLAG_FIRE_3TO5) or else 0-2 (FLAG_FIRE_0TO2), passes the position
     * of each frame whose pointer is not null and (0, 0, 1) transformed by the frame's matrix
     * (fn_80482504) to fn_80051C34 (0x11) in an UnkParams_80051D10 with the character info as
     * owner. It then returns 1 when m_shotInterval is 0 or less (<= 0) and m_shotInterval
     * otherwise, so a NaN interval is returned unchanged. Without either flag it passes nothing
     * and returns 1. */
    float FireShots();
    /* When m_dropFrames[0] is not null, passes its position, the direction (0, -1, 0), the float
     * 5 and a clear flag to fn_80209B58 (kind 0, task layer 11). Returns 2 when m_shotInterval is 0
     * or less (<= 0) and m_shotInterval otherwise, so a NaN interval is returned unchanged. */
    float Drop();
    /* Unless m_shotTimer is above 0 (a NaN timer is not), stores in it what FireShots returns
     * for FLAG_FIRE_3TO5 or FLAG_FIRE_0TO2, or what Drop returns for FLAG_DROP, testing the flags
     * in that order. FLAG_KIND3 is tested (USA 0x80146068) and Fire then returns, so kind 3 fires
     * nothing even with a positive m_fireTime. Update calls Fire only when fn_80169CD0 returned
     * false and m_fireTime is above 0. */
    void Fire();
    /* For each index from first to last - 1: views the nearest character info's position (the zero
     * vector when the weak pointer that fn_800A8A7C returns for the character info's position is
     * expired) in the space of the sight frame's local-to-world matrix, takes the angles of that
     * direction (fn_8040F7F8), clamps both to -40..40 degrees and moves m_angles towards them by at
     * most half a degree (fn_8012773C). */
    void AimAt(int first, int last);
    /* Aims the angles 3-5 (FLAG_AIM_3TO5) or else 0-2 (FLAG_AIM_0TO2); without either flag, moves
     * every angle towards 0 by at most half a degree. Update calls it only when fn_80169CD0
     * returned false; otherwise the angles keep their values. */
    void Aim();
    /* Keeps the frames 0x1E and 0x14 of the model as m_dropFrames, the frames 10 of the shot
     * parts of the given set as m_shotFrames and the frames 1-6 as m_sightFrames, and replaces
     * each target with a new one (fn_801E72DC(0x2800)). The model pointer is not tested. */
    void FindFrames(int set);
    /* Stage 0 releases the frames, the targets and both UnkTarget_80166AF4 objects, switches the
     * model (not tested for null) to its second set, finds the frames of set 1 and, when there is
     * a model, creates field_0x14 and field_0x1C again. Each stage then passes its values to
     * field_0x14 (fn_80167448, fn_80167010) when it is not null, and stages 0-3 play the sound
     * 0x7550 at the character info's position; other values do nothing. */
    void SetStage(int stage);
    /* Keeps the frames 0xC9 of the aim parts of the given set as m_aimFrames and copies each
     * one's modelling matrix to m_aimMatrices; the found frames are not tested. */
    void FindMatrixFrames(int set);

    /* Clears the drop and shot frame pointers and releases the six targets (fn_801E7108). */
    void ReleaseFrames() {
        for (int i = 0; i < 2; i++) {
            m_dropFrames[i] = 0;
        }
        for (int i = 0; i < 6; i++) {
            m_shotFrames[i] = 0;
        }
        for (int i = 0; i < 6; i++) {
            fn_801E7108(m_targets[i]);
        }
    }

    /* 0x004 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x00C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x014 */ boost::shared_ptr<UnkTarget_80166AF4> field_0x14;
    /* 0x01C */ boost::shared_ptr<UnkTarget_80166AF4> field_0x1C;
    /* 0x024 */ boost::shared_ptr<UnkObject_802B4254> field_0x24; /* from fn_802B4254(2, 0) */
    /* 0x02C */ float m_health;                                   /* 400 at construction */
    /* 0x030 */ float m_hitTimer;     /* 0.5 after an accepted hit; hits are ignored above 0 */
    /* 0x034 */ float m_fireTime;     /* the firing time left (OnCommand) */
    /* 0x038 */ float m_shotTimer;    /* the time to the next shot or drop */
    /* 0x03C */ float m_shotInterval; /* from OnCommand; 1 after Initialize */
    /* 0x040 */ float m_timer;        /* the STATE_DEFEATED/STATE_COUNTDOWN timer */
    /* 0x044 */ s32 m_state;          /* a State */
    /* 0x048 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
    /* 0x04C */ RwFrame *m_dropFrames[2];     /* frames 0x1E and 0x14 */
    /* 0x054 */ RwFrame *m_shotFrames[6];
    /* 0x06C */ RwFrame *m_aimFrames[6];   /* turned by m_angles */
    /* 0x084 */ RwFrame *m_sightFrames[6]; /* aimed from */
    /* 0x09C */ RwMatrix m_aimMatrices[6]; /* the modelling matrices of m_aimFrames */
    /* 0x21C */ UnkAngles_80147830 m_angles[6];
    /* 0x264 */ boost::shared_ptr<Target> m_targets[6];
};
CHECK_SIZEOF(SetEggBalloon, 0x294);

} // namespace EggBalloon

extern "C" {
/* Destroys the controller when there is one and creates a new SetEggBalloon with its controller
 * (fn_8014565C). Called from 0x802733F4. */
void fn_8014559C(void);
/* Calls the controller's fn_802742D4 when there is a controller. Called from 0x80273418. */
void fn_801455E8(void);
/* Destroys the controller when there is one. Called from 0x802733D0. */
void fn_80145614(void);
/* Creates a SetEggBalloon and an EggBalloonController::Controller for it under task layer 6,
 * which becomes the file's controller (null when its allocation fails). Called from
 * 0x8027343C. */
void fn_8014565C(void);
}

#endif
