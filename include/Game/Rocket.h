#ifndef GAME_ROCKET_H
#define GAME_ROCKET_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

class CharCommand;
class ColliDetectArgs;

namespace Rocket {

/* Rocket::Rocket (class name from the RTTI strings of its bound callbacks and of its
 * sp_counted_base_impl; it has no RTTI record of its own): the rocket that a RocketBase builds
 * and a RocketTask flies. It has a character info of its own ("Rocket") and the "rocket" and
 * "area" nodes of ROCKET.BIN. Behaviour inferred from the code:
 * - In STATE_WAITING with FLAG_TOUCHED clear, the first character info that fn_8015AE44 passes
 *   to the hit callback (only those whose fn_80078B44 returns true) is, when its weak pointer
 *   still locks, sent a Player::TouchHangCommand and made the rider (see OnCollision).
 * - The first command 11 (a RideCommand in other handlers) sets FLAG_BOARDED and, when the base's
 *   character info still exists, sends it a UseCommand (see OnCommand). The base accepts that
 *   command only in its STATE_READY, where it hands the rocket to a new RocketTask (none when the
 *   allocation fails).
 * - Command 13 with FLAG_BOARDED set in STATE_WAITING starts STATE_IGNITION (0.5 seconds); then
 *   STATE_FLYING moves the rocket by its velocity each update until the squared distance from its
 *   start is > m_distanceSq (see Move), STATE_ARRIVED keeps moving it for 5 more seconds and
 *   STATE_FINISHED sets FLAG_FINISHED, which ends the RocketTask (see Update).
 * - Every command it sends is allocated with CharCommand's operator new and passed to
 *   fn_801DC980 even when the allocation returns null (USA (GUPE8P) 0x80115AC4, 0x80115D64,
 *   0x80116028, 0x80116200, 0x801162A8); the code after the send runs either way.
 * Method, member and constant names are inferred, and so are the parameters' pointer, reference
 * and const spellings unless a comment gives the evidence. */
class Rocket {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        STATE_WAITING = 0,
        STATE_IGNITION = 1,
        STATE_FLYING = 2,
        STATE_ARRIVED = 3,
        STATE_FINISHED = 4
    };

    /* The bits of m_flags (names inferred from where they are set and cleared). */
    enum Flag {
        /* Set by the first command 11; nothing clears it. */
        FLAG_BOARDED = 0,
        /* Set by OnCollision when it makes a character the rider (after sending it a
         * TouchHangCommand, null if the allocation failed); cleared at the start of every
         * update. */
        FLAG_TOUCHED = 1,
        /* Set by every update in STATE_FINISHED; nothing clears it. */
        FLAG_FINISHED = 2
    };

    /* owner is the base's character info. Stores it, enters STATE_WAITING and zeroes the timer;
     * builds the character info with its command unit and handler (OnCommand); copies the
     * position and rotation of the owner when it still exists (its weak pointer locks); loads the
     * model, attaches the character info to the "area" and "rocket" nodes, installs OnCollision
     * as the model's collision callback and places the model (UpdateModelMatrix). It does not
     * set the start, the velocity or the distance: RocketBase::UpdateModelMatrix does. */
    Rocket(const boost::weak_ptr<CharInfo> &owner);
    /* Passes field_0x68 to fn_8034DB60 before the members are destroyed. */
    ~Rocket();

