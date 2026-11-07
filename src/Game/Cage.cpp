/* The Cage gadget (behaviour in the class comment). File name inferred from the class. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&Cage::OnCollision, this, _1); its manager, invoker and copy functions come from the
 * boost headers. */
#include "Game/Cage.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"

/* The three factors that the constructor and ResetFromOwner multiply by the scale parameters
 * before passing them to the model's fn_801617D0 for the loaded entry (placeholder name; their
 * meaning is unknown). */
static Fn8040Vector3 s_size(10.0f, 12.5f, 10.0f);

/* The same kind of factors for model entries 0 and 1, which the update passes for the entry two
 * above the loaded one once the com id query returns 1 (placeholder name). */
static Fn8040Vector3 s_switchedSizes[2] = {Fn8040Vector3(10.0f, 4.0f, 10.0f),
                                           Fn8040Vector3(10.0f, 3.0f, 10.0f)};

/* What the hit callback needs to know (placeholder name): the cage's character info and which
 * command to send, 0 for a BindCommand and 1 for a ReleaseCommand. */
struct UnkHitContext_800F22BC {
    UnkHitContext_800F22BC() : m_command(0) {}

    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ s32 m_command;
};
CHECK_SIZEOF(UnkHitContext_800F22BC, 0xC);

/* The callback that fn_8015B70C calls for the character infos of the collision results: passes the
 * character info's fn_801DC980 a new BindCommand (m_command 0) or ReleaseCommand (m_command 1)
 * holding the cage's character info (a null command when the allocation fails; the call still
 * happens); any other value, or a null context, sends nothing. */
static void fn_800F3070(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_800F22BC *context = static_cast<UnkHitContext_800F22BC *>(arg);

    if (context == 0) {
        return;
    }

    if (context->m_command == 0) {
        hit->fn_801DC980(new BindCommand(boost::weak_ptr<CharInfo>(context->m_charInfo)));
    } else if (context->m_command == 1) {
        hit->fn_801DC980(new ReleaseCommand(boost::weak_ptr<CharInfo>(context->m_charInfo)));
    }
}

/* Multiplies the atomic's bounding sphere radius by the float that data points to, after updating
 * the sphere when the atomic's interpolator marks it dirty; does nothing when atomic or data is
 * null. Returns atomic, so the walk goes on. */
static RpAtomic *fn_800F3014(RpAtomic *atomic, void *data) {
    if (atomic != NULL && data != NULL) {
        RpAtomicGetBoundingSphere(atomic)->radius *= *static_cast<float *>(data);
    }
    return atomic;
}

/* Passes each atomic of the part's clump to fn_800F3014 with data; returns part, so the walk goes
 * on. */
static UnkModelPart_8015FA78 *fn_800F2FD4(UnkModelPart_8015FA78 *part, void *data) {
    fn_80456C54(part->clump, fn_800F3014, data);
    return part;
}

namespace Cage {

const char *Cage::s_name = "Cage";

Cage::Cage(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x3A, 4, "Cage");
    CopyFromOwner(m_owner);
    m_model = fn_8015F800(0, "CASE.BIN");
    m_loadedModel = m_params.model;
    m_model->fn_80162A68(m_loadedModel, 0);
    m_model->fn_80161FC8(m_loadedModel, m_charInfo);
    m_model->m_collisionCallback = ColliDetectCallback(boost::bind(&Cage::OnCollision, this, _1));
    m_model->fn_80160C28(m_loadedModel);
    m_model->fn_8015FF60();

    /* Matching workaround: this block, which passes the loaded entry's size to the model, is
     * written out here and in ResetFromOwner, where the original has the same instructions. An
     * inline member function holding it stays out of line, because the inline calls inside it
     * (the shared pointer's operator-> and Fn8040Vector3's constructor) are at the compiler's
     * inline depth limit. Revisit if a helper form that the compiler expands is found. */
    float scaledY = s_size.y * m_params.scaleY;

    m_model->fn_801617D0(m_loadedModel, -2, s_size.x * m_params.scaleX, scaledY,
                         s_size.z * m_params.scaleZ);

    Fn8040Vector3 offset(0.0f, scaledY, 0.0f);

