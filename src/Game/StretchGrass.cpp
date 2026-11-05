/* The NOBI_KUSA gadget (class described in Game/StretchGrass.h). File name inferred from the
 * class's namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The draw layer's callback and the character info's command handler are boost::function objects
 * built with boost::bind(&SetStretchGrass::method, this, _1); their managers, invokers and copy
 * functions come from the boost headers. */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's and
 * CharaColliBase's derived classes, which these two headers emit through their stand-in inline
 * functions (see TNumEdit.h). TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their
 * names appear in the unit's .rodata (alphabetical order would reverse them). Revisit when the
 * original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/StretchGrass.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_80037A80.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80109AF8.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpdmorph.h"
#include "rw/rpworld.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::Collision::CollisionResultList;

/* The name the draw layer is created with. */
static const char *s_layerName = "SetStretchGrass";

const char *StretchGrass::SetStretchGrass::s_name = "SetStretchGrass";

/* Launch speeds for the levels 0 to 7; a level in between is interpolated (name inferred). */
static const float s_launchSpeeds[8] = {2000.0f, 2200.0f, 2400.0f, 2600.0f,
                                        2800.0f, 3000.0f, 3200.0f, 3400.0f};

/* Half a turn, added to the y angle of the launch angles. */
static const float s_halfTurn = DegreesToRadians(180.0f);

/* The state of the grass that last took a character (inferred): set by the hit callback, cleared
 * by the descriptor's first hook. Only that grass is drawn with its morph clump. */
static StretchGrass::StretchGrassState *s_activeState;

/* What the hit callback needs to know about the grass (placeholder name). */
struct UnkHitContext_8014C108 {
    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ const StretchGrass::StretchGrassParams *m_params;
    /* 0x0C */ StretchGrass::StretchGrassState *m_state;
    /* 0x10 */ const Fn8040Vector3 *m_launchPosition;
    /* 0x14 */ const UnkRotation3 *m_launchAngles;
};
CHECK_SIZEOF(UnkHitContext_8014C108, 0x18);

/* The atomic callback of OnDraw (placeholder name): sets every delta morph target weight of the
 * atomic to the value that data points to, and the radius of its bounding sphere to 500. */
static RpAtomic *fn_8014D7F8(RpAtomic *atomic, void *data) {
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
        sphere->radius = 500.0f;
    }
    return atomic;
}

/* The callback that fn_8015AE44 calls for each character info in the collision results. A waiting
 * grass starts to stretch and takes the character's control; a grass that springs back launches
 * the character once, from its tip, at the speed of its level and along its direction turned by
 * "angle" about the x axis. */
static void fn_8014D324(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8014C108 *context = static_cast<UnkHitContext_8014C108 *>(arg);

    if (context == 0) {
        return;
    }

    boost::shared_ptr<GadgetCharInfo> grassInfo = context->m_charInfo.lock();

    if (grassInfo.get() == 0) {
        return;
    }

    if (context->m_state->m_mode == StretchGrass::StretchGrassState::MODE_WAITING) {
        context->m_state->m_mode = StretchGrass::StretchGrassState::MODE_STRETCHING;
        context->m_state->m_speed = 0.063f;
        context->m_state->field_0x28 = 0;
        s_activeState = context->m_state;
        hit->fn_801DC980(new Player::StartExternalControlCommand(
            boost::weak_ptr<CharInfo>(context->m_charInfo), context->m_launchPosition,
            context->m_launchAngles, 1, 4));
        hit->fn_801DC980(new Player::SetMotionCommand(3, 0.0f, 4));
    } else if (context->m_state->m_mode == StretchGrass::StretchGrassState::MODE_STRETCHING ||
               context->m_state->m_mode == StretchGrass::StretchGrassState::MODE_HOLDING) {
        /* Stretching or holding: the grass already carries a character. The original tests these
         * two modes before MODE_SPRINGING_BACK although nothing happens for them. */
    } else if (context->m_state->m_mode == StretchGrass::StretchGrassState::MODE_SPRINGING_BACK &&
               !context->m_state->field_0x28) {
        hit->fn_801DC980(new Player::IgnoreInputCommand(0, context->m_params->sec));
        hit->fn_801DC980(new ClrVelocityCommand(4));
        hit->fn_801DC980(new SetPositionCommand(&grassInfo->m_position, 4));

        float level = context->m_params->level;
        int index = (int)level;
        float speed = s_launchSpeeds[index];

        if (index < 7) {
            speed += (s_launchSpeeds[index + 1] - speed) * (level - index);
        }

        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B848(&matrix, grassInfo->m_rotation.y);
        fn_8040B884(&matrix, grassInfo->m_rotation.x);
        fn_8040B80C(&matrix, grassInfo->m_rotation.z);
        fn_8040B884(&matrix, DegreesToRadians(context->m_params->angle));

        Fn8040Vector3 direction(0.0f, 0.0f, 1.0f);

        direction = matrix * direction;
        fn_8040CE38(&direction, speed);

        hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
        hit->fn_801DC980(new Player::LaunchCommand(0, 4));

        fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 5);

        context->m_state->field_0x28 = 1;
    }
}

