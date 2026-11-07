/* The HINTRING gadget (class described in Game/HintRing.h). File name inferred from the class's
 * namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The model's collision callback and the draw layer's callback are boost::function objects built
 * with boost::bind(&SetHintRing::method, this, _1); their managers, invokers and copy functions
 * come from the boost headers. */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's and
 * CharaColliBase's derived classes, which TNumEdit.h and CharaColliBaseCharInfo.h emit through
 * their stand-in inline functions (see TNumEdit.h). The three headers come first, in the order of
 * what they put in the unit's .rodata: the TNumEdit names, the axes, then the CharaColliBase
 * names. Revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/FrameRotationAxes.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/HintRing.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_80109D64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkRegistry_8010A458.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkString_8004F888.h"
#include "Game/fn_8005AE20.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80109AF8.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

const char *HintRing::SetHintRing::s_name = "SetHintRing";

HintRing::SetHintRing::SetHintRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_charInfo = fn_8015CD90(4, 4);

    m_model = fn_8015F800(0, "HINTRING.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&SetHintRing::OnCollision, this, _1));

    m_layer = fn_8004E24C(fn_80046178()->field_0x8);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&SetHintRing::OnDraw, this, _1));

    m_clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 0, "HINTRING01.DFF"));
    fn_8010A458()->fn_801EB660(UnkString_8004F888("HintRing"), m_clump);

    ReadOwner(owner);
    CreateTarget();
    UpdateModelMatrix();
}

/* Matching workaround: fn_8010A458, fn_80109D64, fn_80109B80 and fn_80109AF8 are defined here as
 * ordinary functions, at the addresses of the original's copies. Every call to them from other
 * files comes from a higher address, as if they were out-of-line copies of inline functions that
 * the linker kept from the first file that uses them; the headers that declare them keep the plain
 * declarations their other callers use. Revisit when their original header is recovered. */

/* Builds the registry on the first call and caches its address. */
extern "C" UnkRegistry_8010A458 *fn_8010A458() {
    static UnkRegistry_8010A458 *s_registry;

    if (s_registry == 0) {
        static UnkRegistry_8010A458 registry;

        s_registry = &registry;
    }

    return s_registry;
}

HintRing::SetHintRing::~SetHintRing() {
    fn_8010A458()->fn_801EB550(UnkString_8004F888("HintRing"));

    if (m_target.get() != 0) {
        fn_801E7108(m_target);
    }
}

void HintRing::SetHintRing::UpdateBody(float dt) {
    if (m_flags.Test(3)) {
        m_flags.Set(4);
    } else {
        m_flags.Reset(4);
    }

    m_flags.Reset(3);

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        if (m_comId > 0 && fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_flags.Reset(0);
        } else {
            m_flags.Set(0);
        }

        if (!m_flags.Test(0) && !m_flags.Test(2)) {
            if (m_model.get() != 0) {
                m_model->fn_80162790();
            }

            m_flags.Set(2);
        }

        if (m_flags.Test(0) && m_flags.Test(2)) {
            if (m_model.get() != 0) {
                m_model->fn_8016271C();
            }

            m_flags.Reset(2);
        }
        break;

    case 2:
        if (fn_801FAAC8(fn_80109D64(), m_messageId)) {
            m_flags.Set(0);
            m_flags.Reset(5);
            m_state = 1;
        }
        break;
    }
}

void HintRing::SetHintRing::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 navi = m_params.navi;

    ReadOwner(owner);
    UpdateModelMatrix();

    if (navi != m_params.navi) {
        CreateTarget();
    }

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
    }
}

void HintRing::SetHintRing::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_messageId = 0;
    m_flags.Clear();
    m_flags.Set(0);
}

void HintRing::SetHintRing::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

void HintRing::SetHintRing::CreateTarget() {
    m_target.reset();
    m_flags.Set(1);

    switch (m_params.navi) {
    case 0:
        m_target = fn_801E72DC(0x200000);
        break;

    case 1:
        m_target = fn_801E72DC(0x400000);
        break;

    case 2:
        break;

    case 3:
        m_target = fn_801E72DC(0x600000);
        break;

    case 4:
        m_target = fn_801E72DC(0x200000);
        break;

    case 5:
        m_target = fn_801E72DC(0x400000);
        break;

    case 6:
        m_target = fn_801E72DC(0x600000);
        break;

    /* 7 is "navi None"; the original's jump table covers 0 to 7, and 7 shares the code of the
     * values above 7. */
    case 7:
    default:
        m_flags.Reset(1);
        break;
    }

    if (m_target.get() != 0) {
        m_target->SetField20(fn_8005AE20(m_params.range));
    }
}

