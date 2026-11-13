/* The WATCH CAMERA gadget: WatchCamera::SetWatchCamera with its Monitor and Sight (class names from
 * RTTI; behaviour in the class comments in the header). File name inferred from the classes.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/WatchCamera.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/AngleConversion.h"
#include "Game/MissionClearCollision.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSingleton_8000DCC0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace WatchCamera {

/* The probe filter of CanSeeTarget: accepts the objects without flag 0x80000 (name inferred; what
 * the flag marks is not known). */
static bool IsSightBlocker(const UnkProbeFilterRecord *record) {
    return !(record->flags & 0x80000);
}

const char *SetWatchCamera::s_name = "SetWatchCamera";

SetWatchCamera::SetWatchCamera(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x87, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(1, "WATCHCAMERA.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80160A84(0, 1);

    {
        /* Copy-initialised from a zero temporary, as in the original: the zero vector is built in
         * its own stack slot and copied into halfExtents with the vector's out-of-line copy
         * constructor (0x800091EC). */
        Fn8040Vector3 halfExtents = Fn8040Vector3(0.0f, 0.0f, 0.0f);

        boost::shared_ptr<CharaColliBase> probe = fn_800C7D04(
            fn_80021B08(), &m_charInfo->m_position, halfExtents, m_charInfo->m_rotation, 2, 0x40100,
            10, 0, boost::weak_ptr<CharInfo>(lbl_805EE698), lbl_805E07A0, &lbl_805E07B8, 0);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&SetWatchCamera::OnCollision, this, _1)));
        fn_8015BDC4(m_handle.get(), 2, 1);
        UpdateProbe();
    }

    m_monitor = boost::shared_ptr<Monitor>(new Monitor());
    m_monitor->Hide();

    Fn8040Vector3 sightPosition(m_params.lookPosX, m_params.lookPosY, m_params.lookPosZ);

    AddVector(&sightPosition, m_charInfo->m_position);
    m_sightPosition = sightPosition;
    m_sight = boost::shared_ptr<Sight>(new Sight(&m_sightPosition));
    m_sight->Hide();

    m_motion = m_model->fn_801602B4(0);
    m_flags.Set(4);
    UpdateModelMatrix();
}

SetWatchCamera::~SetWatchCamera() {
    StopCamera();
}

void SetWatchCamera::UpdateBody(float dt) {
    m_flags.Reset(1);
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
    if (m_monitor.get() != 0) {
        m_monitor->Update(dt);
    }

    if (m_flags.Test(0)) {
        m_shotTimer -= dt;
        if (m_shotTimer <= 0.0f) {
            m_shotTimer = Shoot();
        }
    }

    if (m_sight.get() != 0) {
        m_sight->Update();
    }

    /* Stops the camera in the first update in which OnCollision, called from fn_8015BE9C above,
     * has not set bit 1, but only while bit 2 is set (ReadOwner clears it without stopping the
     * camera). */
    if (!m_flags.Test(1) && m_flags.Test(2)) {
        StopCamera();
        if (m_model.get() != 0) {
            m_model->fn_8016271C();
        }
        m_state = 1;
        m_flags.Reset(0);
        m_flags.Reset(2);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        m_sightState = 0;
        if (m_sight.get() != 0) {
            m_sight->SetSearchEffect();
        }
        break;
    case 1:
        break;
    case 2:
        if (CanSeeTarget()) {
            m_timer = 2.0f;
            m_state = 3;
            m_sightSpeed = 0.0f;
            if (m_sight.get() != 0) {
                m_sight->SetFoundEffect();
            }
        }
        WanderSight();
        break;
    case 3:
        if (!CanSeeTarget()) {
            m_state = 2;
            m_sightState = 0;
            if (m_sight.get() != 0) {
                m_sight->SetSearchEffect();
            }
        } else {
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                m_timer = 1.0f;
                m_state = 4;
                m_flags.Set(0);
            }
        }
        MoveSightToTarget();
        break;
    case 4:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = 2;
            m_sightState = 0;
            m_flags.Reset(0);
            if (m_sight.get() != 0) {
                m_sight->SetSearchEffect();
            }
        }
        break;
    case 5:
        break;
    }

    UpdateModelMatrix();

    if (m_flags.Test(4)) {
        if (boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock()) {
            /* fn_80415A18 copies the event's data here; its first word is a sound id. The size of
             * the buffer (8 bytes) is inferred from the stack frame. */
            u32 payload[2];

            if (fn_80415A18(motion.get(), 0, 0x5001, payload, 0) != 0) {
                fn_801D54F4(fn_80053164(), payload[0], &m_charInfo->m_position, 0, 1);
            }
        }
    }
}

void SetWatchCamera::fn_800CBC1C(float dt) {
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }

    Fn8040Vector3 lookPosition(m_params.lookPosX, m_params.lookPosY, m_params.lookPosZ);

    AddVector(&lookPosition, m_charInfo->m_position);
    fn_8015F5D8(&lookPosition, 20.0f);
}

