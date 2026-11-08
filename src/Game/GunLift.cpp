/* The GunLift vehicle (class described in Game/GunLift.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the attack objects and to the camera mode are boost::function objects
 * built with boost::bind(&GunLift::method, this, _1); their manager, invoker and copy functions
 * come from the boost headers. The blocks around locked pointers end where the original destroys
 * them. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/GunLift.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkResult_80051B58.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/VehicleTask.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

static const char *s_expressName = "GunLiftExpress";
static const char *s_localName = "GunLiftLocal";

/* The speeds of the two kinds (names inferred, see GunLiftSpeeds). */
static const float s_expressMaxSpeed = 400.0f;
static const float s_expressAcceleration = 200.0f;
static const float s_expressCruiseSpeed = 100.0f;
static const float s_expressCruiseAcceleration = 50.0f;
static const float s_localMaxSpeed = 200.0f;
static const float s_localAcceleration = 100.0f;
static const float s_localCruiseSpeed = 50.0f;
static const float s_localCruiseAcceleration = 50.0f;

/* Indexed by GunLift::m_kind. */
static const Vehicle::GunLiftSpeeds s_speeds[2] = {
    {&s_expressMaxSpeed, &s_expressAcceleration, &s_expressCruiseSpeed,
     &s_expressCruiseAcceleration},
    {&s_localMaxSpeed, &s_localAcceleration, &s_localCruiseSpeed, &s_localCruiseAcceleration},
};

/* Points in the gun's space (names inferred): Update starts the shots at the first, and
 * OnCameraUpdate places the camera at the second and aims it at the first moved by (0, 1, 0)
 * turned by the gun's rotation. */
static Fn8040Vector3 s_muzzleOffset(0.0f, 2.92f, -55.491f);
static Fn8040Vector3 s_cameraOffset(0.0f, 13.039f, 0.3f);

namespace Vehicle {

GunLift::GunLift(int kind, int type) : TVehicleBase(type), m_kind(kind), m_speeds(&s_speeds[kind]) {
    /* The base constructor has stored 6 in field_0x5C and 2 in field_0x30; the original stores 6
     * again and replaces the 2 with 1. */
    field_0x5C = 6;
    field_0x30 = 1;
}

GunLift::~GunLift() {
    {
        boost::shared_ptr<EffectElement> effect = m_effect.lock();

        if (effect) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }
    }
    m_effect.reset();

    if (!m_cameraRequest.expired()) {
        fn_801CEAD4(m_cameraRequest);
    }
    m_camera.reset();

    m_moveSound.fn_801D455C();
    m_accelerateSound.fn_801D455C();
}

void GunLift::fn_801B634C() {
    m_gunPitch = m_turretYaw = m_speed = m_shotTimer = 0.0f;
    m_liftFlags.Clear();
    fn_801B6504(1.0f);

    m_liftInfo = fn_8015CD90(0x5E, 4);
    m_liftInfo->field_0x30 = 1;
    m_turretInfo = fn_8015CD90(0x5E, 4);
    m_turretInfo->field_0x30 = 1;
    m_gunInfo = fn_8015CD90(0x5E, 4);
    m_gunInfo->field_0x30 = 2;

    m_liftModel = fn_8015F800(1, "GUNLIFT.BIN");
    m_liftModel->fn_80162A68(0, 0);
    m_liftModel->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_liftInfo));

    m_turretModel = fn_8015F800(1, "GUNLIFT.BIN");
    m_turretModel->fn_80162A68(1, 0);
    m_turretModel->fn_80161FC8(1, boost::weak_ptr<CharInfo>(GetSelf()));

    m_gunModel = fn_8015F800(1, "GUNLIFT.BIN");
    m_gunModel->fn_80162A68(2, 0);
    m_gunModel->fn_80161FC8(2, boost::weak_ptr<CharInfo>(m_gunInfo));

    void *collision = fn_8001277C(fn_800122D4(), 1, 0x79);

    fn_801B641C(collision);

    {
        boost::shared_ptr<CharaColliAttack> attack = m_turretModel->fn_80161428(1).lock();

        if (attack) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&GunLift::OnAttack, this, _1)));
            attack->field_0x55 = 0;
        }
    }

    {
        boost::shared_ptr<CharaColliAttack> attack = m_gunModel->fn_80161428(2).lock();

        if (attack) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&GunLift::OnAttack, this, _1)));
            attack->field_0x55 = 0;
        }
    }

    m_liftTarget = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 holder;
        holder.ptr = m_liftTarget;
        m_liftModel->fn_8015FA78(fn_80169D00, &holder);
    }

    m_turretTarget = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 holder;
        holder.ptr = m_turretTarget;
        m_turretModel->fn_8015FA78(fn_80169D00, &holder);
    }

    m_liftTarget->fn_80167010(0x1F4);

    m_camera = fn_80009704(0, 0);
    m_camera->m_callback = CameraUpdateCallback(boost::bind(&GunLift::OnCameraUpdate, this, _1));
}

