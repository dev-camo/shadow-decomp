#ifndef GAME_WATCHCAMERA_H
#define GAME_WATCHCAMERA_H

#include "Game/CameraModeUser.h"
#include "Game/ExternalCameraRequest.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Effect {
namespace System {
class EffectElement;
} // namespace System
} // namespace Effect

namespace WatchCamera {

/* WatchCamera::Sight (class name from RTTI): the marker of the point the camera aims at, an effect
 * that follows the position (behaviour inferred). The gadget creates it with new and owns it
 * through a shared pointer. Method names are inferred. */
class Sight {
  public:
    /* Copies the position and tries to start effect 0x6A at it (the original passes the argument,
     * not the copy, to fn_80168C48); when the effect exists, it follows the copy. */
    Sight(const Fn8040Vector3 *position);
    /* Stops the effect when it still exists. */
    ~Sight();

    /* Stops the current effect when it still exists, then tries to start effect id at the
     * position; when the new effect exists, it follows the position. */
    void SetEffect(int id);
    /* Switches to effect 0x6A (SetEffect), the one the gadget shows while it searches, and calls
     * Hide again when bit 3 is set (the sight is hidden). */
    void SetSearchEffect();
    /* Switches to effect 0x6F (SetEffect), the one the gadget shows after CanSeeTarget has
     * returned true in state 2 (also when there was no target to test), and calls Hide again when
     * bit 3 is set (the sight is hidden). */
    void SetFoundEffect();
    /* Clear (Show) or set (Hide) ELEMENT_FLAG_8 of the effect when it still exists, and bit 3 of
     * the flags. */
    void Show();
    void Hide();
    /* Copies the position. */
    void SetPosition(const Fn8040Vector3 *position);
    /* The gadget calls this every frame; its body compiles to nothing (the guard around the call
     * remains). */
    void Update() {}

    /* 0x00 */ Fn8040Vector3 m_position;
    /* Bit 3 is set while the sight is hidden. */
    /* 0x0C */ UnkBitFlags_80014E04 m_flags;
    /* 0x10 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
};
CHECK_SIZEOF(Sight, 0x18);

/* WatchCamera::Monitor (class name from RTTI): a second instance of the model WATCHCAMERA.BIN that
 * the gadget shows from StartCamera to StopCamera, placed in front of the matrix of entry 0 of the
 * fn_80009548 manager (behaviour inferred). The gadget creates it with new and owns it through a
 * shared pointer. Method names are inferred. */
class Monitor {
  public:
    /* Loads the model and passes 1 to its fn_80162A68 (the gadget passes 0 for its own). */
    Monitor();
    ~Monitor();

    /* When there is a model: places it at the matrix of entry 0 of the fn_80009548 manager, moved
     * 1.1 units from the matrix's translation towards the entry's vector at 0x14 (not moved when
     * the two are too close to normalise their difference, which fn_8040CE74 then zeroes), with
     * the x and y axes scaled by twice the two floats of the fn_8000DCC0 record times 1.1 (the
     * code of RainEffect's UpdateModelMatrix). dt is not used. */
    void Update(float dt);
    /* Call fn_80162790 (Hide) or fn_8016271C (Show) of the model when there is one; the gadget
     * calls the same two on its own model when its camera starts and stops (inferred: they hide
     * and show a model). */
    void Hide();
    void Show();

    /* 0x0 */ boost::shared_ptr<GadgetModelInfo> m_model;
};
CHECK_SIZEOF(Monitor, 0x8);

/* The gadget parameters in the order of the descriptor's table (the field names are the table's).
 * posX to posZ offset the collision box from the gadget's position (the offset is not turned with
 * the gadget), angX to angZ are the box's rotation in degrees, and width, height and depth are
 * passed to the box's SetSize. lookPosX to lookPosZ offset the point the camera looks at from the
 * gadget's position in the same way. The look angles are not read in this file. */
struct SetWatchCameraParams {
    SetWatchCameraParams()
        : posX(0.0f), posY(0.0f), posZ(0.0f), angX(0.0f), angY(0.0f), angZ(0.0f), width(10.0f),
          height(10.0f), depth(10.0f), lookPosX(0.0f), lookPosY(0.0f), lookPosZ(0.0f),
          lookAngX(0.0f), lookAngY(0.0f), lookAngZ(0.0f) {}

