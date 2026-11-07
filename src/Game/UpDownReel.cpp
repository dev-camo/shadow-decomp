/* The UD_REEL gadget (class described in Game/UpDownReel.h). File name inferred from the class and
 * the namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The callbacks it hands to the character info's command unit and to the probe's handle are
 * boost::function objects built with boost::bind(&UpDownReel::method, this, _1); their manager,
 * invoker and copy functions come from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/UpDownReel.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8012773C.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The number of the resource that holds the gadget's models; the descriptor's load hook
 * fn_801273F8 adds it to the list of resources to load (name inferred). */
static const int s_resourceId = 0xE5;

namespace UpDownReel {

/* How far an update without a rider turns the handle back towards 0 (name inferred). */
static const float s_returnStep = DegreesToRadians(4.0f);
/* Added to the handle's y angle for the hang angles (name inferred). */
static const float s_halfTurn = DegreesToRadians(180.0f);

const char *UpDownReel::s_name = "UpDownReel";

/* What the probe callback passes to fn_8015AE44 (placeholder name). The probe callback fills in
 * m_charInfo and m_line; the hit callback below stores the character it is called with in m_hit
 * and sets m_found. */
struct UnkHitContext_80127F44 {
    UnkHitContext_80127F44() : m_found(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::weak_ptr<UnkSegment_80020A20> m_line;
    /* 0x10 */ boost::weak_ptr<CharInfo> m_hit;
    /* 0x18 */ bool m_found;
};
CHECK_SIZEOF(UnkHitContext_80127F44, 0x1C);

/* The callback that fn_8015AE44 calls for each character info in the hits (placeholder name): when
 * there is a character info and a context, stores the character info in m_hit; it sets m_found
 * without testing the context. The context test before the store is in the original's code
 * (without it the function is 18 words long instead of 20). */
static void fn_801291F0(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80127F44 *context = static_cast<UnkHitContext_80127F44 *>(arg);

    if (hit.get() != 0 && context != 0) {
        context->m_hit = hit;
    }

    context->m_found = true;
}

UpDownReel::UpDownReel(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_handleRotation(0.0f, 0.0f, 0.0f), m_hangAngles(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x18, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&UpDownReel::OnCommand, this, _1));

    m_models[0] = new GadgetModelInfo(0, s_resourceId);
    m_models[0]->fn_80162A68(0, 0);
    m_models[2] = new GadgetModelInfo(0, s_resourceId);
    m_models[2]->fn_80162A68(2, 0);
    m_models[1] = new GadgetModelInfo(0, s_resourceId);
    m_models[1]->fn_80162A68(1, 0);
    m_state = STATE_WAITING;

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7EA8(fn_80021B08(), &m_charInfo->m_position, 0x40000, 10, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), &lbl_805E07A0, 12.0f);

    m_handle = fn_8015B8C4(probe);
    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&UpDownReel::OnCollision, this, _1)));
    m_line = boost::shared_ptr<UnkSegment_80020A20>(new UnkSegment_80020A20);

    ReadOwner(m_owner);
    UpdateModelMatrix();
}

UpDownReel::~UpDownReel() {
    m_sound.fn_801D455C();

    for (int i = 0; i < 3; i++) {
        if (m_models[i] != 0) {
            delete m_models[i];
            m_models[i] = 0;
        }
    }
}

