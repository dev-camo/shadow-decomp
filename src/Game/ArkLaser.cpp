/* The ArkLaser gadget (class described in Game/ArkLaser.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the probe's attack object are boost::function objects built with
 * boost::bind(&ArkLaser::method, this, _1); their manager, invoker and copy functions come from
 * the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ArkLaser.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The beam length at which the model is not scaled: MakeBeamMatrix scales the model along y by
 * the length divided by this (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the unknown expression that
 * initialises this and the two const floats below. The original's static initialiser (USA
 * (GUPE8P) 0x8026EE44) stores 100, 50 and 100 to these three .sbss words at startup (a constant
 * initialiser would give initialised data and no store). The const qualifier is measured on each:
 * without it the initialiser has the same instructions in another order. Of its 22 words, 4
 * differ when only s_modelLength is not const, 3 when only s_radius is not, 2 when only
 * s_callbackValue is not and 7 when none of the three is. Revisit when the original expressions
 * are found. */
static const float s_modelLength = UnkStartupFloat_800E2570(100.0f).value;
/* The radius of the probe (name inferred). */
static const float s_radius = UnkStartupFloat_800E2570(50.0f).value;
/* The direction that effect 0x27 is created with (name inferred). It is copy-constructed from a
 * temporary, as in the original (the call of the copy constructor at USA 0x8026EE80). */
static Fn8040Vector3 s_effectDirection = Fn8040Vector3(0.0f, 0.0f, -1.0f);
/* What fn_8026DBB8 stores in the callback parameter's field_0x4C (placeholder name; the meaning of
 * that member is unknown). */
static const float s_callbackValue = UnkStartupFloat_800E2570(100.0f).value;

/* Builds the matrix of the beam (name inferred): the identity moved to position, turned about y,
 * x and z by rotation and scaled along y by length / s_modelLength. The matrix is written through
 * the pointer (spelling inferred): returning a local matrix by value compiles to a copy that the
 * original function (USA 0x8026ECC4, 0x9C bytes) does not have (0xAC bytes, measured). */
static void MakeBeamMatrix(Fn8040Matrix34 *matrix, const Fn8040Vector3 *position,
                           const UnkRotation3 *rotation, float length) {
    fn_8040C148(matrix, &lbl_805E0720);
    fn_8040B59C(matrix, *position);
    fn_8040B848(matrix, rotation->y);
    fn_8040B884(matrix, rotation->x);
    fn_8040B80C(matrix, rotation->z);
    fn_8040B720(matrix, 1.0f, length / s_modelLength, 1.0f);
}

const char *ArkLaser::s_name = "ArkLaser";

ArkLaser::ArkLaser(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0) {
    m_name = s_name;
    m_effect.reset();

    m_charInfo = fn_8015CD90(0x6E, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, "ARKLASER.BIN");

    UnkRotation3 rotation(fn_8000AB90(90.0f), 0.0f, 0.0f);

    m_probe =
        fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0.5f * m_params.length,
                    s_radius, 0x140000, 7, 0, &lbl_805E07A0, &rotation, 0);

    {
        boost::shared_ptr<CharaColliAttack> attack = fn_800C4374(fn_80021B08(), m_probe).lock();

        attack->fn_80054DD0(4, 0x100, AttackCallback(boost::bind(&ArkLaser::fn_8026DBB8, this, _1)),
                            AttackCallback(boost::bind(&ArkLaser::fn_8026DBB4, this, _1)), 1.0f);
        attack->field_0x54 = 0;
        m_probe->UnkSlot2(-1);
    }

    SetState(1);
}

ArkLaser::~ArkLaser() {
    if (!m_effect.expired()) {
        boost::shared_ptr<EffectElement> effect = m_effect.lock();

        if (effect) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }
        m_effect.reset();
    }
    m_sound.fn_801D455C();
}

/* The descriptor's load hook: adds the model file to the list of resources to load (behaviour
 * inferred). */
extern "C" void fn_8026E404(void) {
    fn_8006B474("ARKLASER.BIN", 1);
}

extern "C" void fn_8026E400(void) {}

