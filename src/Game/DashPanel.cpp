/* The DashPanel gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&DashPanel::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/DashPanel.h"
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

/* What the hit callback needs to know about the panel (placeholder name): the object that
 * remembers the characters already dealt with, the panel's character info and its parameters.
 * The callback sets m_hit when it launches a character; nothing in this file reads it. */
struct UnkHitContext_800FF074 {
    UnkHitContext_800FF074() : m_params(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const DashPanel::DashPanelParams *m_params;
    /* 0x14 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_800FF074, 0x18);

/* Launch speeds for the levels 0 to 15; a level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {300.0f,  600.0f,  900.0f,  1200.0f, 1500.0f, 1800.0f,
                                         2100.0f, 2400.0f, 2700.0f, 3000.0f, 3300.0f, 3600.0f,
                                         3900.0f, 4200.0f, 4500.0f, 4800.0f};

/* Factors of the push for the levels 0 to 15, interpolated in the same way (name inferred). */
static const float s_pushFactors[16] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

/* Matching workaround: the constructor is defined here, out of line, as a strong global. The
 * original has a single copy, here after the hit callback, which two other units also call (at
 * 0x80121500 and 0x80292FB8, USA addresses). That fits the out-of-line copy of an inline
 * constructor that every constructing unit emits and the linker keeps from the first unit in link
 * order. Several other command constructors also sit shortly after a function that calls them
 * (SetAngleCommand's at 0x800B81F8 follows its caller fn_800B8000). That the command and
 * base constructors were inline in the original headers is inferred, not established; if they
 * were, the original copy is weak and this definition's binding differs, so the unit cannot be
 * promoted until the headers model them. The project's headers declare the base constructors out
 * of line, so an inline definition here is expanded into the callback and leaves no copy, and this
 * definition is emitted after the converting weak_ptr<CharInfo> constructor instead of before it.
 * Revisit when the command headers model those constructors. */
Player::CancelBehaviorCommand::CancelBehaviorCommand(int arg1) : PlayerCommand(0x101, arg1) {}

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that place it on the panel and launch it
 * along the panel's direction. arg is the hit context that OnCollision passes. */
static void fn_800FFB70(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_800FF074 *context = static_cast<UnkHitContext_800FF074 *>(arg);
    boost::shared_ptr<GadgetCharInfo> panelInfo = context->m_charInfo.lock();

    if (panelInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new Player::CancelBehaviorCommand(4));
    hit->fn_801DC980(new ClrVelocityCommand(4));

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, panelInfo->m_rotation.y);
    fn_8040B884(&matrix, panelInfo->m_rotation.x);
    fn_8040B80C(&matrix, panelInfo->m_rotation.z);

    /* 5 units above the panel along its up direction. */
    Fn8040Vector3 position = panelInfo->m_position;
    const Fn8040Vector3 &offset = matrix * Fn8040Vector3(0.0f, 5.0f, 0.0f);
    fn_8040CFF8(&position, &offset);
    hit->fn_801DC980(new SetPositionCommand(&position, 4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        speed += (s_launchSpeeds[index + 1] - speed) * (level - index);
    }

    Fn8040Vector3 front = matrix * Fn8040Vector3(0.0f, 0.0f, -1.0f);
    Fn8040Vector3 top = matrix * Fn8040Vector3(0.0f, 1.0f, 0.0f);
    UnkRotation3 angles = fn_8040F5F4(&front, &top, 2);
    hit->fn_801DC980(new SetAngleCommand(angles, 2, 4));

    /* The launch direction points 3 degrees down from the panel's forward direction. */
    Fn8040Vector3 velocity = Fn8040Vector3(0.0f, -0.052335955f, -0.9986295f) * speed;
    velocity = matrix * velocity;
    hit->fn_801DC980(new SetVelocityCommand(&velocity, 4));

    float factor = s_pushFactors[index];

    if (index < 15) {
        factor += (s_pushFactors[index + 1] - factor) * (context->m_params->level - index);
    }

    Fn8040Vector3 push(0.0f, 0.0f, -1.0f);
    push = matrix * push;
    fn_8040CE38(&push, factor);
    hit->fn_801DC980(new Player::ActingAccelerateCommand(&push, context->m_params->noControlTime));
    hit->fn_801DC980(new Player::PressGroundCommand(4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 1);

    fn_801D560C(fn_80053164(), 0xE010, 0, 0);
    context->m_hit = true;
}

namespace DashPanel {

const char *DashPanel::s_name = "DashPanel";

DashPanel::DashPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(6, 4);
    ReadOwner(owner);
    m_model = fn_8015F9D8(0, 0xD0);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&DashPanel::OnCollision, this, _1));
    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

DashPanel::~DashPanel() {
    fn_8016952C(m_handle);
}

void DashPanel::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void DashPanel::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from the panel along its forward direction (behaviour inferred). */
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 0.0f, -1000000.0f);

        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void DashPanel::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void DashPanel::UpdateModelMatrix() {
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

void DashPanel::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_800FF074 context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    fn_8015B004(&args, fn_800FFB70, &context);
}

} // namespace DashPanel

extern "C" void fn_800FF04C(void) {
    fn_8006B50C(0xD0, 1);
}

extern "C" void fn_800FF048(void) {}

void DashPanel::DashPanel::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new DashPanel(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525A14 = {
    "DASHPANEL", fn_800FF04C, fn_800FF048, DashPanel::DashPanel::Create, 0, 0, 3, 1,
    0,           10,          s_paramInfo,
};