void UpDownReel::UpdateBody(float dt) {
    m_rideFlags.Reset(RIDE_TOUCHED);

    switch (m_state) {
    case STATE_WAITING:
        if (m_params.length - m_params.pos > 0.0f) {
            m_ropeLength += 0.18f;
            if (m_ropeLength >= m_params.length) {
                m_ropeLength = m_params.length;
            }
        } else if (m_params.length - m_params.pos < 0.0f) {
            m_ropeLength -= 0.18f;
            if (m_ropeLength <= m_params.length) {
                m_ropeLength = m_params.length;
            }
        }
        break;
    case STATE_SHORTENING:
        m_ropeLength -= 0.18f;
        if (m_ropeLength <= m_params.pos) {
            m_ropeLength = m_params.pos;
            m_state = STATE_ARRIVED;
            m_sound.fn_801D455C();
        }
        break;
    case STATE_LENGTHENING:
        m_ropeLength += 0.18f;
        if (m_ropeLength > m_params.pos) {
            m_ropeLength = m_params.pos;
            m_state = STATE_ARRIVED;
            m_sound.fn_801D455C();
        }
        break;
    case STATE_ARRIVED:
        /* The ride stops in this state until command id 12 or ReadOwner sets STATE_WAITING;
         * nothing moves the rope here.
         * Matching workaround: the state is listed as an empty case. With it the switch has the
         * original's shape, a compare with 2 first, then with 0, and an unreachable branch to the
         * end after the default branch (USA (GUPE8P) 0x80127FBC-0x80127FD8); without it the switch
         * compares with 1 first and UpdateBody is 204 words instead of 203. Revisit if the
         * original's handling of this state is found. */
        break;
    }

    UpdateModelMatrix();

    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }

    for (int i = 0; i < 3; i++) {
        if (m_models[i] != 0) {
            m_models[i]->fn_80162FB0(dt);
        }
    }

    if (m_rideFlags.Test(RIDE_RIDDEN)) {
        if (m_rideFlags.Test(RIDE_RELEASED)) {
            if (!m_rideFlags.Test(RIDE_NO_LAUNCH)) {
                Launch();
            }

            m_rideFlags.Reset(RIDE_RIDDEN);
            m_rideFlags.Reset(RIDE_RELEASED);
            m_rider.reset();

            if (m_handle.get() != 0) {
                m_handle->SuspendProbe(1.0f);
            }
        } else if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
            m_hangAngles = m_handleRotation;
            m_hangAngles.y += s_halfTurn + m_charInfo->m_rotation.y;
            rider->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                            &m_hangPosition, &m_hangAngles, 3, 4));
        }
    } else {
        m_handleRotation.y = fn_8012773C(0.0f, m_handleRotation.y, s_returnStep);
    }
}

void UpDownReel::OnCollision(ColliDetectArgs &args) {
    if (!m_rideFlags.Test(RIDE_RIDDEN) && !m_rideFlags.Test(RIDE_TOUCHED)) {
        UnkHitContext_80127F44 context;

        context.m_charInfo = m_charInfo;
        context.m_line = m_line;
        fn_8015AE44(&args, fn_801291F0, &context);

        if (context.m_found) {
            if (boost::shared_ptr<CharInfo> hit = context.m_hit.lock()) {
                hit->fn_801DC980(new Player::TouchHangCommand(
                    boost::weak_ptr<CharInfo>(m_charInfo), &m_hangPosition, &m_hangAngles, 3, 4));
            }

            m_rideFlags.Set(RIDE_TOUCHED);
            m_rideFlags.Reset(RIDE_RELEASED);
            m_rider.reset();
        }
    }
}

void UpDownReel::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }

    /* Queues three debug lines, 20 long, along the x, y and z axes through the point 10 * "pos"
     * below the character info, and a debug line from (0, -14, 0) to (0, -14, 50) in a space 10 *
     * m_ropeLength below the character info, turned by its yaw and by "angle" about x (behaviour
     * inferred). */
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B52C(&matrix, 0.0f, 10.0f * -m_params.pos, 0.0f);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);

    Fn8040Vector3 center(0.0f, 0.0f, 0.0f);
    center = matrix * center;

    Fn8040Vector3 start;
    Fn8040Vector3 end;

    start = end = center;
    start.x -= 10.0f;
    end.x += 10.0f;
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

    start = end = center;
    start.y -= 10.0f;
    end.y += 10.0f;
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

    start = end = center;
    start.z -= 10.0f;
    end.z += 10.0f;
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

    Fn8040Matrix34 launchMatrix;

    fn_8040C148(&launchMatrix, &lbl_805E0720);
    fn_8040B59C(&launchMatrix, m_charInfo->m_position);
    fn_8040B52C(&launchMatrix, 0.0f, 10.0f * -m_ropeLength, 0.0f);
    fn_8040B848(&launchMatrix, m_charInfo->m_rotation.y);
    fn_8040B884(&launchMatrix, DegreesToRadians(m_params.angle));

    Fn8040Vector3 lineStart(0.0f, -14.0f, 0.0f);
    Fn8040Vector3 lineEnd(0.0f, -14.0f, 50.0f);

    lineStart = launchMatrix * lineStart;
    lineEnd = launchMatrix * lineEnd;
    fn_802E0B58(UnkSegment_80020A20(lineStart, lineEnd), lbl_805F1370);
}

void UpDownReel::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_ropeLength = m_params.length;
    m_state = STATE_WAITING;
    m_rideFlags.Clear();
    fn_8040E970(&m_handleRotation);
}

