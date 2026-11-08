#ifndef GAME_GUNLIFT_H
#define GAME_GUNLIFT_H

#include "Game/CameraModeUser.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/ExternalCameraRequest.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/PathCtrl.h"
#include "Game/TVehicleBase.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Vehicle {

/* The speeds of a gun lift kind, as pointers to four float constants (placeholder name; member
 * names inferred from GunLift::Update). Update changes the speed only while bit 0 of field_0x70 is
 * set and fn_800ABDCC reports m_path valid. Then, while bit 2 of the lift flags is set, the speed
 * rises by acceleration per second up to maxSpeed; otherwise a speed above cruiseSpeed falls by
 * acceleration per second down to cruiseSpeed, and any other speed rises by cruiseAcceleration per
 * second up to cruiseSpeed. */
struct GunLiftSpeeds {
    /* 0x0 */ const float *maxSpeed;
    /* 0x4 */ const float *acceleration;
    /* 0x8 */ const float *cruiseSpeed;
    /* 0xC */ const float *cruiseAcceleration;
};
CHECK_SIZEOF(Vehicle::GunLiftSpeeds, 0x10);

/* Vehicle::GunLift (class name from RTTI, with CharInfo and Vehicle::TVehicleBase as its bases):
 * the vehicle of the types "GunLiftExpress" (kind 0) and "GunLiftLocal" (kind 1), which differ only
 * in their speeds. While bit 0 of field_0x70 is set (fn_801B62C4 sets it, fn_801B6254 resets it),
 * Update moves it along the path that SetUpPath found and handles the gun. It is built from the
 * parts 0, 1 and 2 of the GUNLIFT.BIN model: UpdateMatrices places part 1 at frame 1 of part 0,
 * turned about y by m_turretYaw, and part 2 at frame 2 of part 1, tilted about x by m_gunPitch; the
 * names lift, turret and gun are inferred from that placement. The frame lookup (fn_80169EF0)
 * leaves its matrix unchanged when the part, the frame or the frame's matrix is missing: the
 * turret is then placed from the identity matrix it starts from, and the gun from whatever
 * m_gunMatrix holds: the previous update's result, or, on the first update, contents the
 * constructor never set (SET_VEHICLE can call SetPosition right after creating the vehicle).
 * Behaviour inferred; method names marked "(name inferred)" and the pointer and reference
 * spellings of the parameters are not original. */
class GunLift : public TVehicleBase {
  public:
    GunLift(int kind, int type);
    virtual ~GunLift();

