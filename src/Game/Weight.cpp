/* The WEIGHT gadget (class and behaviour described in Game/Weight.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The model attack object's callback and the two probes' collision callbacks are boost::function
 * objects built with boost::bind(&Weight::OnAttack, this, _1) and its siblings; their managers,
 * invokers and copy functions come from the boost headers. */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses them. TNumEdit.h comes first, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). Revisit when the original inline functions or their
 * real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Weight.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The unscaled sizes (names inferred from their uses): ApplyScale scales s_modelSize for the
 * model's part 0, s_triggerSize for the trigger probe and s_pressSize for the press probe, and the
 * constructor creates the press and trigger probes with s_pressSize and s_triggerSize.
 *
 * The original's .bss for this file is 0xF8 bytes (0x8057FFA8-0x805800A0 in USA (GUPE8P)) and holds
 * the three vectors at 0xC, 0x18 and 0x24. ApplyScale and the static initialiser form the pooled
 * base address 0x8057FFA8, the first byte of the 0xC bytes before the vectors, but no instruction
 * in the file reads or writes those 0xC bytes or the 0xC8 bytes after the vectors, and what
 * occupies them is unknown. This source's .bss is the three vectors alone (0x24 bytes, at 0x0, 0xC
 * and 0x18). The pooled base relocations are the same, but the displacements from it are 0xC lower:
 * three words of ApplyScale and six of the static initialiser differ (the constructor reaches the
 * vectors through their own symbols and matches). Revisit when the original declarations of the
 * objects around them are found. */
static Fn8040Vector3 s_modelSize(30.0f, 15.0f, 20.0f);
static Fn8040Vector3 s_triggerSize(30.0f, 15.0f, 20.0f);
static Fn8040Vector3 s_pressSize(28.0f, 15.0f, 18.0f);

/* The fall and rise steps per update, the full health (set by ReadOwner and again by OnAttack when
 * the pause starts) and how far above the owner's height a caught character is placed (names
 * inferred). */
static const float s_fallStep = 2.5f;
static const float s_riseStep = 1.0f;
static const float s_maxHealth = 10.0f;
static const float s_catchHeight = 1.0f;

namespace Weight {

const char *Weight::s_name = "Weight";

} // namespace Weight

/* What the press probe's hit callback needs to know about the weight (placeholder name). */
struct UnkHitContext_8012ADA4 {
    UnkHitContext_8012ADA4() : m_height(0.0f), m_caught(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ float m_height;                     /* the height of the owner's position */
    /* 0x0C */ boost::weak_ptr<CharInfo> m_target; /* the last character caught */
    /* 0x14 */ bool m_caught;
};
CHECK_SIZEOF(UnkHitContext_8012ADA4, 0x18);

/* The callback that fn_8015AA84 calls for the character infos it takes from the press probe's hits.
 * When the weight's character info still exists and hit is not null, it sends hit, in this order, a
 * SetPositionCommand to its own x and z one unit above the owner's height, a DamageCommand of 1.0,
 * a Player::StartExternalControlCommand and a ClrVelocityCommand (a failed allocation passes null
 * to the command call), and records hit as the last character caught. */
static void fn_8012C544(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8012ADA4 *context = static_cast<UnkHitContext_8012ADA4 *>(arg);
    boost::shared_ptr<GadgetCharInfo> weightInfo = context->m_charInfo.lock();

    if (weightInfo.get() == 0) {
        return;
    }

    if (hit.get() != 0) {
        Fn8040Vector3 position = hit->m_position;
        position.y = s_catchHeight + context->m_height;
        hit->fn_801DC980(new SetPositionCommand(position, 4));
        hit->fn_801DC980(new DamageCommand(1.0f, 4));
        hit->fn_801DC980(new Player::StartExternalControlCommand(0, 4));
        hit->fn_801DC980(new ClrVelocityCommand(4));

        context->m_target = hit;
        context->m_caught = true;
    }
}

namespace Weight {

Weight::Weight(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x27, 4);
    ReadOwner(m_owner);

