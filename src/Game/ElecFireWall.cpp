/* ElecFireWall (class name from RTTI; the gadget's descriptor is "ELEC_FIREWALL"): a fire wall of
 * the electric circuit area. File name inferred from the class. Method names marked "(name
 * inferred)" are not original. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
/* CharaColliBaseCharInfo.h comes before TNumEdit.h, in the order their names appear in the unit's
 * .rodata. */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/TNumEdit.h"
#include "Game/ElecFireWall.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetAccelerate.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkPartFrameSearch.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The part callback (placeholder name): stores the frame of the clump of the part whose word at
 * 0x8 equals the searched id and stops the walk once a frame is found (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_801F7778(UnkModelPart_8015FA78 *part, void *userData) {
    UnkPartFrameSearch *search = static_cast<UnkPartFrameSearch *>(userData);

    if (search != 0 && part->clump != 0 && part->field_0x8 == search->id) {
        search->frame = static_cast<RwFrame *>(part->clump->object.parent);
        if (search->frame != 0) {
            return 0;
        }
    }
    return part;
}

const char *ElecFireWall::s_name = "ElecFireWall";

/* Three factors that ApplyScale multiplies by the "Scale" parameters before passing them to the
 * model (placeholder name; their meaning is unknown). */
static Fn8040Vector3 lbl_80583B68(86.5f, 195.0f, 86.5f);

ElecFireWall::ElecFireWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CB88(0x56, 4, "ElecFireWall");
    m_model = new GadgetModelInfo(1, "FIREWALL.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    m_attack = m_model->fn_80161428(0);

    {
        boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

        if (attack.get() != 0) {
            attack->fn_80054DD0(
                1, 1, AttackCallback(boost::bind(&ElecFireWall::fn_801F6764, this, _1)),
                AttackCallback(boost::bind(&ElecFireWall::fn_801F6760, this, _1)), 1.0f);
        }
    }

    CopyFromOwner(m_owner);

    m_height = m_params.initialY;
    if (m_height > -100.0f * m_params.scaleY) {
        m_height = -100.0f * m_params.scaleY;
    }
    ApplyScale();

    if (m_params.initialY == 0.0f) {
        m_state = 3;
    } else {
        m_state = 0;
    }
    m_speed = 0.0f;
    m_partScaleY = 1.0f;
    UpdateModelMatrix();
}

ElecFireWall::~ElecFireWall() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecFireWall::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (IsCharInfoInRange()) {
            m_state = 1;
            fn_801D54F4(fn_80053164(), 0x723D, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 1:
        Accelerate(&m_height, &m_speed, 200.0f, 2000.0f, dt);
        if (m_height > -100.0f * m_params.scaleY) {
            m_height = -97.0f * m_params.scaleY;
            m_speed *= 0.333f;
            m_state = 2;
        }
        break;

    case 2:
        Integrate(&m_height, &m_speed, -1.0f * (m_height - -100.0f * m_params.scaleY), 25.0f, dt);
        if (fn_8000A980(m_speed) < 0.001667f) {
            m_height = -100.0f * m_params.scaleY;
            m_state = 3;
        }
        break;

    case 4:
        ShowBreakEffect();
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }

    UpdateModelMatrix();
    ApplyScale();

    /* State 4 returned above; this test sees it only if the attack callback (fn_801F6764) sets
     * it during the two calls above, which is not known to happen. The original makes the test. */
    if (m_state != 4) {
        if (m_height < -370.0f * m_params.scaleY) {
            m_partScaleY = 1.0f;
        } else {
            float lowest = -370.0f * m_params.scaleY;

            m_partScaleY = 1.0f - (lowest - m_height) / lowest;
        }
        UpdatePartFrame();
    }
}

void ElecFireWall::UpdatePartFrame() {
    if (m_model != 0) {
        UnkPartFrameSearch search;

        /* The original stores 0 to both words (the constructor), then 1 to the id and 0 to the
         * frame again; without the second store to the frame the function is one word shorter. */
        search.id = 1;
        search.frame = 0;
        m_model->fn_8015FA78(fn_801F7778, &search);
        m_partFrame = search.frame;
        /* The original copies the frame's matrix before it tests whether a frame was found. */
        m_partMatrix = search.frame->modelling;

        if (m_partFrame != 0) {
            RwV3d scale = {1.0f, m_partScaleY, 1.0f};

            fn_8048764C(m_partFrame, &scale, rwCOMBINEREPLACE);
            fn_804876E8(m_partFrame, &m_partMatrix, rwCOMBINEPOSTCONCAT);
        }
    }
}

void ElecFireWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    m_state = 0;
    m_height = m_params.initialY;
    UpdateModelMatrix();

    if (m_model != 0) {
        m_model->fn_80162EF4();
        ApplyScale();
        m_model->fn_8016108C();
        fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.searchRange);
    }
}