void SetWatchCamera::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
    UpdateProbe();
}

void SetWatchCamera::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
    m_timer = 0.0f;
    m_shotTimer = 0.0f;
    m_flags.Clear();
    m_sightSpeed = 0.0f;
    m_sightState = 0;
    fn_8040CE60(&m_sightVelocity);
}

void SetWatchCamera::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

void SetWatchCamera::UpdateProbe() {
    if (m_handle.get() != 0) {
        Fn8040Vector3 position(m_params.posX, m_params.posY, m_params.posZ);

        AddVector(&position, m_charInfo->m_position);

        UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

        rotation.x = DegreesToRadians(m_params.angX);
        rotation.y = DegreesToRadians(m_params.angY);
        rotation.z = DegreesToRadians(m_params.angZ);

        m_handle->m_probe->SetPosition(&position);
        m_handle->m_probe->SetRotation(rotation);
        m_handle->m_probe->SetSize(m_params.width, -2, m_params.height, m_params.depth);
    }
}

void SetWatchCamera::OnCollision(ColliDetectArgs &args) {
    if (fn_8015AC84(args)) {
        if (!m_flags.Test(2)) {
            StartCamera();
            if (m_model.get() != 0) {
                m_model->fn_80162790();
            }
            m_state = 2;
            m_flags.Reset(0);
            m_flags.Set(2);
        }
        m_flags.Set(1);
    }
}

void SetWatchCamera::StartCamera() {
    m_cameraMode = fn_80009704(0, 0);
    m_cameraMode->m_callback =
        CameraUpdateCallback(boost::bind(&SetWatchCamera::OnCameraUpdate, this, _1));
    m_cameraRequest = fn_801CED24(m_cameraMode, 0, 0, 0, 0, 0, 0);

    if (m_monitor.get() != 0) {
        m_monitor->Show();
    }
    if (m_sight.get() != 0) {
        m_sight->Show();
    }
}

void SetWatchCamera::StopCamera() {
    fn_801CEAD4(m_cameraRequest);
    if (m_cameraMode.get() != 0) {
        m_cameraMode.reset();
    }
    if (m_monitor.get() != 0) {
        m_monitor->Hide();
    }
    if (m_sight.get() != 0) {
        m_sight->Hide();
    }
}

void SetWatchCamera::OnCameraUpdate(CameraUpdateEventArgs &args) {
    if (m_cameraMode.get() != 0) {
        fn_8000DB78(m_cameraMode.get(), &m_charInfo->m_position);

        Fn8040Vector3 lookPosition(m_params.lookPosX, m_params.lookPosY, m_params.lookPosZ);

        AddVector(&lookPosition, m_charInfo->m_position);
        fn_8000DB50(m_cameraMode.get(), &lookPosition);
    }
}

bool SetWatchCamera::CanSeeTarget() {
    bool visible = true;
    boost::weak_ptr<CharInfo> target = fn_800A8B74(fn_80074194(), lbl_805EE588);

    if (boost::shared_ptr<CharInfo> targetInfo = target.lock()) {
        Fn8040Vector3 start = m_charInfo->m_position;
        Fn8040Vector3 end = targetInfo->m_position;
        m_targetPosition = targetInfo->m_position;

        UnkSegment_80020A20 segment(start, end);
        UnkProbeNode_80021FE4 *node =
            fn_80021E24(fn_80020D40(), &segment, 3, 13, IsSightBlocker, 0);

        if (node != 0) {
            if (node->hit != 0) {
                visible = false;
            }
            delete node;
        }
    }
    return visible;
}

float SetWatchCamera::Shoot() {
    Fn8040Vector3 position = m_charInfo->m_position;
    position.y += -20.0f;

    Fn8040Vector3 direction = m_sight->m_position - position;
    fn_8040CE74(&direction);

    UnkParams_80051D10 params(boost::weak_ptr<CharInfo>(m_charInfo), 1, position, direction,
                              Fn8040Vector3(0.0f, 0.0f, 0.0f));

    fn_80051C34(0x1C, params);
    fn_801D560C(fn_80053164(), 0x7840, 0, 0);
    return 1.0f / 12.0f;
}