void UpDownReel::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);

    if (m_models[0] != 0) {
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_models[0]->fn_801633DC(&matrix);
    }

    if (m_models[2] != 0) {
        fn_8040B720(&matrix, 1.0f, m_ropeLength, 1.0f);
        m_models[2]->fn_801633DC(&matrix);
    }

    if (m_models[1] != 0) {
        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B52C(&matrix, 0.0f, 10.0f * -m_ropeLength, 0.0f);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B848(&matrix, m_handleRotation.y);
        m_models[1]->fn_801633DC(&matrix);

        if (m_handle.get() != 0) {
            Fn8040Vector3 probePosition(0.0f, -8.0f, 0.0f);
            probePosition = matrix * probePosition;
            m_handle->m_probe->SetPosition(&probePosition);
        }

        Fn8040Vector3 right(10.0f, -14.0f, 0.0f);
        Fn8040Vector3 left(-10.0f, -14.0f, 0.0f);

        /* The original reads the segment pointer before each transform call; these assignments
         * read it after the call (residual in configure.py). */
        m_line->start = matrix * right;
        m_line->end = matrix * left;
        m_hangPosition = (m_line->start + m_line->end) * 0.5f;
    }
}

} // namespace UpDownReel

/* Brings an angle that is at most one turn outside the range -pi to pi back into it, by adding
 * or subtracting 2 pi once (name inferred). */
static inline float WrapAngle(float angle) {
    if (angle < -lbl_805FAD38) {
        angle += 2.0f * lbl_805FAD38;
    } else if (angle > lbl_805FAD38) {
        angle -= 2.0f * lbl_805FAD38;
    }
    return angle;
}

/* The turn helper declared in Game/fn_8012773C.h (other gadgets call it too); its body is in this
 * file. */
extern "C" float fn_8012773C(float target, float current, float step) {
    float difference = WrapAngle(target - current);

    if (difference <= step && difference >= -step) {
        return target;
    }

    float result;

    if (difference < 0.0f) {
        result = current - step;
    } else {
        result = current + step;
    }

    return WrapAngle(result);
}

namespace UpDownReel {

void UpDownReel::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_state == STATE_WAITING) {
            if (m_params.length - m_params.pos > 0.0f) {
                m_state = STATE_SHORTENING;
                m_sound = fn_801D3EF0(0x1000, this);
            } else if (m_params.length - m_params.pos < 0.0f) {
                m_state = STATE_LENGTHENING;
                m_sound = fn_801D3EF0(0x1000, this);
            }

            m_rideFlags.Set(RIDE_RIDDEN);
            m_rideFlags.Reset(RIDE_RELEASED);
            m_rideFlags.Reset(RIDE_NO_LAUNCH);
            /* Command id 11 is a RideCommand, as in ElecBar's handler, which checks it with
             * dynamic_cast; this handler reads its rider without a check (inferred). */
            m_rider = static_cast<RideCommand &>(command).m_rider;
        }
        break;
    case 12:
        m_rideFlags.Set(RIDE_RELEASED);
        m_state = STATE_WAITING;
        m_sound.fn_801D455C();
        break;
    case 15:
        m_rideFlags.Set(RIDE_NO_LAUNCH);
        break;
    case 1: {
        static const float s_inputTurnStep = DegreesToRadians(2.0f);
        AccelerateInputCommand *accelerate = static_cast<AccelerateInputCommand *>(&command);

        /* The original's code tests the command's address (without the test OnCommand is 102
         * words long instead of 104). */
        if (accelerate != 0) {
            float yaw;
            float pitch;

            fn_8003B9E8(&accelerate->m_vector, &pitch, &yaw);
            yaw -= m_charInfo->m_rotation.y;
            m_handleRotation.y = fn_8012773C(yaw, m_handleRotation.y, s_inputTurnStep);
        }
        break;
    }
    }
}

void UpDownReel::Launch() {
    if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
        rider->fn_801DC980(new ClrVelocityCommand(4));

        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, DegreesToRadians(m_params.angle));

        Fn8040Vector3 velocity(0.0f, 0.0f, 1.0f);

        velocity = matrix * velocity;
        fn_8040CE38(&velocity, m_params.power);

        rider->fn_801DC980(new SetVelocityCommand(&velocity, 4));
        rider->fn_801DC980(new Player::LaunchCommand(0, 4));
    }
}

} // namespace UpDownReel

extern "C" void fn_801273F8(void) {
    fn_8006B50C(s_resourceId, 1);
}

extern "C" void fn_801273F4(void) {}

void UpDownReel::UpDownReel::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new UpDownReel(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "length", 0, 0, 0, 1.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "pos", 0, 0, 0, 1.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "angle", 0, 0, 0, 0.0f, -90.0f, 90.0f, fn_800CBC20, 0},
    {4, "power", 0, 0, 0, 400.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80527FC0 = {
    "UD_REEL", fn_801273F8, fn_801273F4, UpDownReel::UpDownReel::Create, 0, 0, 8, 1,
    0,         10,          s_paramInfo,
};
