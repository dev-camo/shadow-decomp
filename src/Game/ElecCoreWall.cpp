/* ElecCoreWall (class name from RTTI; the gadget's descriptor is "ELEC_COREWALL"): a wall of the
 * electric circuit area made of the clumps of DE0403COREWALL11.DFF, 12.DFF, 21.DFF and 22.DFF,
 * drawn through the callback it hands to a draw layer in copies turned about the y axis in steps
 * of a 32nd of a turn. A box probe around its character info sets the state to STATE_FORWARD while
 * a character info touches it, which runs the delta morph animation of the 11 and 21 clumps
 * forwards; once nothing touches it the state is STATE_BACKWARD and the animation runs backwards.
 * At the end of the animation the 12 and 22 clumps are drawn instead (behaviour inferred). File name inferred from the class.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the draw layer and the handle target are boost::function objects built
 * with boost::bind(&ElecCoreWall::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* FrameRotationAxes.h comes first and TNumEdit.h before CharaColliBaseCharInfo.h: the unit's
 * .rodata has the axes, then the TNumEdit names, then the CharaColliBase names (alphabetical
 * order would give the opposite). */
#include "Game/FrameRotationAxes.h"
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ElecCoreWall.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80037A80.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpdmorph.h"
#include "rw/rpworld.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* What the hit callback receives (placeholder name). */
struct UnkHitContext_80264838 {
    /* 0x0 */ ElecCoreWall *m_owner;
};
CHECK_SIZEOF(UnkHitContext_80264838, 0x4);

const char *ElecCoreWall::s_name = "ElecCoreWall";

/* The angle between neighbouring copies of a clump: a 32nd of a turn. */
static float s_angleStep = 2.0f * lbl_805FAD38 / 32.0f;

/* The callback that fn_8015AA84 calls for each character info in the collision results, with the
 * context of OnCollision: sets the gadget's state to STATE_FORWARD. */
static void fn_80265CBC(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80264838 *context = static_cast<UnkHitContext_80264838 *>(arg);

    if (hit.get() != 0 && context != 0) {
        context->m_owner->m_state = ElecCoreWall::STATE_FORWARD;
    }
}

ElecCoreWall::ElecCoreWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecCoreWall");

    /* fn_80037A80 hands each .DMA resource to the atomics of the clump of the .DFF file with the
     * same number. */
    void *dma11 = fn_80012700(fn_800122D4(), 1, "DE0403COREWALL11.DMA");

    m_clump11 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403COREWALL11.DFF"));
    m_clump12 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403COREWALL12.DFF"));

    void *dma21 = fn_80012700(fn_800122D4(), 1, "DE0403COREWALL21.DMA");

    m_clump21 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403COREWALL21.DFF"));
    m_clump22 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403COREWALL22.DFF"));
    fn_80037A80(m_clump11, dma11);
    fn_80037A80(m_clump21, dma21);

    m_layer = fn_8004DFB0(fn_80046178()->field_0x28, "ElecCoreWall");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ElecCoreWall::OnDraw, this, _1));

    boost::shared_ptr<CharaColliBase> probe = fn_800C7C34(
        fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo),
        Fn8040Vector3(900.0f, 1000.0f, 750.0f), 0x40000, 10, 0, &lbl_805E07A0, &lbl_805E07B8, 0);
    m_handle = fn_8015B8C4(probe);

    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&ElecCoreWall::OnCollision, this, _1)));
    m_handle->field_0x18.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&ElecCoreWall::OnCollisionEnd, this, _1)));

    CopyFromOwner(m_owner);
    m_timer = 0.0f;
    field_0x64 = 0;
    field_0x60 = 0;
    m_state = STATE_IDLE;
}

ElecCoreWall::~ElecCoreWall() {}

void ElecCoreWall::UpdateBody(float dt) {
    /* STATE_IDLE and STATE_DONE do not advance the timer. */
    switch (m_state) {
    case STATE_IDLE:
        break;
    case STATE_FORWARD:
        m_timer += dt;
        break;
    case STATE_BACKWARD:
        m_timer -= dt;
        break;
    case STATE_DONE:
        break;
    }

    fn_8015BE9C(m_handle.get(), dt);
}

void ElecCoreWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    if (m_handle.get() != 0) {
        m_handle->m_probe->SetPosition(&m_charInfo->m_position);
        m_handle->m_probe->SetRotation(m_charInfo->m_rotation);
        fn_8015BE64(m_handle.get());
    }
}

void ElecCoreWall::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

