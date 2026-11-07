/* The scattered rings (class described in Game/ScatteringRing.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The command handler and the draw callback are boost::function objects built with
 * boost::bind(&ScatteringRing::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The RTTI data of CharaColliBaseCharInfo.h comes before that of TNumEdit.h at the start of the
 * unit's .data, .sdata and .rodata, so CharaColliBaseCharInfo.h is included before TNumEdit.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/TNumEdit.h"
#include "Game/ScatteringRing.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommand.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkRenderManager_800289B0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80238A78.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"

using SonicteamUSA::System::Collision::CollisionResultList;

/* Declared in ScatteringRing.h. */
const s32 lbl_805F3CA8 = 10;
const float lbl_805F3CAC = 1.5f;
const float lbl_805F3CB0 = 5.0f;
const float lbl_805F3CB4 = 60.0f;
const float lbl_805F3CB8 = 1.5f;

namespace ScatteringRing {

/* What the hit callback needs to know (placeholder name): whether a call of the callback got as
 * far as fn_8015F6EC, and the ring's field_0x34. */
struct UnkHitContext_8011A04C {
    UnkHitContext_8011A04C() : hit(false), field_0x1(false) {}

    /* 0x0 */ bool hit;
    /* 0x1 */ bool field_0x1;
};
CHECK_SIZEOF(UnkHitContext_8011A04C, 0x2);

void ScatteringRing::Create(const UnkScatterRecord_800B7E40 &record, bool arg1) {
    UnkRingParams_8011B170 param;
    /* Matching workaround: velocity is declared before speed, which gives the original's frame
     * (speed at 0x14, velocity at 0x20); declared after speed, the two swap places. Revisit if the
     * original form of these locals is identified. */
    Fn8040Vector3 velocity;
    Fn8040Vector3 speed(0.0f, 5.109375f, 1.15625f);
    int num = record.num > 0 ? record.num : lbl_805F3CA8;

    float angleStep = DegreesToRadians(360.0f / num);
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);

    RpClump *clump = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0, 0x7C));

    fn_8040B6F8(&matrix);
    fn_8040B848(&matrix, DegreesToRadians(360.0f * fn_8040EE90()));

    for (int i = 0; i < num; i++) {
        fn_8040B848(&matrix, angleStep);
        velocity = matrix * speed;
        param.position = record.position;
        param.velocity = velocity;
        fn_8040E970(&param.rotation);
        param.field_0x24 = 0.0f;
        param.time = 5.0f;
        param.floor = 6.0f;
        param.clump = clump;
        param.field_0x34 = arg1;
        new ScatteringRing(fn_801EB304(fn_8001C4F8(), 11), param);
    }

    fn_801D560C(fn_80053164(), 0xE00C, 0, 0);
}

/* The callback that fn_8015B004 calls for each hit whose collision object and character info
 * still exist and whose character info's fn_80078B2C returns true. For a character info whose
 * fn_80078B34 also returns true, and only when a context was passed and its hit flag is still
 * clear (no earlier call for the same probe got there first), it passes the character info to
 * fn_8015F6EC with 0, 1 and 0 (which sends a new command to its handler, a null one when the
 * allocation fails), passes it to fn_8020642C with 0 when field_0x1 is set, and sets the hit flag.
 * arg is the hit context that IsTouched passes. */
static void fn_8011B184(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8011A04C *context = static_cast<UnkHitContext_8011A04C *>(arg);

    if (hit->fn_80078B34() && context != 0 && !context->hit) {
        fn_8015F6EC(hit, 0, 1, 0);

        if (context->field_0x1) {
            fn_8020642C(hit, 0);
        }

        context->hit = true;
    }
}

const char *ScatteringRing::s_name = "ScatteringRing";

ScatteringRing::ScatteringRing(Task *parent, const UnkRingParams_8011B170 &param) : Task(parent) {
    m_name = s_name;
    m_param = param;
    m_charInfo = fn_8015CD90(0x26, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ScatteringRing::OnCommand, this, _1));
    m_layer = fn_8004DFB0(fn_80046178()->field_0x8, "ScatteringRing");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ScatteringRing::OnDraw, this, _1));
    m_probe = fn_800C7EA8(fn_80021B08(), &m_param.position, 0x40000, 0xA, 0, lbl_805EE698,
                          &lbl_805E07A0, 5.0f);
    m_target = fn_801E7128(m_charInfo, 0x180);
    m_flags.Clear();
    m_time = 10.0f;
}

ScatteringRing::~ScatteringRing() {
    fn_801E7108(m_target);

    if (m_probe.get() != 0) {
        fn_800C78B0(fn_80021B08(), &m_probe);
    }
}

