/* Spring::Spring (class name from RTTI; the gadget's descriptor is "SPRING"): a spring that, when
 * a character touches it, takes the character's control for a time, turns it and launches it
 * along the spring's turned y axis with a speed that the parameter "level" selects (behaviour
 * inferred). Command ids 0x1A and 0x1B sent to its character info turn the launching off and on
 * (inferred).
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's
 * and CharaColliBase's derived classes, which these two headers emit through their stand-in
 * inline functions (see TNumEdit.h). TNumEdit.h comes before CharaColliBaseCharInfo.h, in the
 * order their names appear in the unit's .rodata. Revisit when the original inline functions or
 * their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Spring.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* What the hit callback needs to know about the spring (placeholder name): the handle that
 * remembers the characters already dealt with, the spring's character info, its parameters and
 * its position, and, as output, whether anybody was hit. */
struct UnkHitContext_801202A0 {
    UnkHitContext_801202A0() : m_params(0), m_position(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const Spring::SpringParams *m_params;
    /* 0x14 */ const Fn8040Vector3 *m_position;
    /* 0x18 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_801202A0, 0x1C);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {150.0f,  300.0f,  450.0f,  600.0f,  750.0f,  900.0f,
                                         1050.0f, 1200.0f, 1350.0f, 1500.0f, 1650.0f, 1800.0f,
                                         1950.0f, 2100.0f, 2250.0f, 2400.0f};

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that take its control, turn it to the
 * spring and launch it. */
static void fn_801213E8(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_801202A0 *context = static_cast<UnkHitContext_801202A0 *>(arg);

    boost::shared_ptr<GadgetCharInfo> springInfo = context->m_charInfo.lock();

    if (springInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new SpringCommand());
    hit->fn_801DC980(new Player::IgnoreInputCommand(1, context->m_params->noControlTime));
    hit->fn_801DC980(new Player::CancelBehaviorCommand(4));
    hit->fn_801DC980(new ClrVelocityCommand(4));
    hit->fn_801DC980(new SetPositionCommand(context->m_position, 4));

    /* The angles the character is turned to: those of the forward (0, 0, -1) and up (0, 1, 0)
     * directions in the spring's frame turned a further quarter turn about its x axis (behaviour
     * inferred). */
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, springInfo->m_rotation.y);
    fn_8040B884(&matrix, springInfo->m_rotation.x);
    fn_8040B80C(&matrix, springInfo->m_rotation.z);
    fn_8040B884(&matrix, DegreesToRadians(90.0f));

    Fn8040Vector3 upTurned = matrix * Fn8040Vector3(0.0f, 1.0f, 0.0f);

    Fn8040Vector3 forwardTurned = matrix * Fn8040Vector3(0.0f, 0.0f, -1.0f);

    hit->fn_801DC980(new SetAngleCommand(fn_8040F364(forwardTurned, upTurned), 2, 4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        /* The step to the next entry is formed in a variable of its own in two statements
         * (spelling inferred): the original keeps the entry and the step in one register while
         * the index is converted, and reuses one stack slot for its conversions, which the step
         * written inside the last expression does not give. */
        float step = s_launchSpeeds[index + 1];
        step -= speed;
        speed += step * (level - index);
    }

    /* The launch direction: the spring's y axis turned by its x and z rotations. */
    Fn8040Matrix34 launchMatrix;

    fn_8040C148(&launchMatrix, &lbl_805E0720);
    fn_8040B884(&launchMatrix, springInfo->m_rotation.x);
    fn_8040B80C(&launchMatrix, springInfo->m_rotation.z);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);
    direction = launchMatrix * direction;
    fn_8040CE38(&direction, speed);

    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
    hit->fn_801DC980(new Player::LaunchCommand(0, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 5);

    fn_801D560C(fn_80053164(), 0xE00F, 0, 0);
    context->m_hit = true;
}

namespace Spring {

const char *Spring::s_name = "Spring";

Spring::Spring(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_charInfo = fn_8015CB88(5, 4, "Spring");
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Spring::OnCommand, this, _1));

    m_model = fn_8015F800(0, "SPRING.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col0", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback = ColliDetectCallback(boost::bind(&Spring::OnCollision, this, _1));
    m_model->fn_80160BA0("motion0");
    m_model->fn_8015FF60();

    fn_8040CE60(&m_position);
    m_object = fn_801E72DC(0x400);

    ReadOwner(m_owner);
    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

Spring::~Spring() {
    fn_8016952C(m_handle);
    fn_801E7108(m_object);
}

void Spring::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    UpdateModelMatrix();
}

void Spring::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from (0, 10, 0) to (0, 1000000, 0) in the spring's frame: its
         * position, turned by its x and z rotations (behaviour inferred). */
        Fn8040Vector3 start(0.0f, 10.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 1000000.0f, 0.0f);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void Spring::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_flags.Clear();
    m_flags.Set(0);
}

void Spring::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);

        Fn8040Vector3 top(0.0f, 10.0f, 0.0f);

        m_position = matrix * top;

        if (m_object.get() != 0) {
            fn_800CCE38(m_object.get(), m_position);
        }
    }
}

void Spring::OnCollision(ColliDetectArgs &args) {
    if (m_flags.Test(0)) {
        UnkHitContext_801202A0 context;

        context.m_handle = m_handle;
        context.m_charInfo = m_charInfo;
        context.m_params = &m_params;
        context.m_position = &m_position;
        fn_8015B004(&args, fn_801213E8, &context);

        if (context.m_hit && m_model.get() != 0) {
            m_model->fn_80160B18(0);
        }
    }
}

void Spring::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x1A:
        m_flags.Reset(0);
        if (m_object->TestFlags(1)) {
            m_object->ClearFlags(1);
        }
        break;

    case 0x1B:
        m_flags.Set(0);
        if (!m_object->TestFlags(1)) {
            m_object->SetFlags(1);
        }
        break;
    }
}

} // namespace Spring

extern "C" void fn_801201C4(void) {
    fn_8006B474("SPRING.BIN", 1);
}

extern "C" void fn_801201C0(void) {}

void Spring::Spring::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Spring(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80527898 = {
    "SPRING", fn_801201C4, fn_801201C0, Spring::Spring::Create, 0, 0, 1, 1, 0, 10, s_paramInfo,
};