void GunLift::SetPosition(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    SetUpPath(position);
    UpdateMatrices(0.0f);
}

void GunLift::GetPosition(Fn8040Vector3 *position, UnkRotation3 *rotation) {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_80169EF0(&matrix, m_turretModel, 1, 10);

    if (position != 0) {
        fn_8040FAA0(position, &matrix);
    }
    if (rotation != 0) {
        fn_800493C4(rotation, &matrix);
    }
}

void GunLift::fn_801B62C4() {
    TVehicleBase::fn_801B62C4();

    m_gunPitch = 0.0f;
    m_speed = 0.0f;

    {
        boost::shared_ptr<CharaColliBase> probe = m_turretModel->fn_80161614(1).lock();

        if (probe) {
            boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), probe).lock();

            if (attack) {
                attack->field_0x55 = 1;
            }
        }
    }

    {
        boost::shared_ptr<CharaColliBase> probe = m_gunModel->fn_80161614(2).lock();

        if (probe) {
            boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), probe).lock();

            if (attack) {
                attack->field_0x55 = 1;
            }
        }
    }

    m_cameraRequest = fn_801CED24(m_camera, 8, 0, 0, 0, 0, 1);

    m_gunModel->fn_80160C28(2);
    m_gunModel->fn_8015FF60();
    m_gunModel->fn_80160A84(2, 1);
    m_liftFlags.Set(4);

    m_gunTarget.reset();
    m_gunTarget = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 holder;
        holder.ptr = m_gunTarget;
        m_gunModel->fn_8015FA78(fn_80169D00, &holder);
    }
    m_gunTarget->fn_80167010(0x1F6);

    m_effect = fn_8015D794(&m_position, 0xCC);
    fn_801D54F4(fn_80053164(), 0x7637, &m_gunInfo->m_position, 0, 1);
}

void GunLift::SendStartJump() {
    boost::shared_ptr<CharInfo> character = fn_801B67C4().lock();

    if (character) {
        character->fn_801DC980(new Player::StartJumpCommand(4));
    }
}

void GunLift::fn_801B6254() {
    TVehicleBase::fn_801B6254();

    {
        boost::shared_ptr<EffectElement> effect = m_effect.lock();

        if (effect) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }
    }
    m_effect.reset();
    m_gunTarget.reset();

    m_gunModel->fn_80160670(2);
    m_liftFlags.Reset(4);

    {
        boost::shared_ptr<CharaColliBase> probe = m_turretModel->fn_80161614(1).lock();

        if (probe) {
            boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), probe).lock();

            if (attack) {
                attack->field_0x55 = 0;
            }
        }
    }

    {
        boost::shared_ptr<CharaColliBase> probe = m_gunModel->fn_80161614(2).lock();

        if (probe) {
            boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), probe).lock();

            if (attack) {
                attack->field_0x55 = 0;
            }
        }
    }

    m_gunPitch = 0.0f;
    m_speed = 0.0f;
    fn_801CEAD4(m_cameraRequest);

    m_liftFlags.Reset(0);
    m_liftFlags.Reset(3);
    m_moveSound.fn_801D455C();
    m_accelerateSound.fn_801D455C();
    fn_8034DB60(m_shotObject);
}

