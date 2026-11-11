/* The SecurityLaser gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The attack object's two callbacks are boost::function objects built with
 * boost::bind(&SetSecurityLaser::OnAttackCallback0, this, _1) and the same for OnAttackCallback1;
 * their manager, invoker and copy functions come from the boost headers. */
/* TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names appear in the unit's
 * .rodata (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/SecurityLaser.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkResult_80051B58.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>
#include <cstdio>

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliBase;

static const char *s_modelFile = "SECURITYLASER.BIN";

/* The default of the parameter "length", the distance by which the beam stops short of its end or
 * of the object it hits, and a third value that nothing in this file reads (names inferred).
 * Matching workaround: the original sets these floats (and the table entry that shows the
 * default) with a start-up function in .ctors (a constant initialiser would be emitted as
 * initialised .sdata data instead). UnkStartupFloat_800E2570 stands in for the original
 * initialiser expression and gives that start-up function. The const qualifier lets the
 * constructor load s_defaultLength before the store to m_modelIndex, as the original does; the
 * load of a non-const object waits for that store, which may alias it. Revisit when the original
 * initialiser is found. */
static const float s_defaultLength = UnkStartupFloat_800E2570(100.0f).value;
static const float s_beamMargin = UnkStartupFloat_800E2570(10.0f).value;
static const float s_unusedValue = UnkStartupFloat_800E2570(3.0f).value;

/* The probe predicate of UpdateBeam: skips the gadget's own collision object, which the probe
 * passes as the context (name inferred). */
static bool IsNotOwnColli(const UnkProbeFilterRecord *record) {
    if (record->target != 0 && record->target == record->context) {
        return false;
    }
    return true;
}

namespace SecurityLaser {

const char *SetSecurityLaser::s_name = "SetSecurityLaser";

SetSecurityLaser::SetSecurityLaser(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_modelIndex(0), m_length(s_defaultLength), m_rotTime(0.0f),
      m_rotY(0.0f), m_moveTime(0.0f), m_moveX(0.0f), m_moveY(0.0f), m_moveZ(0.0f), m_type(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x78, 4);
    ReadOwner(owner);

    m_loadedModelIndex = m_modelIndex;
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(m_loadedModelIndex, 0);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, s_modelFile));
    m_beamModel = fn_8015F800(1, s_modelFile);
    m_beamModel->fn_80162A68(2, 0);
    m_endModel = fn_8015F800(1, s_modelFile);
    m_endModel->fn_80162A68(3, 0);

    m_model->fn_80161FC8(m_loadedModelIndex, boost::weak_ptr<CharInfo>(m_charInfo));
    m_colli = m_model->fn_80161614(m_loadedModelIndex);

    m_attack = fn_8005A300(
        1, boost::weak_ptr<CharInfo>(m_charInfo),
        AttackCallback(boost::bind(&SetSecurityLaser::OnAttackCallback0, this, _1)),
        AttackCallback(boost::bind(&SetSecurityLaser::OnAttackCallback1, this, _1)), 1.0f);

    if (m_flags.Test(1)) {
        Fn8040Vector3 position = m_charInfo->m_position;

        Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);

        m_effect = fn_80168C48(&position, &direction, 0x4D);
    }

    FindLoopSound();
    UpdateModelMatrices();
}

SetSecurityLaser::~SetSecurityLaser() {
    if (!m_effect.expired()) {
        boost::shared_ptr<EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }

    if (m_attack.get() != 0) {
        m_attack.reset();
    }
}

void SetSecurityLaser::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_beamModel.get() != 0) {
        m_beamModel->fn_80162FB0(dt);
    }
    if (m_endModel.get() != 0) {
        m_endModel->fn_80162FB0(dt);
    }

    UpdateMotion(dt);
    UpdateModelMatrices();
}

