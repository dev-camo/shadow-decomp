#ifndef GAME_VINE_H
#define GAME_VINE_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class MotionSequenceModule;
struct RpClump;
struct RwFrame;

namespace Vine {

/* The gadget's parameters, in the order of the descriptor's table ("radius", "attack sec",
 * "revive sec"). */
struct VineParams {
    VineParams() : radius(0.0f), attackSec(0.0f), reviveSec(0.0f) {}

    /* When the vine has no target, the nearest character (by distance in x, y and z) becomes it if
     * its squared horizontal distance is less than this value squared. */
    /* 0x0 */ float radius;
    /* 0x4 */ float attackSec; /* the wait before each attack */
    /* 0x8 */ float reviveSec; /* how long the vine stays down before it grows again */
};
CHECK_SIZEOF(VineParams, 0xC);

/* Vine::Vine (class name from RTTI; the gadget's descriptor is "FIRE_VINE"): a plant of two models
 * of VINE.BIN, the vine and its seed, each playing a motion sequence table. While it waits and its
 * target has expired, it takes the nearest character (chosen by full xyz distance) if its squared
 * horizontal (x/z) distance is less than the radius squared, and it keeps that target until the
 * squared horizontal distance exceeds (radius + 10) squared; losing the target leaves the stored
 * target position unchanged. It updates that position while waiting and turns towards the stored
 * position while waiting or attacking. After "attack sec" with a target it plays its attack
 * motion, which shoots a Vine::FireBall towards the stored position from the vine's frame with
 * id 1. Attacks take its health of 3; when that is used up it dies, stays down for "revive sec"
 * and grows again (behaviour inferred). Method and member names are inferred. */
class Vine : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). Nothing in this file enters
     * states 5 and 8; UpdateBody has an empty case for 8. */
    enum {
        STATE_START,
        STATE_WAIT,
        STATE_ATTACK,
        STATE_DIE,
        STATE_DEAD,
        STATE_5,
        STATE_REVIVE,
        STATE_DAMAGE,
        STATE_8
    };

    Vine(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Vine();
    /* Deletes the two motion sequences and clears their pointers. Matching workaround: see its
     * definition. */
    void DeleteSequences();
    /* Advances both models and both motion sequences by dt, runs the state function of m_state
     * and places the models. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places the models; when the vine model exists, also calls
     * its fn_80162EF4 and fn_8016108C and draws the radius as a debug cylinder (half length 5,
     * rotated 90 degrees about x) at the character info's position. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Enters state and restarts its steps. */
    void SetState(int state) {
        m_state = state;
        m_step = 0;
    }

    /* Reads the position, the rotation and the parameters from the owner, restores the full
     * health and enters STATE_WAIT. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the models at the character info. While waiting or attacking it first turns the
     * vine about the y axis towards m_targetPosition, by at most two degrees, from the other side
     * when the vine is upside down. Then, when there are both a launch frame (m_launchFrame) and
     * a target (m_target), moves the target to that frame. */
    void UpdateModelMatrix();
    /* The attack callback of the vine model: while the vine waits or attacks, an attack whose
     * attack object still locks and has none of the bits 0x3D1 set in its word at 0x40 is ignored
     * (the callback reports 5); otherwise, including when the attack object has expired, its
     * value (the parameter's float at 0x50) is taken from the health and the vine enters
     * STATE_DIE when that is used up and STATE_DAMAGE otherwise (the callback reports 1). In the
     * other states the callback reports 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When the first model and the launch frame (m_launchFrame) both exist, creates a
     * Vine::FireBall through FireBall::Create at that frame, flying towards m_targetPosition at a
     * speed of 3, and plays sound 0x7543 there; otherwise it does nothing. */
    void ShootFireBall();
    /* Keeps m_targetInfo and m_targetPosition up to date and returns whether there is a target.
     * Without a target it takes the character nearest to the character info when its squared
     * distance is less than the radius squared; it drops a target whose squared distance exceeds
     * GetLoseDistanceSq, (radius + 10) squared. These two checks use the horizontal (x/z)
     * distance; the nearest character is chosen by full xyz distance. */
    bool UpdateTarget();
    /* The squared distance beyond which a target is dropped: the radius plus 10, squared. */
    float GetLoseDistanceSq();

    /* The state functions that UpdateBody calls for m_state (see each definition). */
    void UpdateDamage(float dt);
    void UpdateRevive(float dt);
    void UpdateDead(float dt);
    void UpdateDie(float dt);
    void UpdateAttack(float dt);
    void UpdateWait(float dt);
    void UpdateStart(float dt);

    /* Loads the motion controls and the motion bases of the "vine" and "seed" motions and creates
     * their motion sequence modules, starting each at group 0. */
    void SetupSequences();
    /* Advances both motion sequences by dt. */
    void UpdateSequences(float dt);
    /* Selects group of the motion sequence of model (0: the vine, 1: the seed). */
    void SetSequenceGroup(int model, int group);
    /* Whether the motion sequence of model has played its group to the end. */
    bool IsSequenceFinished(int model);

    static const char *s_name;

    /* 0x30 */ VineParams m_params;
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ boost::shared_ptr<GadgetModelInfo> m_models[2]; /* the vine and the seed */
    /* 0x54 */ boost::shared_ptr<Target> m_target;
    /* 0x5C */ Fn8040Vector3 m_targetPosition;
    /* 0x68 */ float m_health;
    /* 0x6C */ float m_timer;
    /* The vine model's frame with id 1, where fire balls start. */
    /* 0x70 */ RwFrame *m_launchFrame;
    /* 0x74 */ boost::weak_ptr<_stMtnPlayWork> m_motions[2];
    /* fn_800C9178 passes the low 16 bits of the base - 1 plus the resolved motion number. */
    /* 0x84 */ s32 m_motionBases[2];
    /* 0x8C */ MotionSequenceModule *m_sequences[2];
    /* 0x94 */ boost::weak_ptr<CharInfo> m_targetInfo;
    /* Constructed and destroyed with the gadget; no code here reads or writes it. */
    /* 0x9C */ boost::weak_ptr<Target> field_0x9C;
    /* 0xA4 */ s32 m_state;
    /* 0 when a state is entered, 1 once it has started; the state functions also have an empty
     * case for 2, which nothing here sets. */
    /* 0xA8 */ s32 m_step;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Vine, 0xB0);

