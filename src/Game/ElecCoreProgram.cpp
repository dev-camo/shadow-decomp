/* The ELEC_CORE gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The command handler and the model attack object's callback are boost::function objects built
 * with boost::bind(&ElecCoreProgram::OnCommand, this, _1) and
 * boost::bind(&ElecCoreProgram::OnAttack, this, _1); their manager, invoker and copy functions
 * come from the boost headers. */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's
 * and CharaColliBase's derived classes, which these two headers emit through their stand-in
 * inline functions (see TNumEdit.h). TNumEdit.h comes before CharaColliBaseCharInfo.h, in the
 * order their names appear in the unit's .rodata. Revisit when the original inline functions or
 * their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ElecCoreProgram.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkPartFrameSearch.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80109AF8.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The part callback (placeholder name; ElecFireWall's fn_801F7778 has the same body): stores the
 * frame of the clump of the part whose word at 0x8 equals the searched id and stops the walk once
 * a frame is found (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_801F6240(UnkModelPart_8015FA78 *part, void *userData) {
    UnkPartFrameSearch *search = static_cast<UnkPartFrameSearch *>(userData);

    if (search != 0 && part->clump != 0 && part->field_0x8 == search->id) {
        search->frame = static_cast<RwFrame *>(part->clump->object.parent);
        if (search->frame != 0) {
            return 0;
        }
    }
    return part;
}

const char *ElecCoreProgram::s_name = "ElecCoreProgram";

/* The value that goes linearly from `from` at t0 to `to` at t1 (name inferred). */
static inline float Interpolate(float t, float from, float to, float t0, float t1) {
    float ratio = (t - t0) / (t1 - t0);

    return from + ratio * (to - from);
}

ElecCoreProgram::ElecCoreProgram(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(5, 4, "ElecCoreProgram");

    m_object2 = fn_802B4254(0, 0);
    fn_802B4498(m_object2.get(), 1200.0f);
    fn_802B4510(m_object2.get(), 1);

    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ElecCoreProgram::OnCommand, this, _1));

    m_model = new GadgetModelInfo(1, "CORE.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_target = fn_801E72DC(0xC00);

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->fn_80096B64(0, AttackCallback(boost::bind(&ElecCoreProgram::OnAttack, this, _1)));
    }

    m_model->fn_8016239C("mat1");
    m_model->fn_8016239C("mat2");
    m_model->fn_8016239C("mat3");
    m_model->fn_8016239C("mat4_1");
    m_model->fn_8016239C("mat4_2");
    m_model->fn_8016239C("mat4_3");
    m_model->fn_8016239C("mat4_4");
    m_model->fn_8016239C("mat4_5");
    m_model->fn_8016239C("mat5");

    CopyFromOwner(m_owner);
    m_state = 0;
    m_hitsLeft = 16;
    m_time = 0.0f;
    m_hitTime = 0.0f;
    UpdateModelMatrix();
}

ElecCoreProgram::~ElecCoreProgram() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
    fn_801E7108(m_target);
}

void ElecCoreProgram::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    UpdateModelMatrix();
    RotateParts(dt);

    switch (m_state) {
    case 0:
        UpdateIdle(dt);
        break;

    case 1:
        if (m_time >= 1.0f) {
            boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

            if (actor.get() != 0 && !actor->Field0xCEquals(1)) {
                actor->SetField0x14(1);
                fn_8016B2A4(actor.get());
            }

            fn_8034DE08(9, &m_charInfo->m_position, -2);
            fn_8015A794(this);
            m_flags |= 1;
        } else {
            UpdateBreakColors(dt);
            UpdateBreakScale();
        }
        break;
    }
}

