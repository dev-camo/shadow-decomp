/* The RING gadget (classes described in Game/Ring.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The command handler and the draw callback are boost::function objects built with
 * boost::bind(&Ring::method, this, _1); their manager, invoker and copy functions come from the
 * boost headers.
 *
 * NonMatching: UpdateBody's dispatch, the bool that CollectTouchedRings passes to IsTouched and
 * the register use of OnCommand's 0x14 loop differ from the original; see the note in
 * configure.py. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Ring.h"
#include <cmath>
#include "Game/AngleConversion.h"
#include "Game/CharCommand.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/FrameRotationAxes.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkRenderManager_800289B0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_80238A78.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"

using SonicteamUSA::System::Collision::CollisionResultList;

namespace RegularRing {

/* What the hit callback needs to know (placeholder name): whether the ring's bit 2 is set (the
 * ghost argument of IsTouched), and whether a call of the callback passed its fn_80078B34 test. */
struct UnkRingHitContext_80113AE8 {
    UnkRingHitContext_80113AE8() : m_ghost(false), m_hit(false) {}

    /* 0x0 */ bool m_ghost;
    /* 0x1 */ bool m_hit;
};
CHECK_SIZEOF(UnkRingHitContext_80113AE8, 0x2);

UnkRingElement_8011464C::UnkRingElement_8011464C()
    : m_rotation(0.0f, 0.0f, 0.0f), m_phase(0.0f), field_0x28(0.0f), field_0x2C(0.0f), m_clump(0),
      m_ghostClump(0), m_suspendTime(0.0f) {
    m_flags.Set(1);
}

void UnkRingElement_8011464C::UpdateSuspension(float dt) {
    if (m_flags.Test(3)) {
        m_suspendTime -= dt;

        if (m_suspendTime <= 0.0f) {
            if (m_target.get() != 0) {
                m_target->SetFlags(1);
            }

            if (m_probe.get() != 0) {
                m_probe->UnkSlot1(-1);
            }

            m_flags.Reset(3);
        }
    }
}

void UnkRingElement_8011464C::Suspend(float time) {
    m_flags.Set(3);
    m_suspendTime = time;

    if (m_target.get() != 0) {
        m_target->ClearFlags(1);
    }

    if (m_probe.get() != 0) {
        m_probe->UnkSlot2(-1);
    }
}

/* The callback that fn_8015B004 calls for each hit whose collision object and character info
 * still exist and whose character info's fn_80078B2C returns true. For a character info whose
 * fn_80078B34 also returns true, and only when a context was passed, it requests an item for the
 * character info (fn_8015F6EC with 0, 1 and 0, which sends a new command to its handler, a null
 * one when the allocation fails), passes it to fn_8020642C with 0 and tries to play sound 0xE00B
 * (fn_801D5300), unless m_ghost is set or m_hit is already set (an earlier call for the same
 * probe got there first); then it sets m_hit. arg is the hit context that IsTouched passes. */
static void fn_8011445C(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkRingHitContext_80113AE8 *context = static_cast<UnkRingHitContext_80113AE8 *>(arg);

    if (hit->fn_80078B34() && context != 0) {
        if (!context->m_ghost && !context->m_hit) {
            fn_8015F6EC(hit, 0, 1, 0);
            fn_8020642C(hit, 0);
            fn_801D5300(fn_80053164());
        }

        context->m_hit = true;
    }
}

const char *Ring::s_name = "Ring";

Ring::Ring(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_rings = 0;
    m_count = 0;
    m_charInfo = fn_8015CD90(0x26, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Ring::OnCommand, this, _1));
    ReadOwner(m_owner);
    CreateRings(m_params);
    PlaceRings(m_params);
    CollectTouchedRings();
    UpdatePositions();
}

Ring::~Ring() {
    if (!fn_800C9870(m_owner)) {
        fn_800C9894(m_owner);
        fn_800C9930(m_owner, new RingState(m_rings, m_count));
    }

    DestroyRings();
}