    /* Sets up m_path at position (rotation is not used) and places the parts with
     * UpdateMatrices(0.0f). */
    virtual void SetPosition(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Copies the translation and the angles (fn_800493C4) of frame 10 of part 1 of the turret
     * model to the non-null arguments. The matrix starts as the identity, and fn_80169EF0 leaves it
     * so when the part, the frame or the frame's matrix is missing; the identity's translation and
     * angles are then copied. */
    virtual void GetPosition(Fn8040Vector3 *position, UnkRotation3 *rotation);
    /* Clears the angles, the speed, the shot timer and the lift flags, passes 1.0f to
     * fn_801B6504, builds the character infos and models of the three parts (the turret model is
     * attached to the vehicle itself (GetSelf), not to m_turretInfo), passes the collision
     * resource 0x79 to fn_801B641C, gives the turret's and the gun's attack objects OnAttack and
     * sets their field_0x55 to 0 (each only when its attack object locks), builds the lift's and
     * the turret's targets and creates m_camera with OnCameraUpdate. */
    virtual void fn_801B634C();
    /* An AccelerateInputCommand (id 1) turns the turret and tilts the gun, which is kept between
     * -10 and 60 degrees; a ControlVehicleCommand (id 0x15) changes the lift flags (see
     * m_liftFlags), and its values 1 and 4 call SendStartJump. Bit 1 of the command's flags is set
     * for id 1 and for the values 1, 4, 6 and 7. */
    virtual void OnCommand(CharCommand *command);
    virtual void Update(float dt);
    /* After TVehicleBase's: clears m_gunPitch and m_speed, sets field_0x55 of the turret's and the
     * gun's attack objects to 1 (each only when its collision object and then its attack object
     * lock), requests the camera (fn_801CED24), calls fn_80160A84(2, 1), which looks up the gun
     * model's motion entry with id 2 and requests the motion its entry data names (the id at the
     * entry's 0x1C, minus 1, plus the second argument), or returns false and requests nothing when
     * there is no entry 2, and sets bit 4 of the lift flags whatever the result, creates
     * m_gunTarget and m_effect and passes sound 0x7637 to fn_801D54F4. */
    virtual void fn_801B62C4();
    /* After TVehicleBase's: clears ELEMENT_ACTIVE of m_effect when it locks, releases m_effect
     * and m_gunTarget, passes 2 to the gun model's fn_80160670, sets field_0x55 of the attack
     * objects to 0 (each only when its collision object and then its attack object lock), clears
     * m_gunPitch and m_speed, withdraws the camera request (fn_801CEAD4), stops both sounds and
     * passes m_shotObject to fn_8034DB60. */
    virtual void fn_801B6254();
    /* Return true (false in TVehicleBase). */
    virtual bool fn_801B624C();
    virtual bool fn_801B6244();

    /* Allocate a gun lift of kind 0 or 1 for the vehicle type and construct it when the
     * allocation succeeds (names inferred). */
    static TVehicleBase *CreateExpress(int type);
    static TVehicleBase *CreateLocal(int type);

    /* The attack callback of the turret's and the gun's attack objects (name inferred): sets the
     * result (param.field_0x54) to 5, or, while bit 0 of field_0x70 is set, to 1; in that case it
     * also passes a new DamageCommand (null when the allocation fails) to the fn_801DC980 of the
     * character info at 0x80 when that still exists, and 0x1F5 to m_turretTarget's fn_80167448. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The update callback of m_camera (name inferred). */
    void OnCameraUpdate(CameraUpdateEventArgs &args);
    /* Places the three parts and their character infos (name inferred). */
    void UpdateMatrices(float dt);
    /* Sets up m_path at position (name inferred). */
    void SetUpPath(const Fn8040Vector3 *position);
    /* Passes a new Player::StartJumpCommand (null when the allocation fails) to the fn_801DC980 of
     * the character info at 0x80 when that still exists (name inferred). */
    void SendStartJump();

    /* 0x09C */ int m_kind;
    /* 0x0A0 */ const GunLiftSpeeds *m_speeds;
    /* 0x0A4 */ SonicteamUSA::System::PathCtrl m_path;
    /* 0x0FC */ boost::shared_ptr<GadgetCharInfo> m_liftInfo;
    /* 0x104 */ boost::shared_ptr<GadgetCharInfo> m_turretInfo;
    /* 0x10C */ boost::shared_ptr<GadgetCharInfo> m_gunInfo;
    /* 0x114 */ boost::shared_ptr<GadgetModelInfo> m_liftModel;
    /* 0x11C */ boost::shared_ptr<GadgetModelInfo> m_turretModel;
    /* 0x124 */ boost::shared_ptr<GadgetModelInfo> m_gunModel;
    /* 0x12C */ boost::shared_ptr<UnkTarget_80166AF4> m_liftTarget;
    /* 0x134 */ boost::shared_ptr<UnkTarget_80166AF4> m_turretTarget;
    /* Created by fn_801B62C4 for the gun and reset by fn_801B6254. */
    /* 0x13C */ boost::shared_ptr<UnkTarget_80166AF4> m_gunTarget;
    /* The effect 0xCC that fn_801B62C4 creates at m_position. While bit 0 of field_0x70 is set and
     * the effect locks, Update moves it to the point that fn_80051AF0 reports and clears its
     * ELEMENT_FLAG_8, or sets that flag when nothing is reported. */
    /* 0x144 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x14C */ boost::weak_ptr<PJSCamera::ExternalCameraRequest> m_cameraRequest;
    /* 0x154 */ boost::shared_ptr<SonicteamUSA::System::Camera::CameraModeUser> m_camera;
    /* fn_801B634C clears every bit. Bit 0: set by ControlVehicleCommand 6, reset by 7 and by
     * fn_801B6254; Update shoots only while it is set. Bit 2: set by ControlVehicleCommand 2, reset
     * by 3; while it is set, Update raises the speed towards maxSpeed (see GunLiftSpeeds for when
     * Update changes the speed). Bit 3: set, starting m_moveSound, on an update in which
     * fn_8001C91C returns false, which a step of zero distance (zero speed or dt) on a segment of
     * non-zero length also gives, so it does not prove that the lift moved; reset when fn_8001C91C
     * returns true and by fn_801B6254. Bit 4: set by fn_801B62C4 after it requests the motion of
     * the gun's entry 2; reset by fn_801B6254, and by Update when the gun's motion work locks and
     * bit 0x400000 of its motion player 0's flags (stMtnControlWorkFlag flag 3) is set, which the
     * motion player also sets when a looping motion wraps. Names of the members below inferred
     * from these uses. */
    /* 0x15C */ UnkBitFlags_80014E04 m_liftFlags;
    /* 0x160 */ float m_gunPitch;
    /* 0x164 */ float m_turretYaw;
    /* 0x168 */ float m_speed;
    /* 0x16C */ float m_shotTimer;
    /* The world matrix of the gun. */
    /* 0x170 */ Fn8040Matrix34 m_gunMatrix;
    /* 0x1A0 */ UnkSoundHandle_801D3DC8 m_moveSound;
    /* 0x1BC */ UnkSoundHandle_801D3DC8 m_accelerateSound;
    /* Created with fn_8034DF48(2, -2) at each shot, after the previous one is passed to
     * fn_8034DB60. */
    /* 0x1D8 */ boost::weak_ptr<UnkObject_80169BB0> m_shotObject;
};
CHECK_SIZEOF(Vehicle::GunLift, 0x1E0);

} // namespace Vehicle

#endif