void ScatteringRing::Update(float dt) {
    if (m_flags.Test(0) && m_probe.get() != 0) {
        CollisionResultList *hits = m_probe->GetHits();

        if (hits != 0 && hits->field_0x4 > 0) {
            ColliDetectArgs args;

            args.m_hits = hits;
            args.field_0x4 = 0;

            if (IsTouched(args)) {
                fn_801D5300(fn_80053164());
                fn_8015D794(&m_param.position, 0x8A);
                Task::m_flags |= 1;
                return;
            }
        }
    }

    if (m_flags.Test(1)) {
        if (m_flags.Test(2)) {
            m_flags.Reset(2);
        } else {
            m_flags.Set(2);
        }
    } else if (m_param.time <= lbl_805F3CB8) {
        m_flags.Set(1);
    }

    if (m_flags.Test(0)) {
        m_param.time -= dt;

        if (m_param.time <= 0.0f) {
            Task::m_flags |= 1;
            return;
        }
    }

    m_time -= dt;

    if (m_time <= 0.0f) {
        Task::m_flags |= 1;
        return;
    }

    m_param.floor = 5.0f + GetGroundHeight(m_param.position);
    /* The original negates each component (fneg) before multiplying it by its damping factor
     * (0x8011A278-0x8011A2A0). Written as x -= k * x (and z), the compiler emits one fnmsubs
     * without the fneg, and y -= k * y + c also loses its fneg. */
    m_param.velocity.x += 0.015627f * -m_param.velocity.x;
    m_param.velocity.y += 0.1f * -m_param.velocity.y - 0.12138f;
    m_param.velocity.z += 0.015627f * -m_param.velocity.z;
    fn_8040CFF8(&m_param.position, &m_param.velocity);

    if (m_param.position.y < m_param.floor && m_param.velocity.y < 0.0f) {
        m_param.position.y = m_param.floor;
        m_param.velocity.x *= 0.9f;
        m_param.velocity.y *= -1.11879f;
        m_param.velocity.z *= 0.9f;
        m_flags.Set(0);
    }

    m_param.rotation.y += DegreesToRadians(1.5f);

    if (m_probe.get() != 0) {
        m_probe->SetPosition(&m_param.position);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_param.position);
    }
}

bool ScatteringRing::IsTouched(ColliDetectArgs &args) {
    UnkHitContext_8011A04C context;

    context.field_0x1 = m_param.field_0x34;
    fn_8015B004(&args, fn_8011B184, &context);
    return context.hit;
}

float ScatteringRing::GetGroundHeight(const Fn8040Vector3 &position) {
    Fn8040Vector3 start(position);
    Fn8040Vector3 end(position);

    end.y += -10000.0f;

    UnkSegment_80020A20 segment(start, end);
    CollisionResultList *hits = fn_80021FE4(fn_80020D40(), &segment, 1, 0, 0, 0);

    if (hits != 0) {
        for (UnkProbeIterator_80027374 it(hits->fn_8001EE04()); it.node != hits->fn_8001EDEC();
             fn_80027398(&it)) {
            UnkProbeNode_80021FE4 *node = it.node;

            if (node != 0 && node->field_0xC != 0 && node->hit != 0) {
                Fn8040Vector3 point(*fn_8001E9BC(node->hit));

                delete hits;
                return point.y;
            }
        }
    }

    delete hits;
    return 0.0f;
}

void ScatteringRing::OnDraw(RednerDrawArgs &args) {
    if (m_flags.Test(2)) {
        return;
    }

    RwSphere sphere;

    sphere.radius = lbl_805F3CB0;
    fn_80048390(fn_800289B0(), 5);
    fn_800289B0()->ApplyIndex();

    if (m_param.clump != 0) {
        sphere.center.x = m_param.position.x;
        sphere.center.y = m_param.position.y;
        sphere.center.z = m_param.position.z;

        if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
            RwFrame *frame = static_cast<RwFrame *>(m_param.clump->object.parent);

            fn_800E9CEC(frame, rwCOMBINEREPLACE, m_param.rotation.y);
            fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
            fn_80456BC4(m_param.clump);
        }
    }

    fn_80047780(fn_800289B0());
}

void ScatteringRing::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14: {
        UnkRotation3 rotation(m_param.rotation);

        fn_80238A78(&command, &m_param.position, &rotation, m_param.field_0x34);
        Task::m_flags |= 1;
        command.m_flags |= 2;
        break;
    }
    }
}

} // namespace ScatteringRing