namespace StretchGrass {

SetStretchGrass::SetStretchGrass(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_clump(0), m_clumpOne(0), m_morphData(0), m_drawStretch(0.0f),
      m_drawState(0), m_launchAngles(0.0f, 0.0f, 0.0f) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x28, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SetStretchGrass::OnCommand, this, _1));

    m_probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x40000, 10, 0,
                          &lbl_805E07A0, 10.0f);

    m_layer = fn_8004DFB0(fn_80046178()->field_0x8, s_layerName);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&SetStretchGrass::OnDraw, this, _1));

    m_clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "JG0404STRETCH.DFF"));
    m_clumpOne = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "JG0404STRETCHONE.DFF"));
    m_morphData = fn_80012700(fn_800122D4(), 1, "JG0404STRETCH.DMA");
    m_drawState = &m_state;
    m_drawStretch = 0.0f;
    fn_80038B18(m_clump);
    fn_80038B18(m_clumpOne);
    fn_80037A80(m_clump, m_morphData);

    m_target = fn_801E72DC(0x400);

    ReadOwner(m_owner);
    UpdateTarget();
}

SetStretchGrass::~SetStretchGrass() {
    fn_801E7108(m_target);
    if (m_probe.get() != 0) {
        fn_800C78B0(fn_80021B08(), &m_probe);
    }
}

void SetStretchGrass::UpdateBody(float dt) {
    bool moved = false;

    switch (m_state.m_mode) {
    case StretchGrassState::MODE_NEW:
        m_state.m_mode = StretchGrassState::MODE_WAITING;
        break;
    case StretchGrassState::MODE_STRETCHING:
        m_state.m_speed += 0.005f;
        m_state.m_stretch += m_state.m_speed;
        if (m_state.m_stretch > m_params.length) {
            m_state.m_stretch = m_params.length;
            m_state.m_mode = StretchGrassState::MODE_HOLDING;
            m_state.m_timer = m_params.charge;
        } else {
            fn_801D53C4(fn_80053164(), 0x7531, 0, this, 2, 0, 2);
        }
        moved = true;
        break;
    case StretchGrassState::MODE_SPRINGING_BACK:
        m_state.m_stretch -= 0.42f;
        if (m_state.m_stretch < -0.36f) {
            m_state.m_stretch = -0.36f;
            m_state.m_mode = StretchGrassState::MODE_SWINGING;
        }
        moved = true;
        break;
    case StretchGrassState::MODE_SWINGING:
        m_state.m_stretch *= 0.93f;
        m_state.m_stretch *= -1.0f;
        if (fn_8000A980(m_state.m_stretch) < 0.001f) {
            m_state.m_stretch = 0.0f;
            m_state.m_mode = StretchGrassState::MODE_WAITING;
        }
        moved = true;
        break;
    case StretchGrassState::MODE_HOLDING:
        m_state.m_timer -= dt;
        /* The original keeps both arms, each with its own sound call and mode store. */
        if (m_state.field_0x28) {
            fn_801D560C(fn_80053164(), 0x7532, 0, 0);
            m_state.m_mode = StretchGrassState::MODE_SPRINGING_BACK;
        } else if (m_state.m_timer <= 0.0f) {
            fn_801D560C(fn_80053164(), 0x7532, 0, 0);
            m_state.m_mode = StretchGrassState::MODE_SPRINGING_BACK;
        }
        moved = true;
        break;
    }

    if (moved && m_layer.get() != 0) {
        m_drawStretch = m_state.m_stretch;
    }

    if (m_charInfo.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_state.m_position);
        fn_8040B848(&matrix, m_state.m_rotation.y);
        fn_8040B884(&matrix, m_state.m_rotation.x);
        fn_8040B80C(&matrix, m_state.m_rotation.z);

        Fn8040Vector3 tip(0.0f, 0.0f, -50.0f * m_state.m_stretch);

        m_charInfo->m_position = matrix * tip;

        Fn8040Vector3 launch(0.0f, 0.0f, -50.0f * m_state.m_stretch - 5.0f);

        m_launchPosition = matrix * launch;

        m_launchAngles = m_state.m_rotation;
        m_launchAngles.y += s_halfTurn;
    }

    if (m_probe.get() != 0) {
        CollisionResultList *hits = m_probe->GetHits();

        if (hits != 0 && hits->field_0x4 > 0) {
            ColliDetectArgs args;

            /* The constructor has already cleared both members; the original stores them again. */
            args.m_hits = hits;
            args.field_0x4 = 0;

            UnkHitContext_8014C108 context;

            context.m_charInfo = m_charInfo;
            context.m_params = &m_params;
            context.m_state = &m_state;
            context.m_launchPosition = &m_launchPosition;
            context.m_launchAngles = &m_launchAngles;
            fn_8015AE44(&args, fn_8014D324, &context);
        }
    }
}