void ArkLaser::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ArkLaser(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void ArkLaser::UpdateBody(float dt) {
    switch (m_state) {
    case 1:
        m_timer += dt;
        if (m_timer >= 1.0f) {
            SetState(2);
        }
        break;
    case 2:
        if (stMtnControlWorkFlag(m_model->fn_801602B4(0).lock().get(), 0, 3, 1)) {
            SetState(3);
        }
        break;
    case 3:
        m_timer += dt;
        if (m_timer >= m_params.shotSeconds) {
            SetState(4);
        }
        break;
    case 4:
        if (stMtnControlWorkFlag(m_model->fn_801602B4(0).lock().get(), 0, 3, 1)) {
            SetState(5);
        }
        break;
    case 5:
        m_timer += dt;
        if (m_timer >= m_params.waitSeconds) {
            SetState(1);
        }
        break;
    case 6:
        SetState(1);
        break;
    }

    m_model->fn_80162FB0(dt);
}

void ArkLaser::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    SetState(6);
    UpdateModelMatrix();
    m_probe->SetSize(s_radius, -2, 0.0f, 0.5f * m_params.length);
    m_model->fn_80162EF4();
    m_probe->UnkSlot26();
}

void ArkLaser::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void ArkLaser::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    MakeBeamMatrix(&matrix, &m_charInfo->m_position, &m_charInfo->m_rotation, m_params.length);
    m_model->fn_801633DC(&matrix);
}

void ArkLaser::SetState(int state) {
    switch (m_state) {
    case 3: {
        boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), m_probe).lock();

        attack->field_0x54 = 0;
        m_probe->UnkSlot2(-1);
        m_sound.fn_801D455C();
        break;
    }
    }

    switch (state) {
    case 1:
        m_timer = 0.0f;
        fn_801D54F4(fn_80053164(), 0x7737, &m_charInfo->m_position, 0, 1);
        break;
    case 5:
        m_timer = 0.0f;
        break;
    case 3: {
        boost::shared_ptr<CharaColliAttack> attack = fn_800C42D0(fn_80021B08(), m_probe).lock();

        attack->field_0x54 = 1;
        m_probe->UnkSlot1(-1);
        m_sound = fn_801D3E58(0x7738, &m_charInfo->m_position, this);
        break;
    }
    }

    OnStateChange(m_state, state);
    m_state = state;
}

void ArkLaser::OnStateChange(int previous, int next) {
    switch (previous) {
    case 1: {
        boost::shared_ptr<EffectElement> effect = m_effect.lock();

        if (effect) {
            effect->RequestStop();
        }
        m_effect.reset();
        break;
    }
    case 4:
        m_model->fn_801604FC();
        m_model->fn_80162804();
        break;
    }

    switch (next) {
    case 1:
        if (m_effect.expired()) {
            m_effect = fn_80168C48(&m_charInfo->m_position, &s_effectDirection, 0x27);

            boost::shared_ptr<EffectElement> effect = m_effect.lock();

            if (effect) {
                Fn8040Matrix34 matrix;

                MakeBeamMatrix(&matrix, &m_charInfo->m_position, &m_charInfo->m_rotation,
                               m_params.length);
                fn_80425E68(effect.get(), matrix);
            }
        }
        break;
    case 2:
        m_model->fn_80162A68(0, 0);
        m_model->fn_80160C28(0);
        m_model->fn_80160A84(0, 1);
        UpdateModelMatrix();
        break;
    case 4:
        m_model->fn_80160A84(0, 2);
        break;
    case 6:
        m_model->fn_801604FC();
        m_model->fn_80162804();
        break;
    }
}

void ArkLaser::fn_8026DBB8(AttackCallbackParam &param) {
    Fn8040Vector3 halfAxis(0.0f, 0.5f * m_params.length, 0.0f);
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    halfAxis = matrix * halfAxis;

    UnkSegment_80020A20 axis(m_charInfo->m_position + halfAxis, m_charInfo->m_position - halfAxis);
    Fn8040Vector3 nearest(fn_8040FC3C(&axis, &param.field_0x18));

    param.field_0x34 = param.field_0x18 - nearest;
    fn_8040CE74(&param.field_0x34);
    param.field_0x4C = s_callbackValue;
    param.field_0x50 = 1.0f;
}

void ArkLaser::fn_8026DBB4(AttackCallbackParam &param) {}

static GadgetParamInfo s_paramInfo[] = {
    {4, "Length point", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ShotSec", 0, 0, 0, 2.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "WaitSec", 0, 0, 0, 6.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054CBA8 = {
    "ArkLaser", fn_8026E404, fn_8026E400, ArkLaser::Create, 0, 0, 0x14BE, 1, 0, 10, s_paramInfo,
};