void SetSecurityLaser::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrices();

    if (m_model.get() != 0) {
        if (m_modelIndex != m_loadedModelIndex) {
            m_model->fn_80161880();
            m_model->fn_80162804();
            m_loadedModelIndex = m_modelIndex;
            m_model->fn_80162A68(m_loadedModelIndex, 0);
            m_model->fn_80161FC8(m_loadedModelIndex, boost::weak_ptr<CharInfo>(m_charInfo));
            m_colli = m_model->fn_80161614(m_loadedModelIndex);
        }

        /* Draws the beam at its full length and the line along which the gadget moves. */
        Fn8040Vector3 end(0.0f, 0.0f, m_length);
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(m_charInfo->m_position, end), lbl_805F06D8);

        Fn8040Vector3 move(m_moveX, m_moveY, m_moveZ);
        Fn8040Vector3 back = fn_80016284(move);
        Fn8040Vector3 lineStart = m_charInfo->m_position + move;
        Fn8040Vector3 lineEnd = m_charInfo->m_position + back;
        fn_802E0B58(UnkSegment_80020A20(lineStart, lineEnd), lbl_805F06D8);

        m_model->fn_8016108C();
    }
}

void SetSecurityLaser::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    /* The nine parameters, m_modelIndex to m_type. */
    fn_800C9DBC(owner, &m_modelIndex, 0x24);

    m_moveTimer = 0.0f;
    m_moveSpeed = (m_moveTime != 0.0f) ? 360.0f / m_moveTime : 0.0f;
    m_basePosition = m_charInfo->m_position;

    m_rotTimer = 0.0f;
    m_rotSpeed = (m_rotTime != 0.0f) ? 360.0f / m_rotTime : 0.0f;
    m_angle = 0.0f;
    m_beamFrame = 0;
    m_beamLength = 0.0f;

    m_flags.Clear();
    switch (m_type) {
    case 0:
        m_flags.Set(0);
        m_flags.Set(1);
        break;
    case 1:
        break;
    case 2:
        m_flags.Set(1);
        break;
    }

    m_loopSoundId = -1;
}

void SetSecurityLaser::UpdateModelMatrices() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }

    UpdateBeam();

    if (m_beamFrame == 0) {
        UnkFrameSearch_80169E8C search;

        search.id = 1;
        m_model->fn_8015FA78(fn_80169E8C, &search);
        m_beamFrame = search.frame;
    } else if (m_beamModel.get() != 0 && m_endModel.get() != 0) {
        RwMatrix *ltm = RwFrameGetLTM(m_beamFrame);
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B52C(&matrix, ltm->pos.x, ltm->pos.y, ltm->pos.z);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B848(&matrix, m_angle);
        fn_8040B720(&matrix, 1.0f, 1.0f, 0.1f * m_beamLength);
        m_beamModel->fn_801633DC(&matrix);

        fn_8040B6F8(&matrix);
        fn_8040B52C(&matrix, ltm->pos.x, ltm->pos.y, ltm->pos.z);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B848(&matrix, m_angle);
        fn_8040B52C(&matrix, 0.0f, 0.0f, m_beamLength);
        m_endModel->fn_801633DC(&matrix);

        /* An original quirk: the u16 id is compared with -1, so the test always passes. */
        if (m_flags.Test(1) && m_loopSoundId != -1) {
            Fn8040Vector3 position(0.0f, 0.0f, 0.0f);

            position = matrix * position;
            fn_801D53C4(fn_80053164(), m_loopSoundId, &position, this, 3, 0, 2);
        }
    }
}

void SetSecurityLaser::UpdateMotion(float dt) {
    if (m_moveTime > 0.0f) {
        m_moveTimer += dt;
        if (m_moveTimer > m_moveTime) {
            m_moveTimer = 0.0f;
        }

        float phase = m_moveSpeed * m_moveTimer;
        Fn8040Vector3 move(m_moveX, m_moveY, m_moveZ);
        float distance = fn_8040CF50(move);

        move = fn_8040CEE8(move);
        /* fn_8000AB90 converts degrees to radians as DegreesToRadians does; the original calls it
         * here and converts the swing's angles below in place. */
        fn_8040CE38(&move, distance * std::sin(fn_8000AB90(phase)));
        m_charInfo->m_position = m_basePosition + move;
    }

    if (m_rotTime > 0.0f) {
        m_rotTimer += dt;
        if (m_rotTimer > m_rotTime) {
            m_rotTimer = 0.0f;
        }

        float amplitude = DegreesToRadians(0.5f * m_rotY);

        m_angle = amplitude * std::cos(DegreesToRadians(m_rotSpeed * m_rotTimer));
    }
}

