/* The ROCKET gadget (classes described in Game/Rocket.h). File name inferred from the namespace.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The command handlers of both character infos and the rocket model's collision callback are
 * boost::function objects built with boost::bind(&Class::method, this, _1); their manager,
 * invoker and copy functions come from the boost headers. */
#include "Game/Rocket.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80292A64.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

namespace Rocket {

const char *RocketTask::s_name = "RocketTask";
const char *RocketBase::s_name = "RocketBase";

/* What OnCollision passes to fn_8015AE44 (placeholder name): the hit callback below stores the
 * first character info it is called with in m_hit and sets m_found. */
struct UnkHitContext_80115C28 {
    UnkHitContext_80115C28() : m_found(false) {}

    /* 0x0 */ boost::weak_ptr<CharInfo> m_hit;
    /* 0x8 */ bool m_found;
};
CHECK_SIZEOF(UnkHitContext_80115C28, 0xC);

/* The callback that fn_8015AE44 calls for each character info in the hits (placeholder name): when
 * there is a context that has not found a character yet and the character info is not null, it
 * stores the character info and sets m_found. */
static void fn_80116FEC(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80115C28 *context = static_cast<UnkHitContext_80115C28 *>(arg);

    if (context != 0 && !context->m_found && hit.get() != 0) {
        context->m_hit = hit;
        context->m_found = true;
    }
}

Rocket::Rocket(const boost::weak_ptr<CharInfo> &owner) : m_hangAngles(0.0f, 0.0f, 0.0f) {
    m_owner = owner;
    m_state = STATE_WAITING;
    m_timer = 0.0f;

    m_charInfo = fn_8015CB88(0x1E, 4, "Rocket");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Rocket::OnCommand, this, _1));

    if (boost::shared_ptr<CharInfo> base = m_owner.lock()) {
        m_charInfo->m_position = base->m_position;
        m_charInfo->m_rotation = base->m_rotation;
    }

    m_model = fn_8015F800(0, "ROCKET.BIN");
    m_model->fn_801629D8("rocket", 0);
    m_model->fn_80161F08("area", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->fn_80161F08("rocket", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback = ColliDetectCallback(boost::bind(&Rocket::OnCollision, this, _1));

    UpdateModelMatrix();
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
Rocket::~Rocket() {
    fn_8034DB60(field_0x68);
}
#pragma auto_inline reset

void Rocket::Update(float dt) {
    m_flags.Reset(FLAG_TOUCHED);

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case STATE_WAITING:
        break;

    case STATE_IGNITION:
        if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
            rider->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                            &m_hangPosition, &m_hangAngles, 4, 4));
        }

        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_model->fn_80161880();
            m_state = STATE_FLYING;

            m_target = fn_80166AF4();
            {
                UnkWeakHolder_800CC6C8 weakTarget;
                weakTarget.ptr = m_target;
                m_model->fn_8015FA78(fn_80169D00, &weakTarget);
                m_target->fn_80167010(-1);
            }

            if (!field_0x68.expired()) {
                fn_8034DB60(field_0x68);
            }
            field_0x68 = fn_80169B4C(m_rider, 11);
            fn_801D560C(fn_80053164(), 0x100A, 0, 0);
        }
        break;

    case STATE_FLYING:
        if (Move()) {
            fn_8034DB60(field_0x68);
            m_state = STATE_ARRIVED;
            m_timer = 5.0f;
            UpdateModelMatrix();
            fn_80292A64(m_rider, &m_hangPosition, &m_hangAngles);

            if (boost::shared_ptr<CharInfo> base = m_owner.lock()) {
                base->fn_801DC980(new ReadyCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
            }
        } else if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
            rider->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                            &m_hangPosition, &m_hangAngles, 4, 4));
        }
        break;

    case STATE_ARRIVED:
        Move();

        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_state = STATE_FINISHED;
        }
        break;

    case STATE_FINISHED:
        m_flags.Set(FLAG_FINISHED);
        break;
    }

    UpdateModelMatrix();
}

void Rocket::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        m_model->fn_801633DC(&matrix);

        Fn8040Vector3 hangOffset(0.0f, -8.0f, 1.0f);

        m_hangPosition = matrix * hangOffset;
        m_hangAngles.x = 0.0f;
        m_hangAngles.y = m_charInfo->m_rotation.y;
        m_hangAngles.z = 0.0f;
    }
}

void Rocket::SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation) {
    m_charInfo->m_position = *position;
    m_charInfo->m_rotation = *rotation;
    m_start = m_charInfo->m_position;
    UpdateModelMatrix();
}

void Rocket::SetVelocity(const Fn8040Vector3 *velocity) {
    m_velocity = *velocity;
}

void Rocket::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (!m_flags.Test(FLAG_BOARDED)) {
            m_flags.Set(FLAG_BOARDED);

            if (boost::shared_ptr<CharInfo> base = m_owner.lock()) {
                base->fn_801DC980(new UseCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
            }
        }
        break;

    case 13:
        if (m_flags.Test(FLAG_BOARDED) && m_state == STATE_WAITING) {
            m_state = STATE_IGNITION;
            m_timer = 0.5f;
        }
        break;
    }
}

bool Rocket::Move() {
    AddVector(&m_charInfo->m_position, m_velocity);

    Fn8040Vector3 offset = m_charInfo->m_position - m_start;
    if (fn_8040CF2C(&offset) > m_distanceSq) {
        return true;
    }
    return false;
}