/* The atomic callback of OnDraw (placeholder name): sets every delta morph target weight of the
 * atomic to the time that data points to, and the radius of its bounding sphere to 1000. */
static RpAtomic *fn_80264DF8(RpAtomic *atomic, void *data) {
    if (atomic->geometry != NULL) {
        RwInt32 numTargets = fn_80443A54(atomic->geometry);

        if (numTargets > 0) {
            RwReal *weights = fn_804442E4(atomic);

            for (RwInt32 i = 0; i < numTargets; i++) {
                *weights++ = *static_cast<RwReal *>(data);
            }
        }
    }

    RwSphere *sphere = RpAtomicGetBoundingSphere(atomic);

    if (sphere != NULL) {
        sphere->radius = 1000.0f;
    }
    return atomic;
}

void ElecCoreWall::OnDraw(RednerDrawArgs &args) {
    RwV3d position = {m_charInfo->m_position.x, m_charInfo->m_position.y, m_charInfo->m_position.z};
    RwSphere sphere;

    sphere.radius = 1000.0f;
    sphere.center = position;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        /* i is compared with float constants, as in the original (each test converts it). Each
         * copy of a clump is written out: an inline helper that takes the clump by value keeps it
         * in a saved register, where the original reloads the member for the render call. */
        for (int i = 0; i < 16.0f; i++) {
            RwFrame *frame;

            if (m_timer < 0.0f) {
                /* Nothing in the loop changes the timer, so this runs with i == 0 and the reset
                 * below never happens (kept as in the original). */
                if (i == 15.0f && m_state == STATE_BACKWARD) {
                    m_timer = 0.0f;
                    m_state = STATE_IDLE;
                }
                break;
            }

            if (m_timer >= 0.76666665f) {
                if (i == 0) {
                    frame = static_cast<RwFrame *>(m_clump12->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f + i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump12);
                } else {
                    frame = static_cast<RwFrame *>(m_clump12->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f + i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump12);
                    frame = static_cast<RwFrame *>(m_clump12->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f - i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump12);
                    frame = static_cast<RwFrame *>(m_clump22->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, (i - 1) * s_angleStep);
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump22);
                    frame = static_cast<RwFrame *>(m_clump22->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, -i * s_angleStep);
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump22);
                    if (i == 15.0f && m_state == STATE_FORWARD) {
                        m_state = STATE_DONE;
                    }
                }
            } else {
                float time = m_timer;

                if (i == 0) {
                    fn_80456C54(m_clump11, fn_80264DF8, &time);
                    frame = static_cast<RwFrame *>(m_clump11->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f + i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump11);
                } else {
                    fn_80456C54(m_clump11, fn_80264DF8, &time);
                    frame = static_cast<RwFrame *>(m_clump11->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f + i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump11);
                    fn_80456C54(m_clump11, fn_80264DF8, &time);
                    frame = static_cast<RwFrame *>(m_clump11->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, s_angleStep * (16.0f - i));
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump11);
                    fn_80456C54(m_clump21, fn_80264DF8, &time);
                    frame = static_cast<RwFrame *>(m_clump21->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, (i - 1) * s_angleStep);
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump21);
                    fn_80456C54(m_clump21, fn_80264DF8, &time);
                    frame = static_cast<RwFrame *>(m_clump21->object.parent);
                    fn_800E9CEC(frame, rwCOMBINEREPLACE, -i * s_angleStep);
                    fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_clump21);
                }
            }
        }
    }
}

void ElecCoreWall::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_80264838 context;

    context.m_owner = this;
    fn_8015AA84(&args, fn_80265CBC, &context);
}

void ElecCoreWall::OnCollisionEnd(ColliDetectArgs &args) {
    m_state = STATE_BACKWARD;
}

extern "C" void fn_802647B8(void) {
    fn_8006B474("DE0403COREWALL11.DFF", 0);
    fn_8006B474("DE0403COREWALL11.DMA", 0);
    fn_8006B474("DE0403COREWALL12.DFF", 0);
    fn_8006B474("DE0403COREWALL21.DFF", 0);
    fn_8006B474("DE0403COREWALL21.DMA", 0);
    fn_8006B474("DE0403COREWALL22.DFF", 0);
}

extern "C" void fn_802647B4(void) {}

void ElecCoreWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCoreWall(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8054C084 = {
    "ELEC_COREWALL", fn_802647B8, fn_802647B4, ElecCoreWall::Create, 0, 0, 0x7E9, 1, 0, 10, 0,
};