void ElecCoreProgram::UpdateBreakColors(float dt) {
    static UnkDebugColor_802E0E84 s_mat1Color0(0xFE, 0xB2, 0xB2, 0xB2);
    static UnkDebugColor_802E0E84 s_mat1Color1(0xFE, 0x19, 0x19, 0x19);
    static UnkDebugColor_802E0E84 s_mat1Color2(0xFE, 0xFF, 0xFF, 0xFF);
    static UnkDebugColor_802E0E84 s_mat1Color3(0xFE, 0x00, 0x00, 0x00);
    static UnkDebugColor_802E0E84 s_fadeColor0(0xFE, 0xFF, 0xFF, 0xFF);
    static UnkDebugColor_802E0E84 s_fadeColor1(0xFE, 0x00, 0x00, 0x00);
    static UnkDebugColor_802E0E84 s_mat5Color0(0xFE, 0x00, 0x00, 0x00);
    static UnkDebugColor_802E0E84 s_mat5Color1(0xFE, 0x7F, 0x7F, 0x7F);

    UnkModelAtomicState *mat1 = m_model->fn_801621F0("mat1");
    UnkModelAtomicState *mat2 = m_model->fn_801621F0("mat2");
    UnkModelAtomicState *mat3 = m_model->fn_801621F0("mat3");
    UnkModelAtomicState *mat4_1 = m_model->fn_801621F0("mat4_1");
    UnkModelAtomicState *mat4_2 = m_model->fn_801621F0("mat4_2");
    UnkModelAtomicState *mat4_3 = m_model->fn_801621F0("mat4_3");
    UnkModelAtomicState *mat4_4 = m_model->fn_801621F0("mat4_4");
    UnkModelAtomicState *mat4_5 = m_model->fn_801621F0("mat4_5");
    UnkModelAtomicState *mat5 = m_model->fn_801621F0("mat5");

    m_time += dt;
    float time = m_time;

    if (mat1 != 0) {
        if (0.0f <= time && time < 0.2f) {
            fn_80154C6C(mat1, time, s_mat1Color0, s_mat1Color1, 0.0f, 0.2f);
        } else if (0.2f <= time && time < 0.8f) {
            fn_80154C6C(mat1, time, s_mat1Color1, s_mat1Color2, 0.2f, 0.8f);
        } else {
            fn_80154C6C(mat1, time, s_mat1Color2, s_mat1Color3, 0.8f, 1.0f);
        }
    }

    if (mat2 != 0 && mat3 != 0 && mat4_1 != 0 && mat4_2 != 0 && mat4_3 != 0 && mat4_4 != 0 &&
        mat4_5 != 0 && 0.8f <= time) {
        fn_80154C6C(mat2, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat3, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat4_1, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat4_2, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat4_3, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat4_4, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
        fn_80154C6C(mat4_5, time, s_fadeColor0, s_fadeColor1, 0.8f, 1.0f);
    }

    if (mat5 != 0) {
        if (0.0f <= time && time < 0.1f) {
            fn_80154C6C(mat5, time, s_mat5Color0, s_mat5Color1, 0.0f, 0.1f);
        } else if (0.8f <= time) {
            fn_80154C6C(mat5, time, s_mat5Color1, s_mat5Color0, 0.8f, 1.0f);
        }
    }
}

void ElecCoreProgram::UpdateIdle(float dt) {
    m_time += dt;

    static UnkDebugColor_802E0E84 s_pulseColor0(0xFE, 0x7F, 0x66, 0x00);
    static UnkDebugColor_802E0E84 s_pulseColor1(0xFE, 0xB2, 0xB2, 0xB2);

    float period = 1.0f / 3.0f;
    float phase = m_time - static_cast<int>(m_time / period) * period;

    UnkModelAtomicState *mat1 = m_model->fn_801621F0("mat1");
    if (mat1 != 0) {
        if (0.0f <= phase && phase < 0.25f) {
            fn_80154C6C(mat1, phase, s_pulseColor0, s_pulseColor1, 0.0f, 0.25f);
        } else {
            fn_80154C6C(mat1, phase, s_pulseColor1, s_pulseColor0, 0.25f, period);
        }
    }

    if (m_hitTime > 0.0f) {
        m_hitTime -= dt;
        if (m_hitTime < 0.0f) {
            m_hitTime = 0.0f;
        }
    } else {
        UnkModelAtomicState *mat2 = m_model->fn_801621F0("mat2");
        if (mat2 != 0) {
            if (m_hitsLeft <= 1) {
                mat2->color.field_0x0 = 0x40;
                mat2->color.field_0x1 = 0x00;
                mat2->color.field_0x2 = 0x00;
                mat2->color.field_0x3 = 0x4C;
            } else if (m_hitsLeft <= 8) {
                mat2->color.field_0x0 = 0x40;
                mat2->color.field_0x1 = 0x7F;
                mat2->color.field_0x2 = 0x00;
                mat2->color.field_0x3 = 0x00;
            } else {
                mat2->color.field_0x0 = 0x40;
                mat2->color.field_0x1 = 0x00;
                mat2->color.field_0x2 = 0x4D;
                mat2->color.field_0x3 = 0x00;
            }
        }
    }
}

void ElecCoreProgram::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecCoreProgram::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecCoreProgram::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_charInfo->m_position);
    }
    if (m_object2.get() != 0) {
        Fn8040Vector3 offset(0.0f, 34.0f, 0.0f);

        const Fn8040Vector3 &position = offset + m_charInfo->m_position;
        fn_802B44C8(m_object2.get(), &position);
    }
}

void ElecCoreProgram::OnCommand(CharCommand &command) {}

/* How RotateParts turns the spinning parts (ids 3 to 7, in order; names inferred). */
struct ElecCorePartMotion {
    /* 0x0 */ float angleX;    /* the fixed angle about the x axis */
    /* 0x4 */ float angleZ;    /* the fixed angle about the z axis */
    /* 0x8 */ float spinSpeed; /* the angle about the y axis advances by this times dt */
};
CHECK_SIZEOF(ElecCorePartMotion, 0xC);

/* Defined before RotateParts, its only user; the original's .data has the table after the
 * constructor's two member function pointer constants. */