void GunLift::Update(float dt) {
    if (field_0x70.Test(0) && fn_800ABDCC(&m_path)) {
        if (m_liftFlags.Test(2)) {
            m_speed += *m_speeds->acceleration * dt;
            m_speed = fn_80070460(m_speed, 0.0f, *m_speeds->maxSpeed);
        } else if (m_speed > *m_speeds->cruiseSpeed) {
            m_speed -= *m_speeds->acceleration * dt;
            m_speed = fn_80070494(m_speed, *m_speeds->cruiseSpeed);
        } else {
            m_speed += *m_speeds->cruiseAcceleration * dt;
            m_speed = fn_80070460(m_speed, 0.0f, *m_speeds->cruiseSpeed);
        }

        if (fn_8001C91C(fn_8001B494(), &m_path, 0, &m_liftInfo->m_position, m_speed * dt, 0, 0)) {
            m_speed = 0.0f;

            if (m_liftFlags.Test(3)) {
                fn_801D54F4(fn_80053164(), 0x7636, &m_turretInfo->m_position, 0, 1);
                m_moveSound.fn_801D455C();
                m_accelerateSound.fn_801D455C();
                m_liftFlags.Reset(3);
            }
        } else {
            if (!m_liftFlags.Test(3)) {
                m_moveSound = fn_801D3DC8(0x7635, &m_liftInfo->m_position, this);
                fn_801D54F4(fn_80053164(), 0x7636, &m_turretInfo->m_position, 0, 1);
                m_liftFlags.Set(3);
            }

            if (m_liftFlags.Test(2) && m_accelerateSound.field_0x0 == 0) {
                m_accelerateSound = fn_801D3DC8(0x7647, &m_liftInfo->m_position, this);
            }
        }

        if (m_liftFlags.Test(4)) {
            boost::shared_ptr<_stMtnPlayWork> motion = m_gunModel->fn_801602B4(2).lock();

            if (motion) {
                if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                    m_liftFlags.Reset(4);
                } else {
                    _stMtnPlayWork *control = motion.get();
                    unsigned short eventId;
                    void *payload;

                    while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                        if (eventId == 0x5001) {
                            u32 *sound = static_cast<u32 *>(payload);

                            fn_801D54F4(fn_80053164(), static_cast<u16>(*sound),
                                        &m_gunInfo->m_position, 0, 1);
                        }
                        control = 0;
                    }
                }
            }
        }
    }

    UpdateMatrices(dt);

    if (field_0x70.Test(0)) {
        Fn8040Vector3 muzzle;
        Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);

        muzzle = m_gunMatrix * s_muzzleOffset;
        direction = fn_8040B618(m_gunMatrix, direction);

        UnkParams_80051D10 params(fn_801B66EC(), 0, muzzle, direction, m_liftInfo->field_0x18);
        UnkResult_80051B58 result = fn_80051AF0(0x1C, params, 5.0f);

        if (result.field_0x34 != 0) {
            boost::shared_ptr<EffectElement> effect = m_effect.lock();

            if (effect) {
                Fn8040Matrix34 matrix(m_gunMatrix);

                fn_8040FA20(&matrix, &result.field_0x0);
                effect->ClearFlags(Effect::System::ELEMENT_FLAG_8);
                fn_80425E68(effect.get(), matrix);
            }
        } else {
            boost::shared_ptr<EffectElement> effect = m_effect.lock();

            if (effect) {
                effect->SetFlags(Effect::System::ELEMENT_FLAG_8);
            }
        }

        if (m_liftFlags.Test(0)) {
            m_shotTimer -= dt;

            if (m_shotTimer < 0.0f) {
                fn_80051C34(0x1C, params);
                m_gunTarget->fn_80167448(0x1F7);
                fn_801D54F4(fn_80053164(), 0x200B, &muzzle, 0, 1);
                m_shotTimer = 0.0625f;
                fn_8034DB60(m_shotObject);
                m_shotObject = fn_8034DF48(2, -2);
            }
        }
    }

    m_liftModel->fn_80162FB0(dt);
    m_turretModel->fn_80162FB0(dt);
    m_gunModel->fn_80162FB0(dt);
    m_liftTarget->fn_8016761C(dt);
    m_turretTarget->fn_8016761C(dt);
    if (m_gunTarget.get() != 0) {
        m_gunTarget->fn_8016761C(dt);
    }
}

/* The filter that SetUpPath passes to fn_8001CDA0: accepts the paths whose word has bit 22 set
 * (meaning unknown). */
static bool fn_8031675C(const UnkPathFilterRecord_8001CDA0 *record) {
    return (record->field_0x0 & 0x400000) != 0;
}

void GunLift::SetUpPath(const Fn8040Vector3 *position) {
    fn_8001CDA0(fn_8001B494(), &m_path, 0.0f, position, &lbl_805E07A0, 0, fn_8031675C, 0);

    if (fn_800ABDCC(&m_path)) {
        fn_80018304(&m_path);
    }
}

void GunLift::UpdateMatrices(float dt) {
    if (fn_800ABDCC(&m_path)) {
        Fn8040Vector3 previous(m_liftInfo->m_position);

        m_liftInfo->m_position = fn_80018634(&m_path);
        m_liftInfo->m_rotation = fn_80018434(&m_path, 1, 0);

        if (fn_8000A950(dt)) {
            fn_8040CE60(&m_liftInfo->field_0x18);
        } else {
            m_liftInfo->field_0x18 = m_liftInfo->m_position - previous;
            m_liftInfo->field_0x18 = m_liftInfo->field_0x18 * (1.0f / dt);
        }

        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_liftInfo->m_position);
        fn_8040B848(&matrix, m_liftInfo->m_rotation.y);
        m_liftModel->fn_801633DC(&matrix);
    }

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_80169EF0(&matrix, m_liftModel, 0, 1);
    fn_8040FAA0(&m_turretInfo->m_position, &matrix);
    fn_800493C4(&m_turretInfo->m_rotation, &matrix);
    m_turretInfo->m_rotation.y += m_turretYaw;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_turretInfo->m_position);
    fn_8040B80C(&matrix, m_turretInfo->m_rotation.z);
    fn_8040B884(&matrix, m_turretInfo->m_rotation.x);
    fn_8040B848(&matrix, m_turretInfo->m_rotation.y);
    m_turretModel->fn_801633DC(&matrix);

    fn_80169EF0(&m_gunMatrix, m_turretModel, 1, 2);
    fn_8040FAA0(&m_gunInfo->m_position, &m_gunMatrix);
    fn_800494F8(&m_gunInfo->m_rotation, &m_gunMatrix);
    m_gunInfo->m_rotation.x += m_gunPitch;

    fn_8040C148(&m_gunMatrix, &lbl_805E0720);
    fn_8040B59C(&m_gunMatrix, m_gunInfo->m_position);
    fn_8040B848(&m_gunMatrix, m_gunInfo->m_rotation.y);
    fn_8040B884(&m_gunMatrix, m_gunInfo->m_rotation.x);
    fn_8040B80C(&m_gunMatrix, m_gunInfo->m_rotation.z);
    m_gunModel->fn_801633DC(&m_gunMatrix);

    TVehicleBase::SetPosition(&m_turretInfo->m_position, &m_turretInfo->m_rotation);
}