    m_model = fn_8015F800(0, "WEIGHT.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334("col");
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Weight::OnAttack, this, _1));
    }

    /* Each probe is a local of its own block: the original releases the press probe right after
     * its callback is set, before it creates the trigger probe. */
    {
        Fn8040Vector3 pressSize = s_pressSize;
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), pressSize, 4, 7, 0,
                        &lbl_805E07A0, &lbl_805E07B8, 0);
        m_pressHandle = fn_8015B8C4(probe);

        m_pressHandle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&Weight::OnPressCollision, this, _1)));
    }

    {
        Fn8040Vector3 triggerSize = s_triggerSize;
        boost::shared_ptr<CharaColliBase> probe = fn_800C7D04(
            fn_80021B08(), &m_charInfo->m_position, triggerSize, m_charInfo->m_rotation, 2, 0x40000,
            7, 0, boost::weak_ptr<CharInfo>(lbl_805EE698), lbl_805E07A0, &lbl_805E07B8, 0);
        m_triggerHandle = fn_8015B8C4(probe);

        m_triggerHandle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&Weight::OnTriggerCollision, this, _1)));
    }

    ApplyScale();
    m_holdFlags.Clear();
    UpdateModelMatrix();
}

Weight::~Weight() {}

void Weight::UpdateBody(float dt) {
    switch (m_state) {
    case STATE_START:
        if (m_params.sw == 0) {
            m_state = STATE_TOP_WAIT;
        } else {
            m_charInfo->m_position.y = m_baseY;
            m_state = STATE_BOTTOM_WAIT_SWITCH;
        }
        break;

    case STATE_TOP_WAIT_SWITCH:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_timer = 0.0f;
            m_state = STATE_TOP_WAIT;
        }
        break;

    case STATE_BOTTOM_WAIT_SWITCH:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_timer = 0.0f;
            m_state = STATE_BOTTOM_WAIT;
        }
        break;

    case STATE_TOP_WAIT:
        if (m_params.type == 0) {
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                m_state = STATE_FALL;
            }
        }
        break;

    case STATE_BOTTOM_WAIT:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = STATE_RISE;
            if (m_pressHandle.get() != 0) {
                m_pressHandle->m_probe->UnkSlot2(-1);
            }
        }
        break;

    case STATE_RISE:
        m_charInfo->m_position.y += s_riseStep;
        if (m_charInfo->m_position.y >= m_baseY + m_params.height) {
            m_charInfo->m_position.y = m_baseY + m_params.height;
            m_state = STATE_TOP_WAIT;
            m_timer = m_params.intervalUpper;

            if (m_holdFlags.Test(0)) {
                if (boost::shared_ptr<CharInfo> target = m_target.lock()) {
                    target->fn_801DC980(new Player::EndExternalControlCommand(4));
                }
                m_holdFlags.Reset(0);
            }

            if (m_params.sw == 1 && fn_8016904C(fn_800CB69C(), m_comId) != 1) {
                m_state = STATE_TOP_WAIT_SWITCH;
            }

            if (m_pressHandle.get() != 0) {
                m_pressHandle->m_probe->UnkSlot1(-1);
            }
        }
        break;

    case STATE_FALL:
        m_charInfo->m_position.y -= s_fallStep;
        if (m_charInfo->m_position.y <= m_baseY) {
            m_charInfo->m_position.y = m_baseY;
            m_state = STATE_BOTTOM_WAIT;
            m_timer = m_params.intervalLower;
            fn_801D54F4(fn_80053164(), 0x1019, &m_charInfo->m_position, 0, 1);
        }
        break;

    case STATE_PAUSED:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = m_resumeState;
            if (m_state == STATE_BOTTOM_WAIT) {
                m_timer = m_params.intervalLower;
            } else if (m_state == STATE_TOP_WAIT) {
                m_timer = m_params.intervalUpper;
            }
        }
        break;

    case STATE_INACTIVE:
        /* Matching workaround: STATE_INACTIVE (set by ReadOwner when "height" <= 0) does nothing,
         * but the original's jump table has an entry for it (nine entries, bounds check against 8);
         * without this case the table stops at STATE_PAUSED (bounds check against 7). Revisit if a
         * natural form produces the nine-entry table without it. */
        break;
    }

    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_pressHandle.get() != 0) {
        fn_8015BE9C(m_pressHandle.get(), dt);
    }
    if (m_triggerHandle.get() != 0) {
        fn_8015BE9C(m_triggerHandle.get(), dt);
    }
}

void Weight::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        ApplyScale();
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
    if (m_pressHandle.get() != 0) {
        fn_8015BE64(m_pressHandle.get());
    }
    if (m_triggerHandle.get() != 0) {
        fn_8015BE64(m_triggerHandle.get());
    }
}