void SetStretchGrass::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateTarget();

    /* Queues a debug line from the grass along its launch direction (behaviour inferred). */
    Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
    Fn8040Vector3 end(0.0f, 0.0f, 1000000.0f);
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_state.m_position);
    fn_8040B848(&matrix, m_state.m_rotation.y);
    fn_8040B884(&matrix, m_state.m_rotation.x);
    fn_8040B80C(&matrix, m_state.m_rotation.z);
    fn_8040B884(&matrix, DegreesToRadians(m_params.angle));

    start = matrix * start;

    end = matrix * end;
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

    if (m_probe.get() != 0) {
        m_probe->UnkSlot26();
    }
}

void SetStretchGrass::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_state.m_position);
    fn_800C9ED4(owner, &m_state.m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    if (m_charInfo.get() != 0) {
        m_charInfo->m_position = m_state.m_position;
        m_charInfo->m_rotation = m_state.m_rotation;
    }
}

void SetStretchGrass::UpdateTarget() {
    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_charInfo->m_position);
    }
}

void SetStretchGrass::OnDraw(RednerDrawArgs &args) {
    RwSphere sphere;

    sphere.radius = 1000.0f;
    sphere.center.x = m_drawState->m_position.x;
    sphere.center.y = m_drawState->m_position.y;
    sphere.center.z = m_drawState->m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        if (m_drawState == s_activeState && m_drawStretch != 0.0f) {
            float weight = m_drawStretch;

            fn_80456C54(m_clump, fn_8014D7F8, &weight);

            RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);

            fn_80109AF8(frame, rwCOMBINEREPLACE, m_drawState->m_rotation.z);
            fn_800E9C64(frame, rwCOMBINEPOSTCONCAT, m_drawState->m_rotation.x);
            fn_800E9CEC(frame, rwCOMBINEPOSTCONCAT, m_drawState->m_rotation.y);
            fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
            fn_80456BC4(m_clump);
        } else {
            RwFrame *frame = static_cast<RwFrame *>(m_clumpOne->object.parent);

            fn_80109AF8(frame, rwCOMBINEREPLACE, m_drawState->m_rotation.z);
            fn_800E9C64(frame, rwCOMBINEPOSTCONCAT, m_drawState->m_rotation.x);
            fn_800E9CEC(frame, rwCOMBINEPOSTCONCAT, m_drawState->m_rotation.y);
            fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
            fn_80456BC4(m_clumpOne);
        }
    }
}

void SetStretchGrass::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0xC:
        m_state.field_0x28 = 1;
        break;
    }
}

} // namespace StretchGrass

/* The descriptor's first hook: lists the three resource files for loading and clears
 * s_activeState. */
extern "C" void fn_8014BD6C(void) {
    fn_8006B474("JG0404STRETCH.DFF", 0);
    fn_8006B474("JG0404STRETCHONE.DFF", 0);
    fn_8006B474("JG0404STRETCH.DMA", 0);
    s_activeState = 0;
}

extern "C" void fn_8014BD68(void) {}

void StretchGrass::SetStretchGrass::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetStretchGrass(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 7.0f, fn_800CBC20, 0},
    {4, "angle", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "sec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "length", 0, 0, 0, 4.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "charge", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052A2F0 = {
    "NOBI_KUSA", fn_8014BD6C, fn_8014BD68, StretchGrass::SetStretchGrass::Create, 0, 0, 0x1137, 1,
    0,           10,          s_paramInfo,
};