void Ring::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (m_comId > 0) {
            m_state = 1;
            m_flags.Set(0);
            ClearTargetFlags();
        } else {
            m_state = 2;
            m_flags.Reset(0);
        }
        break;

    case 1:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = 2;
            m_flags.Reset(0);
            SetTargetFlags();
        }
        break;

    case 2:
        if (m_rings != 0) {
            bool anyThere = false;

            for (int i = 0; i < m_count; i++) {
                if (m_rings[i].m_flags.Test(1)) {
                    m_rings[i].UpdateSuspension(dt);
                    m_rings[i].m_phase += 0.01f;
                    m_rings[i].m_rotation.y += 0.05f * std::sin(m_rings[i].m_phase);
                    anyThere = true;
                }
            }

            if (!anyThere) {
                if (m_params.ghost == 1) {
                    fn_8015A7C0(this);
                } else {
                    fn_8015A794(this);
                }

                Task::m_flags |= 1;
            } else {
                CollectTouchedRings();
            }
        }
        break;
    }
}

void Ring::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);

    if (m_params.num != m_count) {
        if (m_params.num > 0) {
            DestroyRings();
            CreateRings(m_params);
            PlaceRings(m_params);
        }
    } else if (m_rings != 0) {
        PlaceRings(m_params);
    }

    CollectTouchedRings();
    UpdatePositions();
    fn_80112A04();
}

void Ring::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_flags.Clear();
}

bool Ring::IsTouched(ColliDetectArgs &args, bool ghost) {
    UnkRingHitContext_80113AE8 context;

    context.m_ghost = ghost;
    fn_8015B004(&args, fn_8011445C, &context);
    return context.m_hit;
}

void Ring::CreateRings(const RingParams &params) {
    if (params.num > 0) {
        m_count = params.num;
        m_rings = new UnkRingElement_8011464C[m_count];

        RpClump *clump = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0, 0x7C));
        RpClump *ghostClump = static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0, 0x5E));

        for (int i = 0; i < m_count; i++) {
            fn_8040CE60(&m_rings[i].m_position);
            fn_8040CE60(&m_rings[i].field_0xC);
            fn_8040E970(&m_rings[i].m_rotation);
            m_rings[i].m_phase = 0.0f;
            m_rings[i].m_clump = clump;
            m_rings[i].m_ghostClump = ghostClump;
        }

        if (RingState *state = dynamic_cast<RingState *>(m_owner->m_adapterState)) {
            /* ghost is read from m_params and num from params, as the bytes show (lwz r0,0x50(r31)
             * with this in r31, at 0x80113698); reading params.ghost here keeps params in one more
             * saved register through the whole function, which is one word longer. */
            /* Matching workaround: two calls that differ only in the last argument, as in the
             * original (0x801136A0-0x801136C4); passing m_params.ghost != 0 (or the same as a
             * conditional expression) to one call computes the bool without a branch and is 6
             * words shorter. Revisit if Apply's parameter turns out not to be a bool. */
            if (m_params.ghost == 0) {
                state->Apply(m_rings, m_count, false);
            } else {
                state->Apply(m_rings, m_count, true);
            }
        }

        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1)) {
                if (m_rings[i].m_flags.Test(2)) {
                    m_rings[i].m_target = fn_801E7128(m_charInfo, 8);
                } else {
                    m_rings[i].m_target = fn_801E7128(m_charInfo, 0x188);
                }

                m_rings[i].m_probe = fn_800C7EA8(fn_80021B08(), &m_rings[i].m_position, 0x40000,
                                                 0xA, 0, lbl_805EE698, &lbl_805E07A0, 5.0f);
            }
        }

        if (m_layer.get() == 0) {
            m_layer = fn_8004DFB0(fn_80046178()->field_0x8, "SetRing");
            m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&Ring::OnDraw, this, _1));
        }
    }
}