void ElecFireWall::ApplyScale() {
    float scaledY = lbl_80583B68.y * m_params.scaleY;

    m_model->fn_801617D0(0, -2, lbl_80583B68.x * m_params.scaleX, scaledY,
                         lbl_80583B68.z * m_params.scaleZ);

    Fn8040Vector3 offset(0.0f, scaledY + m_height, 0.0f);

    m_model->fn_80161740(0, &offset, -2);
}

bool ElecFireWall::IsCharInfoInRange() {
    boost::shared_ptr<CharInfo> nearest;

    nearest = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (nearest.get() != 0) {
        float rangeSquared = m_params.searchRange * m_params.searchRange;

        const Fn8040Vector3 &offset = m_charInfo->m_position - nearest->m_position;
        return fn_8040CF2C(&offset) <= rangeSquared;
    }

    return false;
}

void ElecFireWall::Integrate(float *position, float *velocity, float acceleration, float drag,
                             float duration) {
    if (drag <= 0.0f) {
        float accelerationStep = acceleration * duration;

        *position = *position + *velocity * duration + duration * accelerationStep / 2.0f;
        *velocity = *velocity + accelerationStep;
    } else {
        float growth = std::pow(2.7182817f, drag * duration);
        float decay = 1.0f / growth;
        float inverseDrag = 1.0f / drag;

        *position =
            *position + duration * (acceleration * inverseDrag) +
            (1.0f - decay) * (inverseDrag * (inverseDrag * (drag * *velocity - acceleration)));
        *velocity = acceleration * inverseDrag + decay * (*velocity - acceleration * inverseDrag);
    }
}

void ElecFireWall::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void ElecFireWall::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);

        Fn8040Vector3 offset(0.0f, m_height, 0.0f);

        fn_8040B59C(&matrix, &offset);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B720(&matrix, m_params.scaleX, m_params.scaleY, m_params.scaleZ);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecFireWall::fn_801F6764(AttackCallbackParam &param) {
    param.field_0x34 = m_charInfo->field_0x18;
    param.field_0x4C = fn_8040CF50(&param.field_0x34);
    fn_8040CE74(&param.field_0x34);

    if (param.field_0x4C == 0.0f) {
        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

        param.field_0x34 = up;
        param.field_0x4C = 1.0f;
    }

    m_state = 4;
}

void ElecFireWall::fn_801F6760(AttackCallbackParam &param) {}

void ElecFireWall::ShowBreakEffect() {
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(3);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.3f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 0;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }

    fn_801D560C(fn_80053164(), 0x723E, 0, 0);
}

extern "C" void fn_801F6634(void) {
    fn_8006B474("FIREWALL.BIN", 1);
}

extern "C" void fn_801F6630(void) {}

void ElecFireWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecFireWall(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "Scale X", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Scale Y", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Scale Z", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchRange", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Initial Y", 0, 0, 0, -370.0f, -65536.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80542218 = {
    "ELEC_FIREWALL", fn_801F6634, fn_801F6630, ElecFireWall::Create, 0, 0, 0x7DF, 1, 0, 10,
    s_paramInfo,
};