    /* Clears FLAG_TOUCHED, advances the model and the target object when their pointers are
     * non-null, runs the state and places the model (UpdateModelMatrix) in every state.
     * - STATE_IGNITION: sends the rider, when it still exists, a TouchHangCommand with the hang
     *   position and angles; then counts the timer down by dt and, once it is < 0 (a NaN timer
     *   never is), calls the model's fn_80161880 (without testing the model pointer), enters
     *   STATE_FLYING, creates the target object (fn_80166AF4), hands it to the model's parts
     *   (fn_80169D00) and calls its fn_80167010(-1), passes field_0x68 to fn_8034DB60 unless it
     *   has expired, stores fn_80169B4C(m_rider, 11) in field_0x68 and plays sound 0x100A
     *   (fn_801D560C).
     * - STATE_FLYING: when Move returns true, passes field_0x68 to fn_8034DB60, enters
     *   STATE_ARRIVED with the timer at 5, places the model, calls fn_80292A64 with the rider and
     *   the hang position and angles (it creates a Parachute task when its allocation succeeds)
     *   and sends the base's character info, when it still exists, a ReadyCommand. Otherwise it
     *   sends the rider, when it still exists, a TouchHangCommand.
     * - STATE_ARRIVED: moves the rocket (Move, result unused) and counts the timer down by dt;
     *   once it is < 0, enters STATE_FINISHED.
     * - STATE_FINISHED: sets FLAG_FINISHED.
     * STATE_WAITING does nothing here; Move adds the whole velocity in each update, whatever dt
     * is. */
    void Update(float dt);
    /* When the model pointer is non-null: places the model at the character info's position,
     * turned about the y and x axes by its rotation, sets the hang position to (0, -8, 1) in the
     * model's space and the hang angles to (0, the rotation's y, 0). */
    void UpdateModelMatrix();
    /* Copies the position and the rotation to the character info, makes the position the start
     * of the flight and places the model.
     * Matching workaround: rotation is not const. The original loads each of its components only
     * after storing the previous one, as this compiler does when the stores may change the
     * source; with a const rotation the y and z loads move ahead of the x and y stores (4 words
     * differ). Revisit with the shared Fn8040Vector3/UnkRotation3 parameter conventions. */
    void SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation);
    void SetVelocity(const Fn8040Vector3 *velocity);
    /* The character info's command handler. Command 11, only while FLAG_BOARDED is clear: sets
     * it and sends the base's character info, when it still exists, a UseCommand naming this
     * rocket's character info (FLAG_BOARDED is set before the lock, so it stays set when the base
     * is gone). Command 13, only with FLAG_BOARDED set and in STATE_WAITING:
     * enters STATE_IGNITION with the timer at 0.5. Other commands are ignored. */
    void OnCommand(CharCommand &command);
    /* Adds the velocity to the character info's position and returns whether the squared
     * distance from the start is > m_distanceSq (false when either is NaN). m_distanceSq is the
     * square of the "distance" parameter, so a negative distance gives the same threshold as its
     * absolute value. */
    bool Move();
    /* The model's collision callback. Only in STATE_WAITING and while FLAG_TOUCHED is clear:
     * passes the hits to fn_8015AE44, which calls fn_80116FEC only for hits whose collision
     * object and character info still exist and whose character info's fn_80078B44 returns true
     * (USA (GUPE8P) 0x8015AF50-0x8015AF94); fn_80116FEC keeps the first non-null one. When there
     * is one and its weak pointer still locks, sends it a TouchHangCommand with the hang position
     * and angles (a null command when the allocation at 0x80115AC4 fails), then makes it the
     * rider and sets FLAG_TOUCHED in either case. */
    void OnCollision(ColliDetectArgs &args);

    /* Matching workaround: these inline members stand for unknown original accessors. Called
     * through a smart pointer as a statement or as the bare condition of an if, an inline member
     * leaves the smart pointer's operator-> out of line, as the original does (USA (GUPE8P)
     * 0x80114D04, called at 0x80114CDC and 0x80115928); a plain member access expands it in
     * place. Revisit when the original accessors are identified.
     *
     * Stores the square of distance in m_distanceSq. */
    void SetDistance(float distance) {
        m_distanceSq = distance * distance;
    }
    /* Whether FLAG_FINISHED is set. */
    bool IsFinished() const {
        return m_flags.Test(FLAG_FINISHED);
    }

    /* 0x00 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x10 */ boost::weak_ptr<CharInfo> m_owner; /* the base's character info */
    /* 0x18 */ boost::weak_ptr<CharInfo> m_rider; /* set by OnCollision */
    /* 0x20 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x28 */ Fn8040Vector3 m_start; /* set by SetTransform */
    /* 0x34 */ Fn8040Vector3 m_hangPosition;
    /* 0x40 */ UnkRotation3 m_hangAngles;
    /* 0x4C */ Fn8040Vector3 m_velocity;     /* added to the position once per Move */
    /* 0x58 */ float m_distanceSq;           /* the square of "distance" (SetDistance) */
    /* 0x5C */ float m_timer;                /* seconds left in STATE_IGNITION and STATE_ARRIVED */
    /* 0x60 */ s32 m_state;                  /* a State */
    /* 0x64 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
    /* 0x68 */ boost::weak_ptr<UnkObject_80169BB0> field_0x68; /* from fn_80169B4C */
};
CHECK_SIZEOF(Rocket, 0x70);

/* Rocket::RocketTask (class name from RTTI): the task that keeps a launched rocket updating once
 * its base has let it go, and finishes (Task flag 0x01) after an update that leaves FLAG_FINISHED
 * set. Method names are inferred. */
class RocketTask : public Task {
  public:
    /* Keeps a reference to rocket. Passing it by value is inferred from the caller's copy into a
     * temporary that it destroys after the call. */
    RocketTask(Task *parent, boost::shared_ptr<Rocket> rocket);
    virtual ~RocketTask();
    /* Updates the rocket, then sets Task flag 0x01 when the rocket's FLAG_FINISHED is set. It
     * does not test the rocket pointer. */
    virtual void Update(float dt);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<Rocket> m_rocket;
};
CHECK_SIZEOF(RocketTask, 0x30);