/* What a fire ball is created with (placeholder name; the binary does not name it). The
 * constructor only sets the lifetime, and the assignment copies the members one by one, the
 * vectors with their out-of-line assignment. */
struct FireBallParam_8014EDBC {
    FireBallParam_8014EDBC();
    FireBallParam_8014EDBC &operator=(const FireBallParam_8014EDBC &other);

    /* 0x00 */ Fn8040Vector3 m_position;
    /* Added to the position at each flying update whose segment meets no world. */
    /* 0x0C */ Fn8040Vector3 m_velocity;
    /* 0x18 */ float m_lifetime;
};
CHECK_SIZEOF(FireBallParam_8014EDBC, 0x1C);

/* Vine::FireBall (class name from RTTI): the fire ball that the vine shoots. A clone of the clump
 * JG0000FVINEBALL.DFF with the collision of JG0000FVINEBALL.CCL. Its first update sets the flying
 * state (1) without moving it; the clump and the collision handle are still updated in that update,
 * and the handle can already call the collision callback. Each flying update adds its velocity to
 * the position unless the segment to its next position meets the world, in which case it moves to
 * the hit point and marks a blast; its probe's collision callback also marks a blast while it
 * flies. A marked blast creates a Vine::FireBallBlast at the next update and enters state 3. When
 * the lifetime, which only flying updates reduce, becomes negative the update also enters state 3
 * (still updating the clump and handles that time); in an update where the segment hits and the
 * lifetime runs out, the expiry overwrites the marked blast and no blast is created. State 3 sets
 * the task's finished flag at the next update (behaviour inferred). Method and member names are
 * inferred. */
class FireBall : public Task {
  public:
    FireBall(Task *parent, const FireBallParam_8014EDBC &param);
    virtual ~FireBall();
    virtual void Update(float dt);

    /* Creates a fire ball under task layer 11. */
    static void Create(const FireBallParam_8014EDBC &param);

    /* Moves the clump's frame to the character info's position. */
    void UpdateClumpPosition();
    /* The probe's collision callback: a flying fire ball (state 1) blasts at the next update. */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x28 */ RpClump *m_clump;
    /* 0x2C */ FireBallParam_8014EDBC m_param;
    /* 0x48 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x50 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x58 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0 after construction, 1 flying, 2 to blast at the next update, 3 finished. */
    /* 0x60 */ s32 m_state;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(FireBall, 0x68);

/* Vine::FireBallBlast (class name from RTTI): the blast of a fire ball, a probe sphere at a fixed
 * position. It plays sound 0x7544 and starts effect 0x1A. After 0.13 seconds, if it has a handle,
 * it requests an attack object for the handle's probe, configures the attack only if the returned
 * weak pointer locks, and sizes the probe to the radius. The radius, 5 from its first update,
 * grows to 15 over the next 0.87 seconds, resizing the probe only when the handle exists; then it
 * finishes (behaviour inferred). Method and member names are inferred. */
class FireBallBlast : public Task {
  public:
    FireBallBlast(Task *parent, const Fn8040Vector3 *position);
    virtual ~FireBallBlast();
    virtual void Update(float dt);

    /* Creates a blast at position under task layer 11. */
    static void Create(const Fn8040Vector3 *position);

    /* The attack object's callback; it does nothing. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x28 */ Fn8040Vector3 m_position;
    /* 0x34 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x3C */ float m_radius;
    /* 0x40 */ float m_timer;
    /* 0 after construction, then 1, 2 while the radius grows, 3 finished. */
    /* 0x44 */ s32 m_state;
};
CHECK_SIZEOF(FireBallBlast, 0x48);

} // namespace Vine

#endif