void Rocket::OnCollision(ColliDetectArgs &args) {
    if (m_state == STATE_WAITING && !m_flags.Test(FLAG_TOUCHED)) {
        UnkHitContext_80115C28 context;

        fn_8015AE44(&args, fn_80116FEC, &context);

        if (context.m_found) {
            if (boost::shared_ptr<CharInfo> hit = context.m_hit.lock()) {
                hit->fn_801DC980(new Player::TouchHangCommand(
                    boost::weak_ptr<CharInfo>(m_charInfo), &m_hangPosition, &m_hangAngles, 4, 4));
                m_rider = context.m_hit;
                m_flags.Set(FLAG_TOUCHED);
            }
        }
    }
}

RocketTask::RocketTask(Task *parent, boost::shared_ptr<Rocket> rocket) : Task(parent) {
    m_name = s_name;
    m_rocket = rocket;
}

RocketTask::~RocketTask() {}

void RocketTask::Update(float dt) {
    m_rocket->Update(dt);
    if (m_rocket->IsFinished()) {
        m_flags |= 1;
    }
}

RocketBase::RocketBase(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x1E, 4, "RocketBase");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&RocketBase::OnCommand, this, _1));

    ReadOwner(owner);

    m_model = fn_8015F800(0, "ROCKET.BIN");
    m_model->fn_801629D8("base", 0);
    m_model->fn_80161F08("base", boost::weak_ptr<CharInfo>(m_charInfo));

    UnkFrameSearch_80169E8C mount;
    mount.id = 0x7D3;
    m_model->fn_8015FA78(fn_80169E8C, &mount);
    if (mount.frame != 0) {
        m_mountMatrix = RwFrameGetLTM(mount.frame);
    }

    UnkFrameSearch_80169E8C base;
    base.id = 0x7D0;
    m_model->fn_8015FA78(fn_80169E8C, &base);
    if (base.frame != 0) {
        m_baseFrame = base.frame;
        m_baseMatrix = m_baseFrame->modelling;
    }

    UpdateModelMatrix();
}

RocketBase::~RocketBase() {}

void RocketBase::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_rocket.get() != 0) {
        m_rocket->Update(dt);
    }

    UpdateModelMatrix();

    switch (m_state) {
    case STATE_EMPTY:
        m_rocket = boost::shared_ptr<Rocket>(new Rocket(boost::weak_ptr<CharInfo>(m_charInfo)));
        m_state = STATE_READY;
        break;

    case STATE_READY:
    case STATE_LAUNCHED:
        /* Matching workaround: the two states that wait are listed. With them the switch has the
         * original's shape, a compare with 0 and a branch for values below 0 (USA (GUPE8P)
         * 0x80114F0C-0x80114F18); without them it has one branch fewer and UpdateBody is 57 words
         * instead of 58. Revisit if the original's handling of these states is found. */
        break;
    }
}

void RocketBase::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();

        Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 end(0.0f, m_params.distance, 0.0f);
        Fn8040Vector3 mount(m_mountMatrix->pos.x, m_mountMatrix->pos.y, m_mountMatrix->pos.z);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, mount);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, DegreesToRadians(-m_params.angle));
        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

        UnkRotation3 rotation(fn_8000AB90(90.0f), 0.0f, 0.0f);
        fn_802E0C60(&end, &rotation, 0, m_params.distance, lbl_805F8560);
    }
}

void RocketBase::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_EMPTY;
    field_0x9C = 0.0f;
}

void RocketBase::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_baseFrame != 0) {
        fn_800E9C64(m_baseFrame, rwCOMBINEREPLACE, DegreesToRadians(-m_params.angle));
        fn_804876E8(m_baseFrame, &m_baseMatrix, rwCOMBINEPOSTCONCAT);
    }

    if (m_mountMatrix != 0 && m_rocket.get() != 0) {
        Fn8040Vector3 position(m_mountMatrix->pos.x, m_mountMatrix->pos.y, m_mountMatrix->pos.z);
        UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

        rotation.x = DegreesToRadians(-m_params.angle);
        rotation.y = m_charInfo->m_rotation.y;
        rotation.z = 0.0f;

        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, DegreesToRadians(-m_params.angle));

        Fn8040Vector3 velocity = matrix * up;
        fn_8040CE38(&velocity, 14.0f);

        m_rocket->SetTransform(&position, &rotation);
        m_rocket->SetVelocity(&velocity);
        m_rocket->SetDistance(m_params.distance);
    }
}

void RocketBase::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 13:
        if (m_state == STATE_READY) {
            Task *parent = fn_801EB304(fn_8001C4F8(), 11);

            new RocketTask(parent, m_rocket);
            m_rocket.reset();
            m_state = STATE_LAUNCHED;
        }
        break;

    case 14:
        if (m_state == STATE_LAUNCHED) {
            m_state = STATE_EMPTY;
        }
        break;
    }
}

} // namespace Rocket

extern "C" void fn_80114938(void) {
    fn_8006B474("ROCKET.BIN", 1);
}

extern "C" void fn_80114934(void) {}

void Rocket::RocketBase::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new RocketBase(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "angle", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {4, "distance", 0, 0, 0, 10.0f, 0.0f, 10000.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80526D4C = {
    "ROCKET", fn_80114938, fn_80114934, Rocket::RocketBase::Create, 0, 0, 0xE, 1,
    0,        10,          s_paramInfo,
};