void SetSecurityLaser::UpdateBeam() {
    if (m_beamFrame == 0) {
        return;
    }

    m_beamLength = m_length;

    /* Set when the beam ends at an object it hit, and always for type 2 ("Fixed w/Eff"); the effect
     * at the beam's end has flag 8 cleared then and set otherwise (the flag's meaning is unknown). */
    bool hit = false;
    RwMatrix *ltm = RwFrameGetLTM(m_beamFrame);
    Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
    Fn8040Vector3 end(0.0f, 0.0f, m_beamLength);
    Fn8040Matrix34 matrix;

    fn_8040B6F8(&matrix);
    fn_8040B52C(&matrix, ltm->pos.x, ltm->pos.y, ltm->pos.z);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    fn_8040B848(&matrix, m_angle);
    start = matrix * start;
    end = matrix * end;

    if (m_flags.Test(0)) {
        UnkSegment_80020A20 segment(start, end);
        boost::shared_ptr<CharaColliBase> colli(m_colli.lock());

        if (colli.get() != 0) {
            UnkProbeNode_80021FE4 *node =
                fn_80021E24(fn_80020D40(), &segment, 3, 8, IsNotOwnColli, colli.get());

            if (node != 0) {
                Fn8040Vector3 hitPosition = *fn_8001E9BC(node->hit);
                const Fn8040Vector3 &difference = start - hitPosition;
                m_beamLength = fn_8040CF50(difference);
                end = hitPosition;
                hit = true;
                delete node;
            }
        }
    }

    m_beamLength -= s_beamMargin;
    if (m_beamLength < 0.0f) {
        m_beamLength = 0.0f;
    }

    fn_8005AFE4(7, &start, &end, m_attack);

    if (!m_flags.Test(0) && m_flags.Test(1)) {
        hit = true;
    }

    if (!m_effect.expired()) {
        boost::shared_ptr<EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            fn_80425E4C(effect.get(), &end);
            if (hit) {
                effect->ClearFlags(Effect::System::ELEMENT_FLAG_8);
            } else {
                effect->SetFlags(Effect::System::ELEMENT_FLAG_8);
            }
        }
    }
}

void SetSecurityLaser::FindLoopSound() {
    if (m_model.get() != 0) {
        const char *modelName = fn_801626D8(m_model.get());

        m_loopSoundId = -1;
        if (modelName != 0) {
            char soundName[64];

            sprintf(soundName, "%s_lp", modelName);

            UnkResourceRecord_80208848 *record = fn_80208888(m_resource, soundName);

            if (record != 0) {
                m_loopSoundId = record->field_0x8;
            }
        }
    }
}

void SetSecurityLaser::OnAttackCallback0(AttackCallbackParam &param) {}

void SetSecurityLaser::OnAttackCallback1(AttackCallbackParam &param) {}

} // namespace SecurityLaser

static const char *s_typeLabels[3] = {"Normal", "Fixed", "Fixed w/Eff"};

/* The label callback of the "type" entry, the ninth of the table (name of the function unknown). */
static const char *fn_8022C4C4(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 8) {
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

/* Uses the literal: the original takes the string's address here instead of loading s_modelFile. */
extern "C" void fn_8022C498(void) {
    fn_8006B474("SECURITYLASER.BIN", 1);
}

extern "C" void fn_8022C494(void) {}

void SecurityLaser::SetSecurityLaser::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetSecurityLaser(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "length", 0, 0, 0, s_defaultLength, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "rot time(sec)", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "rot Y(deg)", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "move sec", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {1, "type", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_8022C4C4, 0},
    {0},
};

GadgetTypeInfo lbl_80546DEC = {
    "SECURITY LASER",
    fn_8022C498,
    fn_8022C494,
    SecurityLaser::SetSecurityLaser::Create,
    0,
    0,
    0x13EF,
    1,
    0,
    10,
    s_paramInfo,
};