void Ring::DestroyRings() {
    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_target.get() != 0) {
            fn_801E7108(m_rings[i].m_target);
        }

        if (m_rings[i].m_probe.get() != 0) {
            fn_800C78B0(fn_80021B08(), &m_rings[i].m_probe);
        }
    }

    if (m_rings != 0) {
        delete[] m_rings;
        m_rings = 0;
    }

    m_count = 0;
}

void Ring::PlaceRings(const RingParams &params) {
    if (m_rings == 0) {
        return;
    }

    switch (params.type) {
    case TYPE_NORMAL:
        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1)) {
                m_rings[i].m_position = m_charInfo->m_position;
                m_rings[i].m_rotation.y = m_charInfo->m_rotation.y;
            }
        }
        break;

    case TYPE_LINER: {
        /* The rings are spaced along the character info's -z axis, the first at its position. */
        float spacing = params.param0 / m_count;
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 offset(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1)) {
                offset.z = -spacing * i;
                m_rings[i].m_position = matrix * offset;
                m_rings[i].m_rotation.y = m_charInfo->m_rotation.y;
            }
        }
        break;
    }

    case TYPE_CIRCLE: {
        /* The rings are spread evenly around the character info's y axis, at the radius along
         * its z axis. */
        float radius = params.param0;
        float angleStep = DegreesToRadians(360.0f / m_count);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);

        Fn8040Vector3 offset(0.0f, 0.0f, radius);

        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1)) {
                fn_8040B6F8(&matrix);
                fn_8040B59C(&matrix, m_charInfo->m_position);
                fn_8040B848(&matrix, m_charInfo->m_rotation.y);
                fn_8040B884(&matrix, m_charInfo->m_rotation.x);
                fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
                fn_8040B848(&matrix, angleStep * i);
                m_rings[i].m_position = matrix * offset;
                m_rings[i].m_rotation.y = m_charInfo->m_rotation.y;
            }
        }
        break;
    }

    case TYPE_PARABOLA: {
        /* The rings are spaced along the character info's z axis over the length, and offset
         * along its x axis by the height times the sine of an angle that goes from 0 towards 180
         * degrees. */
        float spacing = params.param0 / m_count;
        float height = params.param1;
        float angleStep = DegreesToRadians(180.0f / m_count);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);

        Fn8040Vector3 offset(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1)) {
                offset.x = height * std::sin(angleStep * i);
                offset.z = spacing * i;
                fn_8040B6F8(&matrix);
                fn_8040B59C(&matrix, m_charInfo->m_position);
                fn_8040B848(&matrix, m_charInfo->m_rotation.y);
                fn_8040B884(&matrix, m_charInfo->m_rotation.x);
                fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
                m_rings[i].m_position = matrix * offset;
                m_rings[i].m_rotation.y = m_charInfo->m_rotation.y;
            }
        }
        break;
    }
    }
}

void Ring::CollectTouchedRings() {
    ColliDetectArgs args;

    for (int i = 0; i < m_count; i++) {
        /* Matching workaround: the probe is tested as m_probe != 0, which converts it with operator
         * bool and keeps the result (addic, subfe and clrlwi. at 0x80112B34-0x80112B3C) as the
         * original does; if (m_rings[i].m_probe) tests the pointer with one cmplwi (2 words
         * shorter). Revisit if the original form of this test is identified. */
        if (m_rings[i].m_probe != 0) {
            CollisionResultList *hits = m_rings[i].m_probe->GetHits();

            if (hits != 0 && hits->field_0x4 > 0) {
                args.m_hits = hits;
                args.field_0x4 = 0;

                if (IsTouched(args, m_rings[i].m_flags.Test(2))) {
                    if (m_params.ghost == 1) {
                        m_rings[i].m_flags.Set(2);
                        m_rings[i].Suspend(1.0f);
                        m_rings[i].m_target->SetFlagWord(8);
                    } else {
                        fn_800C78B0(fn_80021B08(), &m_rings[i].m_probe);
                        fn_801E7108(m_rings[i].m_target);
                        m_rings[i].m_flags.Reset(1);
                        fn_8015D794(&m_rings[i].m_position, 0x8A);
                    }
                }
            }
        }
    }
}