    /* 0x00 */ float posX;
    /* 0x04 */ float posY;
    /* 0x08 */ float posZ;
    /* 0x0C */ float angX;
    /* 0x10 */ float angY;
    /* 0x14 */ float angZ;
    /* 0x18 */ float width;
    /* 0x1C */ float height;
    /* 0x20 */ float depth;
    /* 0x24 */ float lookPosX;
    /* 0x28 */ float lookPosY;
    /* 0x2C */ float lookPosZ;
    /* 0x30 */ float lookAngX;
    /* 0x34 */ float lookAngY;
    /* 0x38 */ float lookAngZ;
};
CHECK_SIZEOF(SetWatchCameraParams, 0x3C);

/* WatchCamera::SetWatchCamera (class name from RTTI; the gadget's descriptor is "WATCH CAMERA"): a
 * security camera (behaviour inferred). While its collision box has hits that fn_8015AC84 accepts,
 * the gadget runs a camera mode at its position that looks at the look position, shows the monitor
 * model instead of its own and moves the sight around. While bit 2 of the flags is set, the camera
 * stops in the first update without such hits. ResetFromOwner clears that bit (ReadOwner) without
 * withdrawing the request or releasing the camera mode, so a camera that is running then is not
 * stopped by that check, and the next accepted hits call StartCamera again, which creates another
 * camera mode and asks fn_801CED24 for another request (registered only when its manager lookup
 * succeeds) without withdrawing the earlier request. Once CanSeeTarget has returned true in each
 * update until the 2-second timer is at most 0 (a false result returns to searching), the gadget
 * shoots towards the sight for one second without calling CanSeeTarget, at most once per update and
 * at least 1/12 second apart, unless the camera stops first. Method and member names are
 * inferred. */
class SetWatchCamera : public Gadget {
  public:
    SetWatchCamera(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetWatchCamera();
    virtual void UpdateBody(float dt);
    /* Passes the handle target to fn_8015BE64 when there is one (purpose unknown) and draws a
     * cross of half length 20 at the look position with fn_8015F5D8; dt is not used. */
    virtual void fn_800CBC1C(float dt);
    /* Calls ReadOwner (which clears all flags but leaves a running camera mode and its request
     * in place), then places the model, calls the model's fn_80162EF4 when there is a model, and
     * places the probe. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 14. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation, and its parameters only when the owner's
     * parameter size equals sizeof(SetWatchCameraParams), 0x3C (fn_800C9DBC; otherwise they keep
     * their values). Then sets the state, the timers, the sight's speed and the sight's state to 0,
     * clears all flags (also bit 2 and bit 4, see m_flags) and zeroes the sight's velocity. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, places it at the matrix that fn_8015CEA0 builds from the character
     * info. */
    void UpdateModelMatrix();
    /* When there is a handle target, places its probe: the position offset by the gadget's
     * position, the angles converted to radians, and the size (passed with -2, as RainEffect
     * passes its size). */
    void UpdateProbe();
    /* The handle target's callback: when fn_8015AC84 accepts the hits, starts the camera unless
     * bit 2 of the flags is set (then also hides the model when there is one, sets state 2, clears
     * bit 0 and sets bit 2), and sets bit 1 (touched in this update). */
    void OnCollision(ColliDetectArgs &args);
    /* Creates the camera mode with OnCameraUpdate as its callback, keeps the weak pointer that
     * fn_801CED24 returns for it (empty when no request was registered), and shows the monitor and
     * the sight when they exist. */
    void StartCamera();
    /* Passes the request to fn_801CEAD4, releases the camera mode when there is one and hides the
     * monitor and the sight when they exist. */
    void StopCamera();
    /* The camera mode's callback: when there is a camera mode, passes the gadget's position to
     * fn_8000DB78 and the look position to fn_8000DB50 (inferred: where the camera is and the
     * point it looks at). */
    void OnCameraUpdate(CameraUpdateEventArgs &args);
    /* When the character info that fn_800A8B74 finds for lbl_805EE588 still exists (its weak
     * pointer locks), copies its position to m_targetPosition and returns false when fn_80021E24
     * returns a node with a hit for the segment from the gadget to it (the node is then deleted);
     * otherwise returns true. That includes the case where the weak pointer does not lock:
     * m_targetPosition then keeps its earlier value (this file sets it only here), which is
     * uninitialised until a lock has succeeded, because its default constructor leaves it unset. In
     * that case the update still enters state 3 from state 2, or does not return from state 3 to
     * state 2 (it still moves on to state 4 when the timer is at most 0), and while the update
     * stays in state 3, MoveSightToTarget moves the sight towards that position when there is a
     * sight. */
    bool CanSeeTarget();
    /* Passes kind 0x1C and a parameter block to fn_80051C34: the position 20 units below the
     * gadget and the unit direction from there to the sight (the zero vector when the sight is
     * too close to normalise the difference; a shot, inferred; the sight is not checked for
     * null). Then tries to play sound 0x7840 and returns 1/12, the time until the next
     * shot. */
    float Shoot();
    /* State 0: picks a goal at a random angle about the y axis and a random distance up to 60
     * from the position of the character info that fn_800A8C54 finds (uninitialised when there is
     * none or its weak pointer does not lock), and sets the sight's velocity to the unit vector
     * towards the goal times 0.8 (the zero vector when the goal is too close to normalise the
     * difference, fn_8040CEE8 normalising a copy with fn_8040CE74). State 1: adds the velocity to the sight position, then returns
     * to state 0 when the new position is at most 5 units from the goal. In both states, then
     * passes the sight position to the sight when there is one. */
    void WanderSight();
    /* When there is a sight: adds 0.02 to m_sightSpeed (capped at 1), then moves the sight position
     * by that fraction of its offset to m_targetPosition (uninitialised until CanSeeTarget's first
     * successful lock) and passes it to the sight. */
    void MoveSightToTarget();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* The model's motion, whose sound events the update plays. */
    /* 0x48 */ boost::weak_ptr<_stMtnPlayWork> m_motion;
    /* 0 at the start, 1 inactive, 2 searching, 3 after CanSeeTarget has returned true in state 2
     * (also when there is no target to test), left for state 2 when it returns false or for state 4
     * when the timer is at most 0, 4 shooting (state 5 does nothing). */
    /* 0x50 */ s32 m_state;
    /* 0x54 */ SetWatchCameraParams m_params;
    /* 0x90 */ float m_timer; /* the time left in states 3 and 4 */
    /* 0x94 */ float m_shotTimer;
    /* ReadOwner clears all bits. Bit 0 (shooting) is set when state 4 starts and cleared when state
     * 4 ends, when OnCollision starts the camera and when UpdateBody stops it. Bit 1 (touched) is
     * cleared at the start of each update and set by OnCollision for accepted hits. Bit 2 is set
     * when OnCollision starts the camera and cleared when UpdateBody stops it. Bit 4 is set once by
     * the constructor, after its ReadOwner call, so a later ResetFromOwner turns it off for good;
     * while it is set, when the motion still exists and fn_80415A18 returns non-zero for event
     * 0x5001, the update tries to play the sound id it copies. */
    /* 0x98 */ UnkBitFlags_80014E04 m_flags;
    /* 0x9C */ boost::weak_ptr<PJSCamera::ExternalCameraRequest> m_cameraRequest;
    /* 0xA4 */ boost::shared_ptr<SonicteamUSA::System::Camera::CameraModeUser> m_cameraMode;
    /* Uninitialised until CanSeeTarget's first successful lock. */
    /* 0xAC */ Fn8040Vector3 m_targetPosition;
    /* 0xB8 */ boost::shared_ptr<Monitor> m_monitor;
    /* 0xC0 */ s32 m_sightState; /* the state of WanderSight */
    /* The fraction MoveSightToTarget moves, up to 1; set to 0 when state 3 is entered. */
    /* 0xC4 */ float m_sightSpeed;
    /* 0xC8 */ Fn8040Vector3 m_sightVelocity;
    /* 0xD4 */ Fn8040Vector3 m_sightGoal;
    /* 0xE0 */ Fn8040Vector3 m_sightPosition;
    /* 0xEC */ boost::shared_ptr<Sight> m_sight;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetWatchCamera, 0xF8);

} // namespace WatchCamera

#endif