void SetWatchCamera::WanderSight() {
    switch (m_sightState) {
    case 0: {
        boost::weak_ptr<CharInfo> center = fn_800A8C54(fn_80074194(), 0);
        /* Uninitialised when there is no such character info (original behaviour). */
        Fn8040Vector3 centerPosition;

        if (!center.expired()) {
            if (boost::shared_ptr<CharInfo> centerInfo = center.lock()) {
                centerPosition = centerInfo->m_position;
            }
        }

        /* The full turn and the largest distance, scaled by random fractions. */
        float yaw = 360.0f;
        float distance = 60.0f;

        yaw *= fn_8040ECF8();
        distance *= fn_8040ECF8();

        Fn8040Vector3 offset(0.0f, 0.0f, distance);
        Fn8040Matrix34 rotation;

        fn_8040B6F8(&rotation);
        fn_8040B848(&rotation, DegreesToRadians(yaw));
        offset = rotation * offset;
        m_sightGoal = centerPosition + offset;
        m_sightVelocity = m_sightGoal - m_sightPosition;
        m_sightVelocity = fn_8040CEE8(m_sightVelocity);
        fn_8040CE38(&m_sightVelocity, 0.8f);
        m_sightState = 1;
        break;
    }
    case 1: {
        fn_8040CFF8(&m_sightPosition, &m_sightVelocity);
        const Fn8040Vector3 &difference = m_sightPosition - m_sightGoal;
        if (fn_8040CF50(difference) <= 5.0f) {
            m_sightState = 0;
        }
        break;
    }
    }

    if (m_sight.get() != 0) {
        m_sight->SetPosition(&m_sightPosition);
    }
}

void Sight::SetPosition(const Fn8040Vector3 *position) {
    m_position = *position;
}

void SetWatchCamera::MoveSightToTarget() {
    if (m_sight.get() != 0) {
        Fn8040Vector3 direction = m_targetPosition - m_sightPosition;
        m_sightSpeed += 0.02f;
        if (m_sightSpeed > 1.0f) {
            m_sightSpeed = 1.0f;
        }
        AddVector(&m_sightPosition, direction * m_sightSpeed);
        m_sight->SetPosition(&m_sightPosition);
    }
}

Monitor::Monitor() {
    m_model = fn_8015F800(1, "WATCHCAMERA.BIN");
    m_model->fn_80162A68(1, 0);
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
Monitor::~Monitor() {}
#pragma auto_inline reset

void Monitor::Update(float dt) {
    if (m_model.get() != 0) {
        float distance = 1.1f;
        Fn8040Matrix34 matrix;
        Fn8040Vector3 matrixPosition;

        fn_8040C148(&matrix, GetEntryMatrix(fn_80009548(), 0));
        fn_8040FAA0(&matrixPosition, &matrix);
        Fn8040Vector3 direction = *(fn_800101F8(fn_80009548(), 0)) - matrixPosition;
        fn_8040CE74(&direction);
        fn_8040CE38(&direction, distance);
        const Fn8040Vector3 &modelPosition = matrixPosition + direction;
        fn_8040FA20(&matrix, modelPosition);

        UnkRecord_8000DCC0 *record = fn_8000DCC0()->field_0x0;

        fn_8040B720(&matrix, 2.0f * (record->field_0x68 * distance),
                    2.0f * (record->field_0x6C * distance), 1.0f);
        m_model->fn_801633DC(&matrix);
    }
}

void Monitor::Hide() {
    if (m_model.get() != 0) {
        m_model->fn_80162790();
    }
}

void Monitor::Show() {
    if (m_model.get() != 0) {
        m_model->fn_8016271C();
    }
}

Sight::Sight(const Fn8040Vector3 *position) {
    m_position = *position;

    Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

    m_effect = fn_80168C48(position, &up, 0x6A);
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->SetTranslationSource(&m_position);
    }
}

/* Matching workaround: auto_inline around this destructor, as for Monitor's above. */
#pragma auto_inline on
Sight::~Sight() {
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
    m_effect.reset();
}
#pragma auto_inline reset

void Sight::Hide() {
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->SetFlags(Effect::System::ELEMENT_FLAG_8);
    }
    m_flags.Set(3);
}

void Sight::Show() {
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->ClearFlags(Effect::System::ELEMENT_FLAG_8);
    }
    m_flags.Reset(3);
}

void Sight::SetFoundEffect() {
    SetEffect(0x6F);
    if (m_flags.Test(3)) {
        Hide();
    }
}

void Sight::SetSearchEffect() {
    SetEffect(0x6A);
    if (m_flags.Test(3)) {
        Hide();
    }
}

void Sight::SetEffect(int id) {
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
    m_effect.reset();

    Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

    m_effect = fn_80168C48(&m_position, &up, id);
    if (boost::shared_ptr<EffectElement> effect = m_effect.lock()) {
        effect->SetTranslationSource(&m_position);
    }
}

} // namespace WatchCamera

extern "C" void fn_80267D90(void) {
    fn_8006B474("WATCHCAMERA.BIN", 1);
}

extern "C" void fn_80267D8C(void) {}

void WatchCamera::SetWatchCamera::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 14);

    new SetWatchCamera(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "posX", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "posY", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "posZ", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "angX", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "angY", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "angZ", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "width", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "height", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "depth", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "lookPosX", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "lookPosY", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "lookPosZ", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "lookAngX", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "lookAngY", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "lookAngZ", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054C5A8 = {
    "WATCH CAMERA",
    fn_80267D90,
    fn_80267D8C,
    WatchCamera::SetWatchCamera::Create,
    0,
    0,
    0x1770,
    1,
    0,
    10,
    s_paramInfo,
};