/* The gadget's parameters, in the order of the descriptor's table (names from its strings). The
 * constructor stores 0 in both; the table gives "distance" 10 as its first float (its default,
 * inferred). */
struct RocketParams {
    RocketParams() : angle(0.0f), distance(0.0f) {}

    /* 0x0 */ float angle; /* degrees; the base and the rocket lean by -angle about x */
    /* 0x4 */ float distance;
};
CHECK_SIZEOF(RocketParams, 0x8);

/* Rocket::RocketBase (class name from RTTI; the gadget's descriptor is "ROCKET"): the base of
 * ROCKET.BIN, which builds a Rocket, keeps it at the frame 0x7D3 of its model, leaned by "angle",
 * and hands it to a RocketTask on a UseCommand received in STATE_READY. Behaviour inferred;
 * method, member and constant names are inferred. */
class RocketBase : public Gadget {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State { STATE_EMPTY = 0, STATE_READY = 1, STATE_LAUNCHED = 2 };

    /* Builds the character info ("RocketBase") with its command unit and handler (OnCommand),
     * reads the owner (ReadOwner), loads the model with its "base" node and looks up two frames
     * of it (fn_80169E8C): when frame 0x7D3 is found, m_mountMatrix points to its
     * local-to-world matrix; when frame 0x7D0 is found, it becomes m_baseFrame and its modelling
     * matrix is copied to m_baseMatrix. The constructor does not initialise either pointer when
     * its frame is not found. Then it places the model (UpdateModelMatrix). */
    RocketBase(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~RocketBase();
    /* Advances the model and updates the held rocket when their pointers are non-null, places
     * everything (UpdateModelMatrix), and in STATE_EMPTY builds a new Rocket with this gadget's
     * character info as its owner, keeps it and enters STATE_READY; the new rocket is placed by
     * the next update. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places everything. When the model pointer is non-null it
     * also calls the model's fn_8016108C and queues a debug line from the position of
     * m_mountMatrix to the point "distance" away along the launch direction (fn_802E0B58), and a
     * debug cylinder at that point with half length "distance" and radius 200, turned by 90
     * degrees about x (fn_802E0C60). It reads m_mountMatrix without testing it. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians) into the character
     * info. Copies the owner's parameters only when the owner's parameter size equals
     * sizeof(RocketParams), 0x8 (fn_800C9DBC); otherwise m_params keeps its previous values.
     * Then enters STATE_EMPTY (without releasing a held rocket) and zeroes field_0x9C. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null, places the model at the character info's position,
     * turned about y by its rotation. When m_baseFrame is non-null, replaces its modelling matrix
     * with a rotation by -angle (in radians) about x and then applies m_baseMatrix after it
     * (fn_800E9C64, fn_804876E8). When both m_mountMatrix and the held rocket are non-null, sets
     * the rocket's transform to the position of m_mountMatrix with the angles (-angle in radians,
     * the character info's y rotation, 0), its velocity to (0, 1, 0) transformed by the matrix
     * that fn_8040B848 (the y rotation) and fn_8040B884 (-angle) build from the identity, scaled
     * by 14, and its m_distanceSq to the square of "distance" (SetDistance; the parameter is
     * not clamped). */
    void UpdateModelMatrix();
    /* The character info's command handler. Command 13 (a UseCommand), only in STATE_READY:
     * creates a RocketTask under task layer 11 with the held rocket, releases the rocket and
     * enters STATE_LAUNCHED; when the allocation fails there is no task, and the rocket is
     * released all the same. Command 14 (a ReadyCommand), only in STATE_LAUNCHED: enters
     * STATE_EMPTY, so the next update builds a new rocket. Other commands are ignored. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Rocket> m_rocket; /* the rocket on the base, until launched */
    /* 0x48 */ RocketParams m_params;
    /* 0x50 */ RwMatrix *m_mountMatrix; /* the local-to-world matrix of frame 0x7D3 */
    /* 0x54 */ RwMatrix m_baseMatrix;   /* frame 0x7D0's modelling matrix at construction */
    /* 0x94 */ RwFrame *m_baseFrame;    /* frame 0x7D0 */
    /* 0x98 */ s32 m_state;             /* a State */
    /* 0x9C */ float field_0x9C;        /* ReadOwner zeroes it; nothing in this file reads it */
};
CHECK_SIZEOF(RocketBase, 0xA0);

} // namespace Rocket

#endif