void GunLift::OnCommand(CharCommand *command) {
    switch (command->m_type) {
    case 1: /* AccelerateInputCommand */ {
        AccelerateInputCommand *input = static_cast<AccelerateInputCommand *>(command);

        m_gunPitch += 1.5f * DegreesToRadians(input->m_vector.z);
        m_turretYaw -= 1.5f * DegreesToRadians(input->m_vector.x);
        m_gunPitch = fn_80070460(m_gunPitch, DegreesToRadians(-10.0f), DegreesToRadians(60.0f));
        command->m_flags |= 2;
        break;
    }
    case 0x15: /* ControlVehicleCommand */
        switch (static_cast<ControlVehicleCommand *>(command)->m_control) {
        case 1:
            SendStartJump();
            command->m_flags |= 2;
            break;
        case 6:
            m_liftFlags.Set(0);
            m_shotTimer = 0.0f;
            command->m_flags |= 2;
            break;
        case 7:
            m_liftFlags.Reset(0);
            fn_8034DB60(m_shotObject);
            command->m_flags |= 2;
            break;
        case 4:
            SendStartJump();
            command->m_flags |= 2;
            break;
        case 2:
            m_liftFlags.Set(2);
            break;
        case 3:
            m_liftFlags.Reset(2);
            m_accelerateSound.fn_801D455C();
            break;
        }
        break;
    }
}

void GunLift::OnCameraUpdate(CameraUpdateEventArgs &args) {
    if (m_camera.get() != 0) {
        Fn8040Vector3 eye;

        eye = m_gunMatrix * s_cameraOffset;
        fn_8000DB78(m_camera.get(), &eye);

        Fn8040Vector3 target;

        target = m_gunMatrix * s_muzzleOffset;

        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

        up = fn_8040B618(m_gunMatrix, up);
        AddVector(&target, up);
        fn_8000DB50(m_camera.get(), &target);
    }
}

void GunLift::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (field_0x70.Test(0)) {
        param.field_0x54 = 1;

        boost::shared_ptr<CharInfo> character = fn_801B67C4().lock();

        if (character) {
            character->fn_801DC980(new DamageCommand(1.0f, Fn8040Vector3(0.0f, 1.0f, 0.0f), 4));
        }
        m_turretTarget->fn_80167448(0x1F5);
    }
}

} // namespace Vehicle

extern "C" void fn_803160B8(void) {
    fn_801B4850(12);
    fn_801B4850(13);
    fn_8006B474("GUNLIFT.BIN", 1);
}

extern "C" void fn_803160B4(void) {}

namespace Vehicle {

TVehicleBase *GunLift::CreateExpress(int type) {
    return new GunLift(0, type);
}

TVehicleBase *GunLift::CreateLocal(int type) {
    return new GunLift(1, type);
}

bool GunLift::fn_801B6244() {
    return true;
}

bool GunLift::fn_801B624C() {
    return true;
}

} // namespace Vehicle

/* The descriptor named "GunLift" has no create function: fn_803160B8 sets the flag bytes of the
 * vehicle types 12 and 13, whose entries in the vehicle type table point to the two records
 * below, and adds GUNLIFT.BIN to the resource list (fn_8006B474). */
GadgetTypeInfo lbl_8055C06C = {"GunLift", fn_803160B8, fn_803160B4, 0, 0, 0x80, 0x1518, 4, 0, 0, 0};

/* The records of the vehicle types "GunLiftExpress" (12) and "GunLiftLocal" (13). The names are
 * copied in by the static initialiser. */
Vehicle::VehicleTypeInfo lbl_805ED118 = {s_expressName, Vehicle::GunLift::CreateExpress};
Vehicle::VehicleTypeInfo lbl_805ED120 = {s_localName, Vehicle::GunLift::CreateLocal};
