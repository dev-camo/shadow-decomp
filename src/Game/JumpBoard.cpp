/* The JumpBoard gadget (behaviour in the class comment). File name inferred from the class.
 * Method names marked "(name inferred)" in the header are not original. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&JumpBoard::OnCollision, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/JumpBoard.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* What the hit callback needs to know about the board (placeholder name): the object that remembers
 * the characters already dealt with, the board's character info and its parameters, and, as
 * output, whether anybody was hit (JumpBoard's OnCollision does not read it). */
struct UnkHitContext_8010E598 {
    UnkHitContext_8010E598() : m_params(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const JumpBoardParams *m_params;
    /* 0x14 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_8010E598, 0x18);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {600.0f,  900.0f,  1200.0f, 1500.0f, 1800.0f, 2100.0f,
                                         2400.0f, 2700.0f, 3000.0f, 3300.0f, 3600.0f, 3900.0f,
                                         4200.0f, 4500.0f, 4800.0f, 5100.0f};

/* Scales of the direction passed to Player::ActingAccelerateCommand for the levels 0 to 15,
 * interpolated like the speeds (name inferred). */
static const float s_accelerationScales[16] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                               1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that place it above the board and launch
 * it. */
static void fn_8010F028(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8010E598 *context = static_cast<UnkHitContext_8010E598 *>(arg);
    boost::shared_ptr<GadgetCharInfo> boardInfo = context->m_charInfo.lock();

    if (boardInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new ClrVelocityCommand(4));

    Fn8040Vector3 position = boardInfo->m_position;
    position.y += 5.0f;
    hit->fn_801DC980(new SetPositionCommand(position, 4));

    int index = (int)context->m_params->level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        /* The step to the next entry is formed in a variable of its own in two statements
         * (spelling inferred): the original keeps the entry and the step in one register while
         * the index is converted, and reuses one stack slot for its conversions, which the step
         * written inside the last expression does not give. */
        float step = s_launchSpeeds[index + 1];
        step -= speed;
        speed += step * (context->m_params->level - index);
    }

    Fn8040Matrix34 matrix;
    Fn8040Vector3 direction;
    Fn8040Vector3 acceleration;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, boardInfo->m_rotation.y);
    fn_8040B884(&matrix, boardInfo->m_rotation.x);
    fn_8040B80C(&matrix, boardInfo->m_rotation.z);
    fn_8040B884(&matrix, context->m_params->angle * (lbl_805FAD38 / 180.0f));

    direction.Set(0.0f, 0.0f, -1.0f);
    direction = matrix * direction;
    fn_8040CE38(&direction, speed);

    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
    hit->fn_801DC980(new Player::LaunchCommand(0, 4));

    float scale = s_accelerationScales[index];

    if (index < 15) {
        float step = s_accelerationScales[index + 1];
        step -= scale;
        scale += step * (context->m_params->level - index);
    }

    acceleration.Set(0.0f, 0.0f, -1.0f);
    acceleration = matrix * acceleration;
    fn_8040CE38(&acceleration, scale);

    hit->fn_801DC980(
        new Player::ActingAccelerateCommand(&acceleration, context->m_params->noControlTime));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 1);

    fn_801D560C(fn_80053164(), 0xE010, 0, 0);
    context->m_hit = true;
}

namespace JumpBoard {

const char *JumpBoard::s_name = "JumpBoard";

JumpBoard::JumpBoard(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CD90(0xA, 4);
    m_model = fn_8015F9D8(0, 0xD8);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&JumpBoard::OnCollision, this, _1));
    ReadOwner(m_owner);
    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

JumpBoard::~JumpBoard() {
    fn_8016952C(m_handle);
}

void JumpBoard::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void JumpBoard::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line along the launch direction, starting at the point 5 up from the
         * board's origin in the board's frame turned by the angle (behaviour inferred). */
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 start(0.0f, 5.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 5.0f, -1000000.0f);

        fn_8040B884(&matrix, m_params.angle * (lbl_805FAD38 / 180.0f));
        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void JumpBoard::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void JumpBoard::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void JumpBoard::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8010E598 context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    fn_8015B004(&args, fn_8010F028, &context);
}

} // namespace JumpBoard

/* Lists the board's model resource, the number the constructor loads the model from. */
extern "C" void fn_8010E570(void) {
    fn_8006B50C(0xD8, 1);
}

extern "C" void fn_8010E56C(void) {}

void JumpBoard::JumpBoard::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new JumpBoard(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "angle", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80526784 = {
    "JUMP_BOARD", fn_8010E570, fn_8010E56C, JumpBoard::JumpBoard::Create, 0, 0, 4, 1, 0,
    10,           s_paramInfo};