void Weight::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);

    m_baseY = m_charInfo->m_position.y;
    m_charInfo->m_position.y += m_params.height;
    m_timer = 0.0f;
    m_health = s_maxHealth;
    m_holdFlags.Clear();

    if (m_params.height <= 0.0f) {
        m_state = STATE_INACTIVE;
        m_resumeState = STATE_INACTIVE;
    } else {
        m_state = STATE_START;
        m_resumeState = STATE_START;
    }
}

void Weight::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);

        fn_8040B790(&matrix, Fn8040Vector3(m_params.scaleX, m_params.scaleY, m_params.scaleZ));
        m_model->fn_801633DC(&matrix);
    }

    if (m_triggerHandle.get() != 0) {
        Fn8040Vector3 position;
        UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

        if (m_owner != 0) {
            fn_800C9F14(m_owner, &position);
            fn_800C9ED4(m_owner, &rotation);
        }

        m_triggerHandle->m_probe->SetPosition(&position);
        m_triggerHandle->m_probe->SetRotation(rotation);
    }
}

void Weight::ApplyScale() {
    if (m_model.get() != 0) {
        Fn8040Vector3 size = s_modelSize;
        size.x *= m_params.scaleX;
        size.y *= m_params.scaleY;
        size.z *= m_params.scaleZ;
        m_model->fn_801617D0(0, -2, size.x, size.y, size.z);

        Fn8040Vector3 offset(0.0f, size.y, 0.0f);

        m_model->fn_80161740(0, &offset, -2);
    }

    if (m_pressHandle.get() != 0) {
        Fn8040Vector3 size = s_pressSize;
        size.x *= m_params.scaleX;
        size.z *= m_params.scaleZ;
        m_pressHandle->m_probe->SetSize(size.x, -2, size.y, size.z);
    }

    if (m_triggerHandle.get() != 0) {
        Fn8040Vector3 size = s_triggerSize;
        size.x *= m_params.scaleX;
        size.y = 0.5f * m_params.height;
        size.z *= m_params.scaleZ;
        m_triggerHandle->m_probe->SetSize(size.x, -2, size.y, size.z);

        Fn8040Vector3 offset(0.0f, size.y, 0.0f);

        m_triggerHandle->m_probe->UnkSlot17(&offset, -2);
    }
}

void Weight::OnPressCollision(ColliDetectArgs &args) {
    if (m_state == STATE_FALL) {
        UnkHitContext_8012ADA4 context;

        context.m_charInfo = m_charInfo;

        Fn8040Vector3 position;

        fn_800C9F14(m_owner, &position);
        context.m_height = position.y;
        fn_8015AA84(&args, fn_8012C544, &context);

        if (context.m_caught) {
            m_holdFlags.Set(0);
            m_target = context.m_target;
        }
    }
}

void Weight::OnTriggerCollision(ColliDetectArgs &args) {
    if (m_params.type == 1 && m_state == STATE_TOP_WAIT && fn_8015AC84(args)) {
        m_state = STATE_FALL;
    }
}

void Weight::OnAttack(AttackCallbackParam &param) {
    if (m_state != STATE_PAUSED) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 2) {
                param.field_0x54 = 5;
                return;
            }
        }

        param.field_0x54 = 1;
        m_health -= param.field_0x50;
        if (m_health <= 0.0f) {
            m_resumeState = m_state;
            m_state = STATE_PAUSED;
            m_health = s_maxHealth;
            m_timer = m_params.intervalDamage;
        }
    } else {
        param.field_0x54 = 5;
    }
}

} // namespace Weight

extern "C" void fn_8012AC28(void) {
    fn_8006B474("WEIGHT.BIN", 1);
}

extern "C" void fn_8012AC24(void) {}

void Weight::Weight::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Weight(parent, owner);
}

/* The names of the values of the "type" and "sw" parameters. */
static const char *s_typeLabels[2] = {"type : Piston", "type : Fall"};
static const char *s_switchLabels[2] = {"switch : disable", "switch : enable"};

/* The label callback of the descriptor's "type" and "sw" parameters (behaviour inferred). */
static const char *fn_8012AB88(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    }
    if (cursor->m_index == 5) {
        return s_switchLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_8012AB88, 0},
    {4, "height", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "intv upper(sec)", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "intv lower(sec)", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "intv damage(sec)", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "sw", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_8012AB88, 0},
    {4, "scale X", 0, 0, 0, 1.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528344 = {
    "WEIGHT", fn_8012AC28, fn_8012AC24, Weight::Weight::Create, 0, 0, 0x19, 1, 0, 10, s_paramInfo,
};
