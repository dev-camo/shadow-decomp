/* The CaptureCage gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The attack object's callback and the model's collision callback are boost::function objects
 * built with boost::bind(&CaptureCage::method, this, _1); their managers, invokers and copy
 * functions come from the boost headers. */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses them. TNumEdit.h comes first, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). Revisit when the original inline functions or their
 * real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CaptureCage.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The model file that the constructor loads and the descriptor's load hook lists, and the crash
 * effect's clump file. */
static const char *s_modelFile = "CAPTURECAGE.BIN";
static const char *s_crashFile = "CAPTCAGECRSH01.DFF";

namespace CaptureCage {

const char *CaptureCage::s_name = "CaptureCage";

} // namespace CaptureCage

/* What the hit callback needs (placeholder name): the cage's position on input, and the nearest
 * character info found so far, its squared distance and whether there is one on output. */
struct UnkHitContext_800F36E0 {
    UnkHitContext_800F36E0() : m_distanceSquared(100000000.0f), m_found(false) {}

    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ boost::weak_ptr<CharInfo> m_nearest;
    /* 0x14 */ float m_distanceSquared;
    /* 0x18 */ bool m_found;
};
CHECK_SIZEOF(UnkHitContext_800F36E0, 0x1C);

/* The callback that fn_8015B70C calls for the character infos of the collision results: when the
 * hit's squared distance from the context's position is smaller than the context's distance (false
 * for NaN), stores the hit, its squared distance and true in the context; a null context does
 * nothing. */
static void fn_800F48A8(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_800F36E0 *context = static_cast<UnkHitContext_800F36E0 *>(arg);

    if (context != 0) {
        const Fn8040Vector3 &offset = context->m_position - hit->m_position;
        float distanceSquared = fn_8040CF2C(offset);

        if (distanceSquared < context->m_distanceSquared) {
            context->m_distanceSquared = distanceSquared;
            context->m_nearest = hit;
            context->m_found = true;
        }
    }
}

namespace CaptureCage {

CaptureCage::CaptureCage(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x16, 4, "CaptureCage");
    CopyFromOwner(owner);
    m_model = fn_8015F800(0, s_modelFile);
    m_model->fn_80162A68(0, 0);

    if (m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo))) {
        boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161428(0);

        if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&CaptureCage::OnAttack, this, _1));
        }
    }

    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&CaptureCage::OnCollision, this, _1));
    m_model->fn_80161FC8(1, boost::weak_ptr<CharInfo>(m_charInfo));
    UpdateModelMatrix();
}

CaptureCage::~CaptureCage() {}

void CaptureCage::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_INTACT;
        break;

    case STATE_BREAKING:
        m_model->fn_80161880();
        m_model->fn_80162804();
        StartCrashEffect();
        m_model->fn_80162A68(1, 0);
        m_model->fn_80161FC8(1, boost::weak_ptr<CharInfo>(m_charInfo));
        UpdateModelMatrix();
        m_timer = 10.0f;
        m_state = STATE_CRASHING;

        if (!m_captured.expired()) {
            if (boost::shared_ptr<CharInfo> captured = m_captured.lock()) {
                captured->fn_801DC980(new ReleaseCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
            }

            m_captured.reset();
            m_stateFlags.Reset(FLAG_BIT0);
        }

        m_stateFlags.Set(FLAG_BIT1);
        break;

    case STATE_CRASHING:
        m_timer -= dt;

        if (!m_stateFlags.Test(FLAG_BIT2) && m_timer <= 2.0f) {
            m_model->fn_8016268C(-1.0f);
            m_stateFlags.Set(FLAG_BIT2);
        }

        if (m_timer <= 0.0f) {
            m_state = STATE_DONE;
        }
        break;

    case STATE_DONE:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void CaptureCage::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
    }
}

void CaptureCage::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_START;
    m_health = 5.0f;
    m_timer = 0.0f;
}

void CaptureCage::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void CaptureCage::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_state == STATE_INTACT) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 3) != 0) {
                /* The original stores 5 again here (0x800F3888). */
                param.field_0x54 = 5;
                return;
            }

            m_health -= param.field_0x50;

            if (m_health <= 0.0f) {
                param.field_0x54 = 3;
                m_state = STATE_BREAKING;

                if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                    fn_8020642C(attacker, 0x1F);
                }
            } else {
                param.field_0x54 = 1;
            }

            /* Matching workaround: this return ends the block of the locked attack object. The
             * original destroys that pointer at the end of the block and branches to the end of
             * the function (0x800F392C-0x800F3938), and destroys it again on its own for a failed
             * lock (0x800F393C); without the return both paths share one destructor call (79
             * words instead of the original's 83). Revisit if another form gives the original's
             * layout. */
            return;
        }
    }
}

void CaptureCage::StartCrashEffect() {
    RpClump *clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 0, s_crashFile));

    if (clump != 0) {
        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x4.y += 20.0f;
        effect.field_0x34 = 1.3f;
        effect.field_0x44 = clump;
        effect.field_0x54 = 1;
        effect.field_0x4C = 150;
        effect.field_0x40 = m_charInfo->m_position.y;
        effect.field_0x4A = 1;
        fn_8015D93C(&effect);
    }
}

void CaptureCage::OnCollision(ColliDetectArgs &args) {
    if (m_stateFlags.Test(FLAG_BIT1)) {
        return;
    }

    if (m_stateFlags.Test(FLAG_BIT0)) {
        return;
    }

    UnkHitContext_800F36E0 context;

    context.m_position = m_charInfo->m_position;
    fn_8015B70C(&args, fn_800F48A8, &context, 3);

    if (context.m_found) {
        m_captured = context.m_nearest;

        if (boost::shared_ptr<CharInfo> captured = m_captured.lock()) {
            captured->fn_801DC980(new BindCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
        }
    }
}

} // namespace CaptureCage

/* The descriptor's hookA: lists the model file that the constructor loads. */
extern "C" void fn_800F34E0(void) {
    fn_8006B474(s_modelFile, 1);
}

/* The descriptor's hookB; empty. */
extern "C" void fn_800F34DC(void) {}

void CaptureCage::CaptureCage::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new CaptureCage(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "N/A", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "N/A", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "N/A", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525118 = {
    "CAPTURE_CAGE",
    fn_800F34E0,
    fn_800F34DC,
    CaptureCage::CaptureCage::Create,
    0,
    0,
    0x37,
    1,
    0,
    10,
    s_paramInfo,
};