void Ring::UpdatePositions() {
    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_target.get() != 0) {
            fn_800CCE38(m_rings[i].m_target.get(), m_rings[i].m_position);
        }

        if (m_rings[i].m_probe.get() != 0) {
            m_rings[i].m_probe->SetPosition(&m_rings[i].m_position);
        }
    }
}

void Ring::fn_80112A04() {
    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_probe.get() != 0) {
            m_rings[i].m_probe->UnkSlot26();
        }
    }
}

void Ring::OnDraw(RednerDrawArgs &args) {
    if (m_flags.Test(0)) {
        return;
    }

    RwSphere sphere;

    sphere.radius = 5.0f;
    fn_80048390(fn_800289B0(), 5);
    fn_800289B0()->ApplyIndex();

    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_flags.Test(1)) {
            sphere.center.x = m_rings[i].m_position.x;
            sphere.center.y = m_rings[i].m_position.y;
            sphere.center.z = m_rings[i].m_position.z;

            if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
                if (m_rings[i].m_flags.Test(2)) {
                    RwFrame *frame = static_cast<RwFrame *>(m_rings[i].m_ghostClump->object.parent);

                    fn_800E9CEC(frame, rwCOMBINEREPLACE, m_rings[i].m_rotation.y);
                    fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_rings[i].m_ghostClump);
                } else {
                    RwFrame *frame = static_cast<RwFrame *>(m_rings[i].m_clump->object.parent);

                    fn_800E9CEC(frame, rwCOMBINEREPLACE, m_rings[i].m_rotation.y);
                    fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
                    fn_80456BC4(m_rings[i].m_clump);
                }
            }
        }
    }

    fn_80047780(fn_800289B0());
}

/* A VacuumCommand (id 0x14) or a UseCommand (id 0xD) whose weak pointer (m_target, field_0x1C)
 * locks to the target of a ring that is there collects the first such ring and returns. When
 * m_params.ghost == 1 the ring gets bit 2, Suspend(1.0f) runs and its target's flag word becomes
 * 8; otherwise its probe and target are released and its bit 1 is cleared. A VacuumCommand then
 * passes the command, the ring's position and rotation and 1 to fn_80238A78. A UseCommand
 * requests an item for the character info in m_user (fn_8015F6EC with 0, 1 and 0) and passes it
 * to fn_8020642C with 0 when m_user still locks; when ghost == 1 it does so only if the ring's
 * bit 2 was clear before. Bit 1 of the command's flags is set only when a ring is collected;
 * other command ids are ignored. */
void Ring::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14: {
        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1) &&
                m_rings[i].m_target == static_cast<VacuumCommand &>(command).m_target.lock()) {
                if (m_params.ghost == 1) {
                    m_rings[i].m_flags.Set(2);
                    m_rings[i].Suspend(1.0f);
                    m_rings[i].m_target->SetFlagWord(8);
                } else {
                    fn_800C78B0(fn_80021B08(), &m_rings[i].m_probe);
                    fn_801E7108(m_rings[i].m_target);
                    m_rings[i].m_flags.Reset(1);
                }

                fn_80238A78(&command, &m_rings[i].m_position, &m_rings[i].m_rotation, 1);
                command.m_flags |= 2;
                return;
            }
        }
        break;
    }

    case 0xD: {
        UseCommand &use = static_cast<UseCommand &>(command);

        for (int i = 0; i < m_count; i++) {
            if (m_rings[i].m_flags.Test(1) && m_rings[i].m_target == use.field_0x1C.lock()) {
                if (m_params.ghost == 1) {
                    if (!m_rings[i].m_flags.Test(2)) {
                        if (boost::shared_ptr<CharInfo> user = use.m_user.lock()) {
                            fn_8015F6EC(user, 0, 1, 0);
                            fn_8020642C(user, 0);
                        }
                    }

                    m_rings[i].m_flags.Set(2);
                    m_rings[i].Suspend(1.0f);
                    m_rings[i].m_target->SetFlagWord(8);
                } else {
                    fn_800C78B0(fn_80021B08(), &m_rings[i].m_probe);
                    fn_801E7108(m_rings[i].m_target);
                    m_rings[i].m_flags.Reset(1);

                    if (boost::shared_ptr<CharInfo> user = use.m_user.lock()) {
                        fn_8015F6EC(user, 0, 1, 0);
                        fn_8020642C(user, 0);
                    }
                }

                command.m_flags |= 2;
                return;
            }
        }
        break;
    }
    }
}