/* Builds the manager on the first call and caches its address. */
extern "C" UnkManager_80109D64 *fn_80109D64(void) {
    static UnkManager_80109D64 *s_manager;

    if (s_manager == 0) {
        static UnkManager_80109D64 manager;

        s_manager = &manager;
    }

    return s_manager;
}

void HintRing::SetHintRing::OnCollision(ColliDetectArgs &args) {
    if (m_flags.Test(0)) {
        if (m_flags.Test(1) ? fn_8015B1C4(args) : fn_8015AC84(args)) {
            m_flags.Set(3);

            if (!m_flags.Test(5) && !m_flags.Test(4)) {
                m_messageId = 0;

                if (m_params.branchId >= 0) {
                    m_messageId =
                        fn_801FAF00(fn_80109D64(), m_params.messageId, m_params.branchId, 1, 0);
                } else {
                    switch (fn_8006D6A8()->field_0x20) {
                    case 1:
                        m_messageId = fn_801FAF00(fn_80109D64(), m_params.messageId, 1, 1, 0);
                        break;

                    case 2:
                        m_messageId = fn_801FAF00(fn_80109D64(), m_params.messageId, 2, 1, 0);
                        break;

                    case 0:
                        m_messageId = fn_801FAF00(fn_80109D64(), m_params.messageId, 0, 1, 0);
                        break;
                    }
                }

                if (m_messageId != -1) {
                    fn_801D560C(fn_80053164(), 0xE01B, 0, 0);
                    m_flags.Set(5);
                    m_state = 2;
                }
            }
        }
    }
}

/* Returns the angles that fn_8000A610 computes from the entry with the given index of the manager
 * (behaviour inferred). The BDTrue gadget and OnDraw above use them as rotation angles. */
extern "C" UnkRotation3 fn_80109B80(UnkManager_80009548 *manager, int index) {
    return fn_8000A610(&fn_8000D0F0(manager, index)->field_0x8);
}

extern "C" void fn_80109AF8(RwFrame *frame, int combine, float angle) {
    fn_8047ECD4(&frame->modelling, &s_zAxis, combine, 1.0f - std::cos(angle), std::sin(angle));
    fn_8048722C(frame);
}

void HintRing::SetHintRing::OnDraw(RednerDrawArgs &args) {
    if (!m_flags.Test(2)) {
        RwSphere sphere;

        sphere.radius = 10.0f;
        sphere.center.x = m_charInfo->m_position.x;
        sphere.center.y = m_charInfo->m_position.y;
        sphere.center.z = m_charInfo->m_position.z;

        if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
            RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);
            UnkRotation3 angles = fn_80109B80(fn_80009548(), 0);

            fn_80109AF8(frame, rwCOMBINEREPLACE, angles.z);
            fn_800E9C64(frame, rwCOMBINEPOSTCONCAT, angles.x);
            fn_800E9CEC(frame, rwCOMBINEPOSTCONCAT, angles.y);
            fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
            fn_80456BC4(m_clump);
        }
    }
}

extern "C" void fn_801099C8(void) {
    fn_8006B474("HINTRING.BIN", 1);
}

extern "C" void fn_801099C4(void) {}

void HintRing::SetHintRing::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetHintRing(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The labels of the "Brunch" parameter: for a value of 0 or more, and for a negative value. */
static const char *s_branchLabels[2] = {"Brunch Id", "Brunch Id -> Auto"};

/* The labels of the "navi" parameter's values 0 to 7. */
static const char *s_naviLabels[8] = {"navi H",   "navi D",   "navi E",     "navi H+D",
                                      "navi H+E", "navi D+E", "navi H+D+E", "navi None"};

/* The label callback of the descriptor's "Brunch" and "navi" parameters (behaviour inferred). The
 * "navi" value is not range-checked. */
static const char *fn_8010991C(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 1) {
        if (cursor->m_value >= 0) {
            return s_branchLabels[0];
        }

        return s_branchLabels[1];
    }

    if (cursor->m_index == 4) {
        return s_naviLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Message Id", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "Brunch", 0xFFFFFFFF, 0xFFFFFFFF, 0x10000, 0.0f, 0.0f, 0.0f, fn_8010991C, 0},
    {4, "range", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "sec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "navi", 7, 0, 7, 0.0f, 0.0f, 0.0f, fn_8010991C, 0},
    {0},
};

GadgetTypeInfo lbl_80526490 = {
    "HINTRING", fn_801099C8, fn_801099C4, HintRing::SetHintRing::Create, 0, 0, 0x11, 1,
    0,          10,          s_paramInfo,
};
