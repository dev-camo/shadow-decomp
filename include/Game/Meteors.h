#ifndef GAME_METEORS_H
#define GAME_METEORS_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Meteor.h"
#include "Game/Target.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class ColliDetectArgs;

/* The 0x24-byte settings of a Meteors (placeholder type name; the type has no RTTI). Meteor passes
 * them to the MeteorsTask fragments it creates and MeteorsHolder converts its gadget parameters
 * into them (0x8021FD54). The member names follow Meteors' use of them (inferred). */
struct UnkMeteorsParams_8021FCE4 {
    /* Converts the gadget parameters, as Meteor's ReadOwner does: the range and the speed scaled
     * by 10, the angles converted to radians, the direction the z axis turned about the x axis,
     * then about the y axis (behaviour inferred). That 0x8021FD54 is a constructor is inferred: it
     * builds the object at its first argument and returns that address. */
    explicit UnkMeteorsParams_8021FCE4(const MeteorParams &params);
    UnkMeteorsParams_8021FCE4(float range, float moveSpeed, const Fn8040Vector3 &direction,
                              UnkRotation3 rotateSpeed, float lifetime);
    UnkMeteorsParams_8021FCE4(const UnkMeteorsParams_8021FCE4 &other);

    /* 0x00 */ float m_range;     /* a character within it starts a waiting Meteors moving */
    /* 0x04 */ float m_moveSpeed; /* per second along m_direction */
    /* 0x08 */ Fn8040Vector3 m_direction;
    /* 0x14 */ UnkRotation3 m_rotateSpeed; /* radians per second about x, y and z */
    /* 0x20 */ float m_lifetime;           /* seconds of movement before it breaks up */
};
CHECK_SIZEOF(UnkMeteorsParams_8021FCE4, 0x24);

/* Meteors (class name from the bound callbacks' RTTI strings; the gadget's descriptor is
 * "METEOR_S"): a meteor that waits until a character comes within its range, then flies along its
 * direction while turning, shifts its model colour in the last second of its lifetime and
 * breaks up with an effect, a CommonBomb and a sound when the lifetime runs out, when attacks have
 * used up its health or when its swept sphere hits an object (behaviour inferred). Method names
 * marked "(name inferred)" are not original. */
class Meteors {
  public:
    Meteors(Task *task, const Fn8040Vector3 &position, const UnkRotation3 &rotation,
            const UnkMeteorsParams_8021FCE4 &params, int state);
    ~Meteors();

    /* Runs the state machine for one frame (name inferred). */
    void Update(float dt);
    /* Re-reads the position and rotation from the owner, places the model and draws the range and
     * the path (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Builds the model's matrix from the character info's position and rotation, and copies only
     * the position to the object with fn_800CCE38 (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: breaks the meteor up when it touches an object whose word at
     * 0x1C has none of the bits 0x40018 (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* The callback of the attack object for an attack on it (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The two callbacks the meteor gives its attack object with fn_80054DD0; the first passes the
     * meteor's direction, speed and a strength of 3 to the attacked object, the second breaks the
     * meteor up when the result word at 0x54 is 1 (behaviour inferred). */
    void fn_8021E4D8(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_8021E4A4(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x00 */ Task *m_task; /* the task that owns this object (name inferred) */
    /* 0x04 */ s32 m_state;  /* 0 waiting, 1 moving, 2 broken up (inferred) */
    /* 0x08 */ UnkMeteorsParams_8021FCE4 m_params;
    /* Bit 0 (set by the constructor): the next update calls fn_8016111C on the model; bit 1: the
     * meteor breaks up at the update's final flag check, in the same update when the update itself
     * sets it (inferred). */
    /* 0x2C */ UnkBitFlags_80014E04 m_flags;
    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<Target> m_object;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x50 */ float m_timer;  /* counts the lifetime down while moving (inferred) */
    /* 0x54 */ float m_health; /* 1 at start; attacks subtract strength (inferred) */
};
CHECK_SIZEOF(Meteors, 0x58);

/* MeteorsTask (class name from RTTI): a task that owns a Meteors through a shared pointer. Its
 * constructor (0x8021E334) creates the Meteors from its arguments; state is the Meteors' initial
 * state (Meteor passes 1, so its fragments move at once). */
class MeteorsTask : public Task {
  public:
    MeteorsTask(Task *parent, const Fn8040Vector3 &position, const UnkRotation3 &rotation,
                const UnkMeteorsParams_8021FCE4 &params, int state);
    virtual ~MeteorsTask();
    virtual void Update(float dt);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<Meteors> m_meteors;
};
CHECK_SIZEOF(MeteorsTask, 0x30);

/* MeteorsHolder (class name from RTTI; the gadget's descriptor is "METEOR_S"): the placed gadget,
 * which owns a Meteors built from its parameters. */
class MeteorsHolder : public Gadget {
  public:
    MeteorsHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~MeteorsHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 and its meteor (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Builds the meteor from the owner's position and rotation and the parameters (name
     * inferred). */
    void CreateMeteors(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<Meteors> m_meteors;
    /* 0x38 */ MeteorParams m_params;
};
CHECK_SIZEOF(MeteorsHolder, 0x58);

#endif
