/* The DashRing gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&DashRing::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/DashRing.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* What the hit callback needs to know about the ring (placeholder name): the object that
 * remembers the characters already dealt with, the ring's character info and its parameters.
 * The callback sets m_hit when it launches a character; nothing in this file reads it. */
struct UnkHitContext_8010021C {
    UnkHitContext_8010021C() : m_params(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const DashRing::DashRingParams *m_params;
    /* 0x14 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_8010021C, 0x18);

/* Launch speeds for the levels 0 to 15; a level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {400.0f,  600.0f,  800.0f,  1000.0f, 1200.0f, 1400.0f,
                                         1600.0f, 1800.0f, 2000.0f, 2200.0f, 2400.0f, 2600.0f,
                                         2800.0f, 3000.0f, 3200.0f, 3400.0f};

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that take its control, place it at the ring
 * and launch it along the ring's direction. arg is the hit context that OnCollision passes. */
static void fn_80100CBC(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8010021C *context = static_cast<UnkHitContext_8010021C *>(arg);
    boost::shared_ptr<GadgetCharInfo> ringInfo = context->m_charInfo.lock();

    if (ringInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new Player::IgnoreInputCommand(0, context->m_params->noControlTime));
    hit->fn_801DC980(new ClrVelocityCommand(4));
    hit->fn_801DC980(new SetPositionCommand(&ringInfo->m_position, 4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        speed += (s_launchSpeeds[index + 1] - speed) * (level - index);
    }

    /* Matching workaround: the vectors are declared together in this order because the order of
     * declaration places their stack slots; declaring each at its first use moves them and adds 10
     * differing words. velocityTemporary receives the transformed vector before it is copied,
     * possibly standing for a by-value temporary of the original (inferred). Revisit when the
     * vector functions are modelled with by-value results. */
    Fn8040Matrix34 matrix;
    Fn8040Vector3 velocity;
    Fn8040Vector3 up;
    Fn8040Vector3 angles;
    Fn8040Vector3 velocityTemporary;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, ringInfo->m_rotation.y);
    fn_8040B884(&matrix, ringInfo->m_rotation.x);
    fn_8040B80C(&matrix, ringInfo->m_rotation.z);

    velocity.Set(0.0f, 0.0f, -1.0f);
    fn_8040B67C(&velocityTemporary, &matrix, &velocity);
    fn_800091D0(&velocity, &velocityTemporary);
    fn_8040CE38(&velocity, speed);

    up.Set(0.0f, 1.0f, 0.0f);
    angles = fn_8040F5F4(&velocity, &up, 2);

    hit->fn_801DC980(new SetAngleCommand(angles, 2, 4));
    hit->fn_801DC980(new SetVelocityCommand(&velocity, 4));
    hit->fn_801DC980(new Player::LaunchCommand(0, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 1);

    fn_801D560C(fn_80053164(), 0xE012, 0, 0);
    context->m_hit = true;
}

namespace DashRing {

const char *DashRing::s_name = "DashRing";

DashRing::DashRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    /* The parameters are read here, before the character info and the model exist, and again by
     * ReadOwner below together with the position and rotation (the reason is not known). */
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CD90(0xB, 4);
    m_model = fn_8015F9D8(0, 0xD1);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&DashRing::OnCollision, this, _1));
    ReadOwner(m_owner);
    UpdateModelMatrix();
    m_sound = fn_801D3DC8(0xE011, &m_charInfo->m_position, this);
    m_handle = fn_801695B8();
}

DashRing::~DashRing() {
    fn_8016952C(m_handle);
    m_sound.fn_801D455C();
}

void DashRing::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void DashRing::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from the ring along its forward direction (behaviour inferred). */
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 0.0f, -1000000.0f);
        Fn8040Vector3 startTemporary;
        Fn8040Vector3 endTemporary;

        fn_8040B67C(&startTemporary, &matrix, &start);
        fn_800091D0(&start, &startTemporary);
        fn_8040B67C(&endTemporary, &matrix, &end);
        fn_800091D0(&end, &endTemporary);
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void DashRing::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void DashRing::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void DashRing::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8010021C context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    fn_8015B004(&args, fn_80100CBC, &context);
}

} // namespace DashRing

extern "C" void fn_801001F4(void) {
    fn_8006B50C(0xD1, 1);
}

extern "C" void fn_801001F0(void) {}

void DashRing::DashRing::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new DashRing(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525AFC = {
    "DASHRING", fn_801001F4, fn_801001F0, DashRing::DashRing::Create, 0, 0, 6, 1,
    0,          10,          s_paramInfo,
};