static ElecCorePartMotion s_partMotions[5] = {
    {-3.0f, -5.0f, 2.0f * lbl_805FAD38},          /* part 3 */
    {0.0f, 30.0f, -2.0f * lbl_805FAD38 / 1.5f},   /* part 4 */
    {50.0f, 5.0f, -2.0f * lbl_805FAD38 / 1.2f},   /* part 5 */
    {30.0f, 70.0f, 2.0f * lbl_805FAD38 / 0.8f},   /* part 6 */
    {-30.0f, -50.0f, 2.0f * lbl_805FAD38 / 0.9f}, /* part 7 */
};

void ElecCoreProgram::RotateParts(float dt) {
    if (m_model != 0) {
        UnkPartFrameSearch search;

        for (int i = 0; i < 5; i++) {
            search.id = i + 3;
            search.frame = 0;
            m_model->fn_8015FA78(fn_801F6240, &search);
            m_partFrames[search.id] = search.frame;
            m_partMatrices[search.id] = search.frame->modelling;

            m_partAngles[search.id] += dt * s_partMotions[i].spinSpeed;
            fn_80109AF8(m_partFrames[search.id], rwCOMBINEREPLACE, s_partMotions[i].angleZ);
            fn_800E9C64(m_partFrames[search.id], rwCOMBINEPRECONCAT, s_partMotions[i].angleX);
            fn_800E9CEC(m_partFrames[search.id], rwCOMBINEPRECONCAT, m_partAngles[search.id]);
            fn_804876E8(m_partFrames[search.id], &m_partMatrices[search.id], rwCOMBINEPOSTCONCAT);
        }
    }
}

void ElecCoreProgram::UpdateBreakScale() {
    if (m_model != 0) {
        UnkPartFrameSearch search;

        search.id = 0;
        search.frame = 0;
        m_model->fn_8015FA78(fn_801F6240, &search);
        m_partFrames[0] = search.frame;
        m_partMatrices[0] = search.frame->modelling;

        search.id = 8;
        search.frame = 0;
        m_model->fn_8015FA78(fn_801F6240, &search);
        m_partFrames[8] = search.frame;
        m_partMatrices[8] = search.frame->modelling;

        float time = m_time;

        if (m_partFrames[0] != 0) {
            float factor;

            if (time < 0.2f) {
                factor = Interpolate(time, 1.0f, 0.1f, 0.0f, 0.2f);
            } else if (time < 0.8f) {
                factor = Interpolate(time, 0.1f, 20.0f, 0.2f, 0.8f);
            } else {
                factor = Interpolate(time, 20.0f, 30.0f, 0.8f, 1.0f);
            }

            RwV3d scale = {factor, factor, factor};

            fn_8048764C(m_partFrames[0], &scale, rwCOMBINEREPLACE);
            fn_804876E8(m_partFrames[0], &m_partMatrices[0], rwCOMBINEPOSTCONCAT);
        }

        if (m_partFrames[8] != 0) {
            float factor;

            if (time < 0.1f) {
                factor = Interpolate(time, 1.0f, 20.0f, 0.0f, 0.1f);
            } else if (time < 0.8f) {
                factor = Interpolate(time, 20.0f, 160.0f, 0.1f, 0.8f);
            } else {
                factor = Interpolate(time, 160.0f, 200.0f, 0.8f, 1.0f);
            }

            RwV3d scale = {factor, factor, factor};

            fn_8048764C(m_partFrames[8], &scale, rwCOMBINEREPLACE);
            fn_804876E8(m_partFrames[8], &m_partMatrices[8], rwCOMBINEPOSTCONCAT);
        }
    }
}

void ElecCoreProgram::OnAttack(AttackCallbackParam &param) {
    if (m_hitTime == 0.0f && m_state == 0) {
        UnkModelAtomicState *mat2 = m_model->fn_801621F0("mat2");
        if (mat2 != 0) {
            mat2->color.field_0x0 = 0x60;
            mat2->color.field_0x1 = 0xC0;
            mat2->color.field_0x2 = 0x40;
            mat2->color.field_0x3 = 0x40;
        }

        m_hitTime = 0.05f;
        m_hitsLeft--;
        if (m_hitsLeft <= 0) {
            param.field_0x54 = 3;
            m_state = 1;
            m_time = 0.0f;
            fn_801D54F4(fn_80053164(), 0x7241, &m_charInfo->m_position, 0, 1);
        } else {
            param.field_0x54 = 1;
            fn_801D54F4(fn_80053164(), 0x7240, &m_charInfo->m_position, 0, 1);
        }
    } else {
        param.field_0x54 = 8;
    }
}

extern "C" void fn_801F44F0(void) {
    fn_8006B474("CORE.BIN", 1);
}

extern "C" void fn_801F44EC(void) {}

void ElecCoreProgram::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCoreProgram(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80541F94 = {
    "ELEC_CORE", fn_801F44F0, fn_801F44EC, ElecCoreProgram::Create, 0, 0, 0x7DB, 1, 0, 10, 0,
};
