/* The RollCircle gadget and its RollCircleObj objects (behaviour in the class comments). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The objects live in a std::list of shared pointers and the indices of the objects queued for
 * breaking in a std::vector; the three callbacks of an object's attack object are boost::function
 * objects built with boost::bind(&RollCircleObj::..., this, _1). */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit; nothing in this file uses them.
 * TNumEdit.h comes first, in the order their names appear in the unit's .rodata (alphabetical order
 * would reverse them). Revisit when the original include order is known. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/RollCircle.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/AngleConversion.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The break effect file of each model, in the order of the MODEL parameter (Big, Sml, Head). */
static const char *s_breakFiles[3] = {
    "CN0201ROLLCIRCLEBIGBRK.DFF",
    "CN0201ROLLCIRCLESMLBRK.DFF",
    "CN0000ROLLOBJECTHEADBRK.DFF",
};

/* The starting health of an object of each model (name inferred from the use). */
static const float s_startHealths[3] = {4.0f, 2.0f, 2.0f};

/* The state that the owner remembers between the creations of the gadget (class name from RTTI,
 * derived from SetAdapterState): the health of each object (name inferred). */
class RollCircleSetState : public SetAdapterState {
  public:
    RollCircleSetState(u32 count);
    virtual ~RollCircleSetState();

    /* 0x4 */ float *m_healths;
};
CHECK_SIZEOF(RollCircleSetState, 0x8);

/* Matching workaround: the constructor and the destructor are defined out of line. The original
 * emits them after RollCircleObj's constructor, the function defined next; an inline constructor
 * would be emitted right after its first caller, CreateState. Revisit if the original's definitions
 * turn out to be inline and another cause of that order is found. */
RollCircleSetState::RollCircleSetState(u32 count) : m_healths(new float[count]) {}

RollCircleSetState::~RollCircleSetState() {
    delete[] m_healths;
    m_healths = 0;
}

RollCircleObj::RollCircleObj(UnkGadgetOwner_800C9CD0 *owner, RollCircle *holder, u32 index)
    : m_index(index), m_holder(holder), m_owner(owner) {
    m_angle = index * (DegreesToRadians(360.0f) / m_holder->m_params.objNum);
    m_charInfo = fn_8015CD90(0x2C, 4);
    m_charInfo->field_0x30 = 1;
    m_model = fn_8015F800(1, "ROLLCIRCLE.BIN");
    m_model->fn_80162A68(m_holder->m_params.model, 0);
    m_model->fn_80161FC8(m_holder->m_params.model, boost::weak_ptr<CharInfo>(m_charInfo));

    {
        boost::shared_ptr<CharaColliAttack> attack =
            m_model->fn_80161428(m_holder->m_params.model).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(4, AttackCallback(boost::bind(&RollCircleObj::OnAttack, this, _1)));
            attack->fn_80054DD0(
                4, 1, AttackCallback(boost::bind(&RollCircleObj::fn_800D61C4, this, _1)),
                AttackCallback(boost::bind(&RollCircleObj::fn_800D6170, this, _1)), 1.0f);
        }
    }

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x2800);
    m_target->SetFlags(1);
    UpdatePosition();
    UpdateModelMatrix();
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
RollCircleObj::~RollCircleObj() {}
#pragma auto_inline reset

void RollCircleObj::Update(float dt) {
    UpdatePosition();
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void RollCircleObj::Reset() {
    UpdatePosition();
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void RollCircleObj::UpdatePosition() {
    Fn8040Vector3 offset = fn_8040D0AC(m_holder->m_offset, m_angle);
    offset = fn_8040D044(offset, m_holder->m_rotation.z);
    offset = fn_8040D118(offset, m_holder->m_rotation.x);
    Fn8040Vector3 previous = m_charInfo->m_position;
    m_charInfo->m_position = m_holder->m_position + offset;
    m_charInfo->m_rotation = m_holder->m_objRotation;
    m_charInfo->field_0x18 = m_charInfo->m_position - previous;
}

void RollCircleObj::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

void RollCircleObj::Break() {
    UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

    effect.field_0x44 = static_cast<RpClump *>(
        fn_80012700(fn_800122D4(), 1, s_breakFiles[m_holder->m_params.model]));
    effect.field_0x34 = 1.0f;
    effect.field_0x4A = 0;
    effect.field_0x54 = 1;
    fn_8015D93C(&effect);

    fn_801D54F4(fn_80053164(), 0x7330, &m_charInfo->m_position, 0, 1);
}

void RollCircleObj::fn_800D61C4(AttackCallbackParam &param) {
    param.field_0x34 = m_charInfo->field_0x18;
    fn_8040CE38(&param.field_0x34, 1.0f / lbl_8057D6A0.field_0x18);
    param.field_0x4C = fn_8040CF50(&param.field_0x34);
    fn_8040CE74(&param.field_0x34);
    param.field_0x50 = 1.0f;
}

void RollCircleObj::fn_800D6170(AttackCallbackParam &param) {
    if (param.field_0x54 & 1) {
        if (!m_holder->IsQueued(m_index)) {
            m_holder->Queue(m_index);
        }
    }
}

void RollCircleObj::OnAttack(AttackCallbackParam &param) {
    float *healths = dynamic_cast<RollCircleSetState *>(m_owner->m_adapterState)->m_healths;

    healths[m_index] -= param.field_0x50;

    if (healths[m_index] < 0.0001f) {
        if (!m_holder->IsQueued(m_index)) {
            {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x3F);
                }
            }

            m_holder->Queue(m_index);
        }

        param.field_0x54 = 3;
    } else {
        param.field_0x54 = 1;
    }
}

