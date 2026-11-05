/* The LongSpring gadget (behaviour in the class comment). File name inferred from the class.
 * Method names marked "(name inferred)" in the header are not original. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&LongSpring::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/LongSpring.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* What the hit callback needs to know about the spring (placeholder name): the object that
 * remembers the characters already dealt with, the spring's character info, its parameters and
 * its three launch points, and, as output, whether anybody was hit. */
struct UnkHitContext_8010F588 {
    UnkHitContext_8010F588() : m_params(0), m_launchPoints(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const LongSpringParams *m_params;
    /* 0x14 */ const Fn8040Vector3 *m_launchPoints;
    /* 0x18 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_8010F588, 0x1C);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {150.0f,  300.0f,  450.0f,  600.0f,  750.0f,  900.0f,
                                         1050.0f, 1200.0f, 1350.0f, 1500.0f, 1650.0f, 1800.0f,
                                         1950.0f, 2100.0f, 2250.0f, 2400.0f};

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that place it at the nearest launch point
 * and launch it. */
static void fn_80110320(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8010F588 *context = static_cast<UnkHitContext_8010F588 *>(arg);
    boost::shared_ptr<GadgetCharInfo> springInfo = context->m_charInfo.lock();

    if (springInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new SpringCommand());
    hit->fn_801DC980(new Player::IgnoreInputCommand(1, context->m_params->noControlTime));
    hit->fn_801DC980(new ClrVelocityCommand(4));

    Fn8040Vector3 direction;
    float nearestDistanceSquared = 100000000.0f;
    int nearest = 0;

    for (int i = 0; i < 3; i++) {
        Fn8040Vector3 difference;

        fn_8040CD74(&difference, &hit->m_position, &context->m_launchPoints[i]);
        float distanceSquared = fn_8040CF2C(&difference);

        if (distanceSquared < nearestDistanceSquared) {
            nearestDistanceSquared = distanceSquared;
            nearest = i;
        }
    }

    int index = (int)context->m_params->level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        speed += (s_launchSpeeds[index + 1] - speed) * (context->m_params->level - index);
    }

    hit->fn_801DC980(new SetPositionCommand(&context->m_launchPoints[nearest], 4));

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, springInfo->m_rotation.y);
    fn_8040B884(&matrix, springInfo->m_rotation.x);
    fn_8040B80C(&matrix, springInfo->m_rotation.z);

    Fn8040Vector3 directionTemporary;

    direction.Set(0.0f, 1.0f, 0.0f);
    fn_8040B67C(&directionTemporary, &matrix, &direction);
    fn_800091D0(&direction, &directionTemporary);
    fn_8040CE38(&direction, speed);

    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
    hit->fn_801DC980(new Player::LaunchCommand(1, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 5);

    fn_801D560C(fn_80053164(), 0xE00F, 0, 0);
    context->m_hit = true;
}

namespace LongSpring {

const char *LongSpring::s_name = "LongSpring";

LongSpring::LongSpring(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CB88(5, 4, "LongSpring");
    m_model = fn_8015F9D8(0, 0xD9);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&LongSpring::OnCollision, this, _1));
    m_model->fn_80160BA0("motion");
    m_model->fn_8015FF60();

    for (int i = 0; i < 3; i++) {
        fn_8040CE60(&m_launchPoints[i]);
        m_targets[i] = fn_801E72DC(0x400);
    }

    ReadOwner(m_owner);
    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

LongSpring::~LongSpring() {
    fn_8016952C(m_handle);

    for (int i = 0; i < 3; i++) {
        fn_801E7108(m_targets[i]);
    }
}

void LongSpring::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    UpdateModelMatrix();
}

void LongSpring::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line upwards from each launch point (behaviour inferred). */
        static Fn8040Vector3 s_lineStarts[3] = {Fn8040Vector3(0.0f, 10.0f, 10.0f),
                                                Fn8040Vector3(20.0f, 10.0f, 10.0f),
                                                Fn8040Vector3(-20.0f, 10.0f, 10.0f)};
        static Fn8040Vector3 s_lineEnds[3] = {Fn8040Vector3(0.0f, 1000000.0f, 10.0f),
                                              Fn8040Vector3(20.0f, 1000000.0f, 10.0f),
                                              Fn8040Vector3(-20.0f, 1000000.0f, 10.0f)};
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        for (int i = 0; i < 3; i++) {
            Fn8040Vector3 start;
            Fn8040Vector3 end;
            Fn8040Vector3 startTemporary;
            Fn8040Vector3 endTemporary;

            fn_8040B67C(&startTemporary, &matrix, &s_lineStarts[i]);
            fn_800091D0(&start, &startTemporary);
            fn_8040B67C(&endTemporary, &matrix, &s_lineEnds[i]);
            fn_800091D0(&end, &endTemporary);
            fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
        }
    }
}

void LongSpring::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void LongSpring::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);

        /* The launch points' offsets in the model's frame. */
        static Fn8040Vector3 s_launchPointOffsets[3] = {Fn8040Vector3(0.0f, 10.0f, 10.0f),
                                                        Fn8040Vector3(20.0f, 10.0f, 10.0f),
                                                        Fn8040Vector3(-20.0f, 10.0f, 10.0f)};

        for (int i = 0; i < 3; i++) {
            Fn8040Vector3 transformed;

            fn_8040B67C(&transformed, &matrix, &s_launchPointOffsets[i]);
            fn_800091D0(&m_launchPoints[i], &transformed);

            if (m_targets[i].get() != 0) {
                fn_800CCE38(m_targets[i].get(), &m_launchPoints[i]);
            }
        }
    }
}

void LongSpring::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8010F588 context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    context.m_launchPoints = m_launchPoints;
    fn_8015B004(&args, fn_80110320, &context);

    if (context.m_hit && m_model.get() != 0) {
        m_model->fn_80160B18(0);
    }
}

} // namespace LongSpring

/* Lists the spring's model resource, the number the constructor loads the model from. */
extern "C" void fn_8010F560(void) {
    fn_8006B50C(0xD9, 1);
}

extern "C" void fn_8010F55C(void) {}

void LongSpring::LongSpring::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new LongSpring(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052686C = {
    "LONGSPRING", fn_8010F560, fn_8010F55C, LongSpring::LongSpring::Create, 0, 0, 2, 1, 0,
    10,           s_paramInfo};
