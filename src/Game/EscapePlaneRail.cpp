/* EscapePlaneRail::EscapePlaneRail (class name from RTTI; the gadget's descriptor is
 * "PLANE RAIL"): a gadget, in the namespace EscapePlaneRail, that shows the ESCAPEPLANERAIL.BIN
 * model. It waits until the stage switch that the owner names is set, then moves down at the
 * parameter "speed" until it has dropped by the parameter "dist" (behaviour inferred). File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

static const char *s_modelFile = "ESCAPEPLANERAIL.BIN";

/* The gadget parameters in the order of the descriptor's table below. */
struct EscapePlaneRailParams {
    EscapePlaneRailParams() : dist(0.0f), speed(0.0f) {}

    /* 0x0 */ float dist;  /* how far the rail drops */
    /* 0x4 */ float speed; /* the drop speed, per second */
};
CHECK_SIZEOF(EscapePlaneRailParams, 0x8);

namespace EscapePlaneRail {

class EscapePlaneRail : public Gadget {
  public:
    EscapePlaneRail(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~EscapePlaneRail();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters and switch id into the gadget (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's matrix (name inferred). */
    void UpdateModelMatrix();
    /* Lowers the character info by speed * dt, stopping at dist below the start; returns true
     * once it is there (name inferred). */
    bool StepDown(float dt);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ EscapePlaneRailParams m_params;
    /* 0x48 */ s32 m_state;    /* 0 waiting for the switch, 1 dropping, 2 down (inferred) */
    /* 0x4C */ s32 m_switchId; /* the id of the stage switch that starts the drop (inferred) */
    /* 0x50 */ Fn8040Vector3 m_startPosition;
    /* 0x5C */ u32 field_0x5C; /* in the allocation size (0x60); no code here touches it */
};
CHECK_SIZEOF(EscapePlaneRail, 0x60);

const char *EscapePlaneRail::s_name = "EscapePlaneRail";

EscapePlaneRail::EscapePlaneRail(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x81, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    UpdateModelMatrix();
}

EscapePlaneRail::~EscapePlaneRail() {}

void EscapePlaneRail::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (fn_8016904C(fn_800CB69C(), m_switchId) == 1) {
            m_state = 1;
        }
        break;
    case 1:
        if (StepDown(dt)) {
            m_state = 2;
        }
        UpdateModelMatrix();
        break;
    case 2:
        break;
    }
}

void EscapePlaneRail::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();

    /* Draws the path of the drop. */
    Fn8040Vector3 start;
    Fn8040Vector3 end;

    fn_800091EC(&start, &m_startPosition);
    fn_800091EC(&end, &start);
    end.y -= m_params.dist;

    UnkSegment_80020A20 path(start, end);

    fn_802E0B58(path, lbl_805F06D8);
}

void EscapePlaneRail::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_switchId);
    fn_800091D0(&m_startPosition, &m_charInfo->m_position);
    m_state = 0;
}

void EscapePlaneRail::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

bool EscapePlaneRail::StepDown(float dt) {
    float bottom = m_startPosition.y - m_params.dist;

    m_charInfo->m_position.y -= m_params.speed * dt;

    if (m_charInfo->m_position.y < bottom) {
        m_charInfo->m_position.y = bottom;
        return true;
    }
    return false;
}

} // namespace EscapePlaneRail

extern "C" void fn_802ABAF0(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_802ABAEC(void) {}

void EscapePlaneRail::EscapePlaneRail::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new EscapePlaneRail(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "dist", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "speed", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80554718 = {
    "PLANE RAIL",
    fn_802ABAF0,
    fn_802ABAEC,
    EscapePlaneRail::EscapePlaneRail::Create,
    0,
    0,
    0x13F3,
    1,
    0,
    10,
    s_paramInfo,
};