const char *RollCircle::s_name = "RollCircle";

extern "C" void fn_800D6050(void) {
    fn_8006B474("ROLLCIRCLE.BIN", 1);
}

extern "C" void fn_800D604C(void) {}

void RollCircle::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new RollCircle(fn_801EB304(fn_8001C4F8(), 11), owner);
}

RollCircle::RollCircle(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_rotation(0.0f, 0.0f, 0.0f), m_objRotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    m_objs.clear();
    m_queued.clear();
    ReadOwner(owner);

    if (owner->m_adapterState == 0) {
        CreateState();
    }

    CreateObjs();
}

RollCircle::~RollCircle() {}

void RollCircle::UpdateBody(float dt) {
    BreakQueued();
    m_offset = fn_8040D0AC(m_offset, m_params.circleSpeed * dt);
    m_objRotation.x += m_params.objAngSpdX * dt;
    m_objRotation.y += m_params.objAngSpdY * dt;
    m_objRotation.z += m_params.objAngSpdZ * dt;

    for (ObjList::iterator it = m_objs.begin(); it != m_objs.end(); it++) {
        (*it)->Update(dt);
    }
}

void RollCircle::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ObjList::iterator it;
    u32 model = m_params.model;
    u32 objNum = m_params.objNum;

    ReadOwner(owner);

    if (objNum != m_params.objNum || model != m_params.model) {
        CreateState();
        CreateObjs();
    }

    for (it = m_objs.begin(); it != m_objs.end(); it++) {
        (*it)->Reset();
    }
}

void RollCircle::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.radius = 10.0f * m_params.radius;
    m_params.circleSpeed = DegreesToRadians(m_params.circleSpeed);
    m_params.objAngSpdX = DegreesToRadians(m_params.objAngSpdX);
    m_params.objAngSpdY = DegreesToRadians(m_params.objAngSpdY);
    m_params.objAngSpdZ = DegreesToRadians(m_params.objAngSpdZ);
    m_queued.reserve(m_params.objNum);

    fn_8040CE60(&m_offset);
    m_offset.z -= m_params.radius;
    m_offset = fn_8040D0AC(m_offset, m_rotation.y);
    fn_8040E970(&m_objRotation);
}

void RollCircle::CreateState() {
    if (m_params.objNum != 0) {
        if (m_owner->m_adapterState != 0) {
            fn_800C9894(m_owner);
        }

        RollCircleSetState *state = new RollCircleSetState(m_params.objNum);
        /* The original reads the array before the owner takes the state. */
        float *healths = state->m_healths;

        fn_800C9930(m_owner, state);

        for (u32 i = 0; i < m_params.objNum; i++) {
            healths[i] = s_startHealths[m_params.model];
        }
    }
}

void RollCircle::CreateObjs() {
    float *healths = dynamic_cast<RollCircleSetState *>(m_owner->m_adapterState)->m_healths;

    m_objs.clear();

    for (u32 i = 0; i < m_params.objNum; i++) {
        if (healths[i] < 0.0001f) {
            continue;
        }

        m_objs.push_back(boost::shared_ptr<RollCircleObj>(new RollCircleObj(m_owner, this, i)));
    }
}

bool RollCircle::IsQueued(u32 index) {
    if (!m_queued.empty()) {
        for (std::vector<u32>::iterator it = m_queued.begin(); it != m_queued.end(); ++it) {
            if (*it == index) {
                return true;
            }
        }
    }

    return false;
}

void RollCircle::Queue(u32 index) {
    m_queued.push_back(index);
}

void RollCircle::BreakQueued() {
    if (!m_queued.empty()) {
        for (std::vector<u32>::iterator it = m_queued.begin(); it != m_queued.end(); ++it) {
            BreakObj(*it);
        }

        m_queued.clear();

        if (m_objs.empty()) {
            fn_800C9B34(m_owner);
            m_flags |= 1;
        }
    }
}

void RollCircle::BreakObj(u32 index) {
    ObjList::iterator it;

    for (it = m_objs.begin(); it != m_objs.end(); it++) {
        if ((*it)->m_index == index) {
            (*it)->Break();
            break;
        }
    }

    if (it != m_objs.end()) {
        m_objs.erase(it);
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "MODEL 0:Big 1:Sml 2:Head", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "OBJ_NUM", 4, 1, 0x20, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RADIUS m", 0, 0, 0, 20.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "CIRCLE_SPD deg/sec", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "OBJ_ANGSPD_X deg/sec", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "OBJ_ANGSPD_Y deg/sec", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "OBJ_ANGSPD_Z deg/sec", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805234EC = {
    "ROLL_CIRCLE", fn_800D6050, fn_800D604C, RollCircle::Create, 0, 0, 0x836, 1, 0, 20, s_paramInfo,
};
