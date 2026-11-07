/* The Catapult gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler is a boost::function built with
 * boost::bind(&Catapult::OnCommand, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/Catapult.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
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
#include "layout_check.h"

/* What the launch needs to know about the catapult (placeholder name): its character info and
 * its parameters. */
struct UnkLaunchContext_80142260 {
    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ const Catapult::CatapultParams *m_params;
};
CHECK_SIZEOF(UnkLaunchContext_80142260, 0xC);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {600.0f,  900.0f,  1200.0f, 1500.0f, 1800.0f, 2100.0f,
                                         2400.0f, 2700.0f, 3000.0f, 3300.0f, 3600.0f, 3900.0f,
                                         4200.0f, 4500.0f, 4800.0f, 5100.0f};

/* Launches user (behaviour inferred): sends it an IgnoreInputCommand with the parameter's time
 * and a ClrVelocityCommand, then a SetVelocityCommand along the catapult's y axis, turned by the
 * character info's rotations and the parameters X and Z, with the speed of the parameter's level;
 * calls fn_80169BB0 with a weak pointer to user and kind 5 (the callee ignores the pointer) and
 * sends user a LaunchCommand. Does nothing once
 * the catapult's character info is gone. */
static void fn_80142C88(boost::shared_ptr<CharInfo> user, UnkLaunchContext_80142260 *context) {
    boost::shared_ptr<GadgetCharInfo> catapultInfo = context->m_charInfo.lock();

    if (catapultInfo.get() == 0) {
        return;
    }

    user->fn_801DC980(new Player::IgnoreInputCommand(0, context->m_params->noControlTime));
    user->fn_801DC980(new ClrVelocityCommand(4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        speed += (s_launchSpeeds[index + 1] - speed) * (level - index);
    }

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, catapultInfo->m_rotation.y);
    fn_8040B884(&matrix, catapultInfo->m_rotation.x);
    fn_8040B80C(&matrix, catapultInfo->m_rotation.z);
    fn_8040B884(&matrix, DegreesToRadians(context->m_params->x));
    fn_8040B80C(&matrix, DegreesToRadians(context->m_params->z));

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

    direction = matrix * direction;
    fn_8040CE38(&direction, speed);

    user->fn_801DC980(new SetVelocityCommand(&direction, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(user), 5);

    user->fn_801DC980(new Player::LaunchCommand(0, 4));
}

namespace Catapult {

const char *Catapult::s_name = "Catapult";

Catapult::Catapult(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x36, 4);
    ReadOwner(owner);

    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Catapult::OnCommand, this, _1));

    m_modelIndex = m_params.model;
    m_model = fn_8015F800(1, "CATAPULT.BIN");
    m_model->fn_80162A68(m_modelIndex, 0);
    m_model->fn_80160C28(m_modelIndex);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo), m_modelIndex);

    UpdateModelMatrix();
}

Catapult::~Catapult() {}

void Catapult::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        m_timer = 1.0f;
        break;

    case 2:
        if (m_flags.Test(0)) {
            m_model->fn_80160A84(m_modelIndex, 2);
            m_model->fn_80161308(1.0f);

            if (boost::shared_ptr<CharInfo> user = m_user.lock()) {
                UnkLaunchContext_80142260 context;

                context.m_charInfo = m_charInfo;
                context.m_params = &m_params;
                fn_80142C88(user, &context);
            }

            m_timer = 1.0f;
            m_state = 3;
            m_flags.Reset(0);
        }
        break;

    case 3:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = 0;
        }
        break;
    }

    UpdateModelMatrix();
}

void Catapult::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        if (m_params.model != m_modelIndex) {
            int count = fn_8015FF44(m_model.get());

            m_modelIndex = m_params.model;
            if (m_modelIndex < 0) {
                m_modelIndex = 0;
            }
            if (m_modelIndex > count - 1) {
                m_modelIndex = count - 1;
            }

            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_model->fn_80162A68(m_modelIndex, 0);
            m_model->fn_80160C28(m_modelIndex);
            m_model->fn_8015FF60();
            m_model->fn_80161BD8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo), m_modelIndex);

            m_params.model = m_modelIndex;
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }

        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from the origin to (0, 1000000, 0) in the launch frame: the
         * character info's position and rotations, then the parameters X and Z (behaviour
         * inferred). */
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 1000000.0f, 0.0f);

        fn_8040B884(&matrix, DegreesToRadians(m_params.x));
        fn_8040B80C(&matrix, DegreesToRadians(m_params.z));

        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void Catapult::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
}

void Catapult::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void Catapult::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_state == 1 || m_state == 3) {
            m_flags.Set(1);
            m_state = 2;
            m_model->fn_80160A84(m_modelIndex, 1);
            m_charInfo->SetField0x50(0x37);
            fn_801D54F4(fn_80053164(), 0x7548, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 12:
        if (m_state == 2 && m_flags.Test(1)) {
            m_timer = 1.5f;
            m_state = 3;
            m_model->fn_801612B8();
            m_model->fn_80160A84(m_modelIndex, 2);
            m_flags.Reset(1);
            /* back to the value that the constructor's fn_8015CD90 call gave the character info */
            m_charInfo->SetField0x50(0x36);
        }
        break;

    case 13:
        if (m_state == 2 && m_flags.Test(1)) {
            m_user = static_cast<UseCommand &>(command).m_user;
            m_flags.Set(0);
            m_flags.Reset(1);
            m_charInfo->SetField0x50(0x36);
            fn_801D560C(fn_80053164(), 0x7549, 0, 0);
        }
        break;
    }
}

} // namespace Catapult

extern "C" void fn_80141D94(void) {
    fn_8006B474("CATAPULT.BIN", 1);
}

extern "C" void fn_80141D90(void) {}

void Catapult::Catapult::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Catapult(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "X", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80529744 = {
    "CATAPULT", fn_80141D94, fn_80141D90, Catapult::Catapult::Create, 0, 0, 0x1138, 1,
    0,          10,          s_paramInfo,
};