    m_model->fn_80161740(m_loadedModel, &offset, -2);
    ScaleBoundingSpheres();
    UpdateModelMatrix();
}

Cage::~Cage() {}

void Cage::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    /* NonMatching: the original's compare tree for this switch has one more branch (a blt). */
    switch (m_state) {
    case 0:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = 2;

            if (m_model.get() != 0) {
                m_model->fn_80160B18(m_loadedModel);
                m_model->fn_80161880();

                int switchedEntry = m_loadedModel + 2;

                m_model->fn_80161FC8(switchedEntry, m_charInfo);

                float scaledY = m_params.scaleY * s_switchedSizes[m_loadedModel].y;

                m_model->fn_801617D0(switchedEntry, -2,
                                     m_params.scaleX * s_switchedSizes[m_loadedModel].x, scaledY,
                                     m_params.scaleZ * s_switchedSizes[m_loadedModel].z);

                Fn8040Vector3 offset(0.0f, scaledY, 0.0f);

                m_model->fn_80161740(switchedEntry, &offset, -2);
            }

            switch (m_params.model) {
            case 0:
                fn_801D54F4(fn_80053164(), 0x1002, &m_charInfo->m_position, 0, 1);
                break;
            case 1:
                fn_801D54F4(fn_80053164(), 0x1001, &m_charInfo->m_position, 0, 1);
                break;
            }
        }
        break;
    }

    UpdateModelMatrix();
}

void Cage::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();

        if (m_params.model != m_loadedModel) {
            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_loadedModel = m_params.model;
            m_model->fn_80162A68(m_loadedModel, 0);
            m_model->fn_80161FC8(m_loadedModel, m_charInfo);
            m_model->fn_80160C28(m_loadedModel);
            m_model->fn_8015FF60();
            m_params.model = m_loadedModel;
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }

        /* The constructor's size block (see the note there). */
        float scaledY = s_size.y * m_params.scaleY;

        m_model->fn_801617D0(m_loadedModel, -2, s_size.x * m_params.scaleX, scaledY,
                             s_size.z * m_params.scaleZ);

        Fn8040Vector3 offset(0.0f, scaledY, 0.0f);

        m_model->fn_80161740(m_loadedModel, &offset, -2);
        ScaleBoundingSpheres();
        m_model->fn_8016108C();
    }
}

void Cage::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
    m_state = 0;
}

void Cage::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B790(&matrix, Fn8040Vector3(m_params.scaleX, m_params.scaleY, m_params.scaleZ));
        m_model->fn_801633DC(&matrix);
    }
}

/* The original builds a context and calls fn_8015B70C separately in each branch (two stack
 * contexts, two calls); one shared context compiles 15 words shorter. */
void Cage::OnCollision(ColliDetectArgs &args) {
    if (fn_8016904C(fn_800CB69C(), m_comId) != 1) {
        UnkHitContext_800F22BC context;

        context.m_charInfo = m_charInfo;
        fn_8015B70C(&args, fn_800F3070, &context, 4);
    } else {
        UnkHitContext_800F22BC context;

        context.m_charInfo = m_charInfo;
        context.m_command = 1;
        fn_8015B70C(&args, fn_800F3070, &context, 4);
    }
}

void Cage::ScaleBoundingSpheres() {
    if (m_model.get() != 0) {
        float scale = m_params.scaleX;

        if (scale < m_params.scaleY) {
            scale = m_params.scaleY;
        }
        if (scale < m_params.scaleZ) {
            scale = m_params.scaleZ;
        }

        m_model->fn_8015FA78(fn_800F2FD4, &scale);
    }
}

} // namespace Cage

/* The descriptor's hookA: lists the model file that the constructor loads. */
extern "C" void fn_800F2228(void) {
    fn_8006B474("CASE.BIN", 1);
}

/* The descriptor's hookB; empty. */
extern "C" void fn_800F2224(void) {}

void Cage::Cage::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Cage(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "scale X", 0, 0, 0, 1.0f, 1.0f, 10.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 1.0f, 10.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 1.0f, 10.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80524FCC = {
    "CAGE", fn_800F2228, fn_800F2224, Cage::Cage::Create, 0, 0, 7, 1, 0, 10, s_paramInfo,
};