void Ring::SetTargetFlags() {
    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_flags.Test(1) && m_rings[i].m_target.get() != 0) {
            m_rings[i].m_target->SetFlags(1);
        }
    }
}

void Ring::ClearTargetFlags() {
    for (int i = 0; i < m_count; i++) {
        if (m_rings[i].m_flags.Test(1) && m_rings[i].m_target.get() != 0) {
            m_rings[i].m_target->ClearFlags(1);
        }
    }
}

} // namespace RegularRing

extern "C" void fn_801123A8(void) {
    fn_8006B50C(0xDA, 1);
}

extern "C" void fn_801123A4(void) {}

void RegularRing::Ring::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Ring(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The labels of the values of the "type" and "ghost" parameters, and those that the descriptor
 * shows for "param0" and "param1" for each type. */
static const char *s_typeLabels[4] = {"type : normal", "type : liner", "type : circle",
                                      "type : parabola"};
static const char *s_param0Labels[4] = {"", "length", "radius", "length"};
static const char *s_param1Labels[4] = {"", "", "", "height"};
static const char *s_ghostLabels[2] = {"ghost off", "ghost on"};

/* The label callback of the descriptor's "type", "param0", "param1" and "ghost" entries: the
 * labels of "param0" and "param1" follow the type last shown (behaviour inferred). */
static const char *fn_801122A8(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_type = 0;

    if (cursor->m_index == 0) {
        s_type = cursor->m_value;
        return s_typeLabels[cursor->m_value];
    } else if (cursor->m_index == 2) {
        return s_param0Labels[s_type];
    } else if (cursor->m_index == 3) {
        return s_param1Labels[s_type];
    } else if (cursor->m_index == 4) {
        return s_ghostLabels[cursor->m_value];
    }

    return 0;
}

namespace RegularRing {

RingState::RingState(const UnkRingElement_8011464C *rings, int count) {
    for (int i = 0; i < 4; i++) {
        m_bits[i].Clear();
    }

    for (int i = 0; i < count; i++) {
        if (rings[i].m_flags.Test(1) && !rings[i].m_flags.Test(2)) {
            m_bits[i / 32].Set(i % 32);
        }
    }
}

void RingState::Apply(UnkRingElement_8011464C *rings, int count, bool ghost) {
    for (int i = 0; i < count; i++) {
        if (m_bits[i / 32].Test(i % 32)) {
            rings[i].m_flags.Set(1);
        } else if (ghost) {
            rings[i].m_flags.Set(2);
        } else {
            rings[i].m_flags.Reset(1);
        }
    }
}

} // namespace RegularRing

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 3, 0.0f, 0.0f, 0.0f, fn_801122A8, 0},
    {1, "num", 1, 1, 0x80, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "param0", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_801122A8, 0},
    {4, "param1", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_801122A8, 0},
    {1, "ghost", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_801122A8, 0},
    {0},
};

GadgetTypeInfo lbl_80526BE4 = {
    "RING", fn_801123A8, fn_801123A4, RegularRing::Ring::Create, 0, 0, 0x10, 1, 0, 10, s_paramInfo,
};
