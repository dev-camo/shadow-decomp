/* Tornado (class name from RTTI; the gadget's descriptor is "TORNADO"): a tornado that takes the
 * characters the TornadoCollision tasks hand to its character info (TornadoCharInfo), carries each
 * of them up round its cylinder with a TornadoTowing task and launches them (behaviour inferred).
 * File name inferred from the class. Method and member names are inferred. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/Tornado.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommand.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkManager_80009548.h"
#include "Game/fn_8006_resource_list.h"
#include <algorithm>
#include <cmath>

const char *TornadoTowing::s_name = "TornadoTowing";

const char *Tornado::s_name = "Tornado";

/* The original has this destructor, as a global function, right after AttachTarget, with the
 * class's vtable in this file. */
TornadoCharInfo::~TornadoCharInfo() {}

bool TornadoCharInfo::AttachTarget(int tag, boost::shared_ptr<CharInfo> victim) {
    if (tag == m_tornado->m_tag) {
        m_tornado->AddTowing(victim);
        return true;
    }

    return false;
}

TornadoTowing::TornadoTowing(Task *parent, Tornado *tornado, boost::shared_ptr<CharInfo> charInfo)
    : Task(parent), m_tornado(tornado), m_state(0), m_charInfo(charInfo) {
    /* Matching workaround: every vector is declared here, in this order, because the order of
     * declaration places their stack slots; each *Temporary local receives a computed vector
     * before it is copied, possibly standing for a by-value temporary of the original (inferred).
     * upForRight and upForLeft hold the same up vector (0, 1, 0), once for each branch, as the
     * original has two stack slots for it. Revisit when the vector functions are modelled with
     * by-value results. */
    Fn8040Vector3 position;
    Fn8040Vector3 center;
    Fn8040Vector3 toCenter;
    Fn8040Vector3 direction;
    Fn8040Vector3 sideways;
    Fn8040Vector3 offset;
    Fn8040Vector3 toCenterTemporary;
    Fn8040Vector3 directionTemporary;
    Fn8040Vector3 turnedRightTemporary;
    Fn8040Vector3 turnedLeftTemporary;
    Fn8040Vector3 scaledTemporary;
    Fn8040Vector3 startTemporary;
    Fn8040Vector3 upForRight;
    Fn8040Vector3 sidewaysRightTemporary;
    Fn8040Vector3 upForLeft;
    Fn8040Vector3 sidewaysLeftTemporary;
    Fn8040Vector3 sidewaysStartTemporary;
    Fn8040Vector3 offsetTemporary;
    Fn8040Vector3 forward;
    Fn8040Vector3 startAheadTemporary;
    Fn8040Vector3 directionToStart;

    m_name = s_name;

    charInfo->fn_801DC980(new Player::IgnoreInputCommand(2, 0));
    charInfo->fn_801DC980(new Player::StreamCommand(true, 4));
    charInfo->fn_801DC980(new Player::InvincibleCommand(true, 4));

    fn_800091D0(&m_position, &charInfo->m_position);
    fn_800091EC(&position, &m_position);
    fn_800091EC(&center, &m_tornado->m_charInfo->m_position);
    position.y = 0.0f;
    center.y = 0.0f;
    fn_8040CD74(&toCenterTemporary, &center, &position);
    fn_800091EC(&toCenter, &toCenterTemporary);

    float distance = fn_8040CF50(&toCenter);

    if (distance > m_tornado->m_params.cylinderRadius) {
        /* Outside the cylinder the start is the point where a tangent from the character touches
         * the cylinder, on the side that the parameter "TURN" selects. */
        float angle = std::asin(m_tornado->m_params.cylinderRadius / distance);
        float cosine = std::cos(angle);
        float tangentLength = cosine * distance;

        fn_8040CEE8(&directionTemporary, &toCenter);
        fn_800091EC(&direction, &directionTemporary);

        if (m_tornado->m_params.turn != 0) {
            fn_8040D0AC(&turnedRightTemporary, &direction, angle);
            fn_800091D0(&direction, &turnedRightTemporary);
        } else {
            fn_8040D0AC(&turnedLeftTemporary, &direction, -angle);
            fn_800091D0(&direction, &turnedLeftTemporary);
        }

        fn_8040CDA8(&scaledTemporary, &direction, tangentLength);
        fn_800091D0(&direction, &scaledTemporary);
        fn_8040CDD0(&startTemporary, &m_position, &direction);
        fn_800091D0(&m_start, &startTemporary);
        m_start.y = m_tornado->m_charInfo->m_position.y - m_tornado->m_params.cylinderY;
    } else if (distance != 0.0f) {
        /* Inside it the start is the point of the cylinder's surface at right angles to the
         * direction to the centre, on the side that "TURN" selects. */
        const Fn8040Vector3 *crossResult;

        if (m_tornado->m_params.turn != 0) {
            upForRight.Set(0.0f, 1.0f, 0.0f);
            fn_8040CFB8(&sidewaysRightTemporary, &upForRight, &toCenter);
            crossResult = &sidewaysRightTemporary;
        } else {
            upForLeft.Set(0.0f, 1.0f, 0.0f);
            fn_8040CFB8(&sidewaysLeftTemporary, &toCenter, &upForLeft);
            crossResult = &sidewaysLeftTemporary;
        }

        fn_800091EC(&sideways, crossResult);
        fn_8040CE74(&sideways);
        fn_8040CE38(&sideways, m_tornado->m_params.cylinderRadius);
        fn_8040CDD0(&sidewaysStartTemporary, &sideways, &m_tornado->m_charInfo->m_position);
        fn_800091D0(&m_start, &sidewaysStartTemporary);
        m_start.y = m_tornado->m_charInfo->m_position.y - m_tornado->m_params.cylinderY;
    } else {
        /* On the axis the start is the point of the surface in the z direction. */
        forward.Set(0.0f, 0.0f, 1.0f);
        fn_8040CDA8(&offsetTemporary, &forward, m_tornado->m_params.cylinderRadius);
        fn_800091EC(&offset, &offsetTemporary);
        fn_8040CDD0(&startAheadTemporary, &offset, &m_tornado->m_charInfo->m_position);
        fn_800091D0(&m_start, &startAheadTemporary);
        m_start.y = m_tornado->m_charInfo->m_position.y - m_tornado->m_params.cylinderY;
    }

    fn_8040CD74(&directionToStart, &m_start, &m_position);
    fn_800091D0(&m_direction, &directionToStart);
    fn_8040CE74(&m_direction);

    fn_8000D664(fn_80009548(), 0, -1.0f, 12.0f, 10.0f);

    if (charInfo->fn_80078B44()) {
        m_effect = fn_8034DEE4(7, -2);
    }

    m_state = 1;
}

TornadoTowing::~TornadoTowing() {
    if (m_state != 4) {
        boost::shared_ptr<CharInfo> charInfo = m_charInfo.lock();

        if (charInfo.get() != 0) {
            charInfo->fn_801DC980(new Player::IgnoreInputCommand(3, 0));
            charInfo->fn_801DC980(new Player::InvincibleCommand(false, 4));
        }
    }

    fn_8000D534(fn_80009548(), 0);
    fn_8034DB60(m_effect);
    m_tornado->RemoveTowing(this);
}

void TornadoTowing::Update(float dt) {
    boost::shared_ptr<CharInfo> charInfo = m_charInfo.lock();

    if (charInfo.get() != 0) {
        switch (m_state) {
        case 1:
            if (MoveToCylinder(charInfo, dt)) {
                m_state = 2;
            }
            break;
        case 2:
            if (Rise(charInfo, dt)) {
                m_state = 3;
            }
            break;
        case 3:
            if (Circle(charInfo, dt)) {
                Launch(charInfo);
                m_state = 4;
                m_flags |= 1;
            }
            break;
        }
    } else {
        m_flags |= 1;
    }
}

bool TornadoTowing::MoveToCylinder(boost::shared_ptr<CharInfo> charInfo, float dt) {
    /* Matching workaround: the vectors are declared here, in this order, because the order of
     * declaration places their stack slots (declared at their first uses, 5 words differ);
     * toStartTemporary possibly stands for a by-value temporary of the original (inferred).
     * Revisit when the vector functions are modelled with by-value results. */
    bool reached = false;
    Fn8040Vector3 toStart;
    Fn8040Vector3 toStartTemporary;
    Fn8040Vector3 move;

    fn_8040CD74(&toStartTemporary, &m_start, &m_position);
    fn_800091EC(&toStart, &toStartTemporary);

    float step = m_tornado->m_params.turnSpeed * dt;

    if (fn_8040CF50(&toStart) <= step) {
        step = fn_8040CF50(&toStart);
        reached = true;
    }

    fn_800091A4(&move, &m_direction, step);
    AddVector(&m_position, move);
    charInfo->fn_801DC980(new SetPositionCommand(&m_position, 4));
    return reached;
}

bool TornadoTowing::Rise(boost::shared_ptr<CharInfo> charInfo, float dt) {
    bool reached = false;
    float angle = dt * (m_tornado->m_params.turnSpeed / m_tornado->m_params.cylinderRadius);
    /* Matching workaround: the vectors are declared here, in this order, because the order of
     * declaration places their stack slots (declared at their first uses, 12 words differ); the
     * *Temporary locals possibly stand for by-value temporaries of the original (inferred).
     * Revisit when the vector functions are modelled with by-value results. */
    Fn8040Vector3 offset;
    Fn8040Vector3 offsetTemporary;
    Fn8040Vector3 turnedLeftTemporary;
    Fn8040Vector3 turnedRightTemporary;

    fn_8040CD74(&offsetTemporary, &m_position, &m_tornado->m_charInfo->m_position);
    fn_800091EC(&offset, &offsetTemporary);
    offset.y = 0.0f;

    if (m_tornado->m_params.turn == 0) {
        fn_8040D0AC(&turnedLeftTemporary, &offset, angle);
        fn_800091D0(&offset, &turnedLeftTemporary);
    } else {
        fn_8040D0AC(&turnedRightTemporary, &offset, -angle);
        fn_800091D0(&offset, &turnedRightTemporary);
    }

    AddVector(&offset, m_tornado->m_charInfo->m_position);
    offset.y = m_position.y + m_tornado->m_params.upSpeed * dt;

    float top = m_tornado->m_params.cylinderY + m_tornado->m_charInfo->m_position.y;

    if (offset.y > top) {
        offset.y = top;
        fn_800091D0(&m_top, &m_tornado->m_charInfo->m_position);
        reached = true;
        m_top.y += m_tornado->m_params.cylinderY;
    }

    fn_800091D0(&m_position, &offset);
    charInfo->fn_801DC980(new SetPositionCommand(&m_position, 4));
    return reached;
}

bool TornadoTowing::Circle(boost::shared_ptr<CharInfo> charInfo, float dt) {
    float turnStep = m_tornado->m_params.turnSpeed * dt;
    float upStep = m_tornado->m_params.upSpeed * dt;
    /* Matching workaround: the vectors are declared here, in this order, because the order of
     * declaration places their stack slots (declared at their first uses, 12 words differ); the
     * *Temporary locals possibly stand for by-value temporaries of the original (inferred).
     * Revisit when the vector functions are modelled with by-value results. */
    Fn8040Vector3 offset;
    Fn8040Vector3 offsetTemporary;
    Fn8040Vector3 turnedLeftTemporary;
    Fn8040Vector3 turnedRightTemporary;
    Fn8040Vector3 positionTemporary;

    fn_8040CD74(&offsetTemporary, &m_position, &m_top);
    fn_800091EC(&offset, &offsetTemporary);
    offset.y = 0.0f;

    float radius = fn_8040CF50(&offset) - upStep;

    if (radius <= turnStep) {
        return true;
    }

    float angle = turnStep / m_tornado->m_params.cylinderRadius;

    fn_8040CE74(&offset);

    if (m_tornado->m_params.turn == 0) {
        fn_8040D0AC(&turnedLeftTemporary, &offset, angle);
        fn_800091D0(&offset, &turnedLeftTemporary);
    } else {
        fn_8040D0AC(&turnedRightTemporary, &offset, -angle);
        fn_800091D0(&offset, &turnedRightTemporary);
    }

    fn_8040CE38(&offset, radius);
    fn_8040CDD0(&positionTemporary, &m_top, &offset);
    fn_800091D0(&m_position, &positionTemporary);
    charInfo->fn_801DC980(new SetPositionCommand(&m_position, 4));
    return false;
}

/* Matching workaround: the three command constructors below are defined here, out of line, as
 * strong globals. The original has a single copy of each, here after the towing task's launch
 * function, their first caller; other units call the copies of the SetVelocityCommand and
 * IgnoreInputCommand constructors. That fits the out-of-line copy of an inline constructor that
 * every constructing unit emits and the linker keeps from the first unit in link order (see
 * DashPanel.cpp's CancelBehaviorCommand). That the constructors were inline in the original
 * headers is inferred, not established. Revisit when the command headers model those
 * constructors. */
Player::InvincibleCommand::InvincibleCommand(bool enable, int arg1)
    : PlayerCommand(0x126, arg1), m_enable(enable) {}

Player::IgnoreInputCommand::IgnoreInputCommand(int arg0, float seconds)
    : PlayerCommand(0x127, 0), m_seconds(seconds), field_0x18(0), m_arg0(arg0) {}

SetVelocityCommand::SetVelocityCommand(const Fn8040Vector3 *velocity, int arg1)
    : CharCommand(5, arg1) {
    fn_800091EC(&m_velocity, velocity);
}

void TornadoTowing::Launch(boost::shared_ptr<CharInfo> charInfo) {
    /* Matching workaround: velocity is declared before the commands and the two rotation results
     * after them, because the order of declaration places their stack slots. The original keeps
     * the rotation results below the SetAngleCommand argument, where temporaries made after it
     * go; here they take the slots above it (8 words differ, all stack offsets of the argument and
     * the two results). Revisit when the vector functions are modelled with by-value results. */
    Fn8040Vector3 velocity;

    charInfo->fn_801DC980(new Player::IgnoreInputCommand(3, 0));
    charInfo->fn_801DC980(new Player::InvincibleCommand(false, 4));
    charInfo->fn_801DC980(new Player::IgnoreInputCommand(0, m_tornado->m_params.noControl));
    charInfo->fn_801DC980(new ClrVelocityCommand(4));
    charInfo->fn_801DC980(new SetPositionCommand(&m_top, 4));
    charInfo->fn_801DC980(new SetAngleCommand(
        UnkRotation3(0.0f, m_tornado->m_params.launchY - lbl_805FAD38, 0.0f), 2, 4));

    Fn8040Vector3 turnedTemporary;
    Fn8040Vector3 turnedAgainTemporary;

    velocity.Set(0.0f, 0.0f, 1.0f);
    fn_8040D118(&turnedTemporary, &velocity, m_tornado->m_params.launchX);
    fn_800091D0(&velocity, &turnedTemporary);
    fn_8040D0AC(&turnedAgainTemporary, &velocity, m_tornado->m_params.launchY);
    fn_800091D0(&velocity, &turnedAgainTemporary);
    fn_8040CE38(&velocity, m_tornado->m_params.launchSpeed);
    charInfo->fn_801DC980(new SetVelocityCommand(&velocity, 4));
    charInfo->fn_801DC980(new Player::StreamCommand(false, 4));
}

extern "C" void fn_800DE434(void) {
    fn_8006B474("TORNADO.BIN", 1);
}

extern "C" void fn_800DE430(void) {}

void Tornado::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Tornado(fn_801EB304(fn_8001C4F8(), 11), owner);
}

Tornado::Tornado(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;

    m_towings.clear();
    m_towings.reserve(2);

    m_charInfo = boost::shared_ptr<TornadoCharInfo>(new TornadoCharInfo(this));
    m_charInfo->SetField0x5C(4);
    fn_801DCD8C(fn_800DDF20(), m_charInfo);

    ReadOwner(owner);

    m_model = fn_8015F800(1, "TORNADO.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160BA0("TornadoMtn");
    m_model->fn_801607A8("TornadoMtn");
    m_model->fn_8015FF60();
    m_model->fn_801609F8("TornadoMtn");
    UpdateModelMatrix();

    m_sound = fn_801D3E58(0x732E, &m_charInfo->m_position, this);
}

/* Returns the character info table, constructing it on first use (behaviour inferred). Other
 * units (TornadoCollision.cpp, MonsterControl.cpp) call this copy.
 * Matching workaround: a global function, where the original's copy looks like the out-of-line
 * copy of an inline function (inferred): it sits among the functions that ~Tornado calls out of
 * line, before the converting constructor of the character info pointer (here it comes after
 * it), and its .sbss guard precedes its pointer (here it follows it). Defined inline here, it is
 * expanded into the constructor and the destructor, which the original keeps as calls; the
 * RainEffect gadget's GetTaskGroup is the same case. Revisit when the condition that keeps the
 * original out of line is found. */
extern "C" UnkCharInfoTable_800DDF20 *fn_800DDF20(void) {
    static UnkCharInfoTable_800DDF20 *s_table;

    if (s_table == 0) {
        static UnkCharInfoTable_800DDF20 s_instance;
        s_table = &s_instance;
    }

    return s_table;
}

Tornado::~Tornado() {
    m_sound.fn_801D455C();
    fn_801DCCF4(fn_800DDF20(), m_charInfo);
}

void Tornado::UpdateBody(float dt) {
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void Tornado::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    DrawRange();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
}

bool Tornado::AddTowing(boost::shared_ptr<CharInfo> charInfo) {
    for (std::vector<TornadoTowing *>::iterator it = m_towings.begin(); it != m_towings.end();
         ++it) {
        boost::shared_ptr<CharInfo> towed = (*it)->m_charInfo.lock();

        if (towed == charInfo) {
            return false;
        }
    }

    m_towings.push_back(new TornadoTowing(this, this, charInfo));
    return true;
}

void Tornado::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9E60(owner, &m_tag);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_params.turnSpeed = 10.0f * m_params.turnSpeed;
    m_params.upSpeed = 10.0f * m_params.upSpeed;
    m_params.cylinderRadius = 10.0f * m_params.cylinderRadius;
    m_params.cylinderY = 0.5f * (10.0f * m_params.cylinderY);
    m_params.cylinderOffsetY = 10.0f * m_params.cylinderOffsetY;
    m_params.launchX = DegreesToRadians(m_params.launchX);
    m_params.launchY = DegreesToRadians(m_params.launchY);
    m_params.launchSpeed = 10.0f * m_params.launchSpeed;

    m_charInfo->m_position.y += m_params.cylinderOffsetY;
}

void Tornado::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        /* Matching workaround: toPointTemporary is declared after toPoint and angles because the
         * order of declaration places their stack slots (declared before toPoint, 12 words
         * differ); it possibly stands for a by-value temporary of the original (inferred). Revisit
         * when the vector functions are modelled with by-value results. */
        Fn8040Vector3 position;
        Fn8040Vector3 toPoint;
        UnkRotation3 angles;
        Fn8040Vector3 toPointTemporary;
        Fn8040Matrix34 matrix;

        fn_800091EC(&position, &m_charInfo->m_position);
        position.y -= m_params.cylinderOffsetY;

        Fn8040Vector3 *pointPosition = fn_80010244(fn_80009548(), 0);
        fn_8040CD74(&toPointTemporary, pointPosition, &position);
        fn_800091EC(&toPoint, &toPointTemporary);
        angles.Set(0.0f, 0.0f, 0.0f);
        toPoint.y = 0.0f;
        fn_8040CE74(&toPoint);
        fn_8003B9E8(&toPoint, &angles.x, &angles.y);

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &position);
        fn_8040B848(&matrix, angles.y + lbl_805FAD38);
        m_model->fn_801633DC(&matrix);
    }
}

void Tornado::RemoveTowing(TornadoTowing *towing) {
    std::vector<TornadoTowing *>::iterator it =
        std::find(m_towings.begin(), m_towings.end(), towing);

    if (it != m_towings.end()) {
        m_towings.erase(it);
    }
}

void Tornado::DrawRange() {
    /* Matching workaround: direction is declared first, and set after the line, because the order
     * of declaration places the stack slots of direction and rotation. Revisit with the other
     * vector declarations of this file. */
    Fn8040Vector3 direction;
    float pitch = DegreesToRadians(90.0f);
    UnkRotation3 rotation(pitch, 0.0f, 0.0f);

    fn_802E0C60(&m_charInfo->m_position, &rotation, 0, m_params.cylinderY, m_params.cylinderRadius);

    UnkSegment_80020A20 line;
    direction.Set(0.0f, 0.0f, 1.0f);
    Fn8040Vector3 turnedTemporary;
    Fn8040Vector3 turnedAgainTemporary;
    Fn8040Vector3 endTemporary;

    fn_800091D0(&line.start, &m_charInfo->m_position);
    line.start.y += m_params.cylinderY;
    fn_8040D118(&turnedTemporary, &direction, m_params.launchX);
    fn_800091D0(&direction, &turnedTemporary);
    fn_8040D0AC(&turnedAgainTemporary, &direction, m_params.launchY);
    fn_800091D0(&direction, &turnedAgainTemporary);
    fn_8040CE38(&direction, 0.1f * m_params.launchSpeed);
    fn_8040CDD0(&endTemporary, &line.start, &direction);
    fn_800091D0(&line.end, &endTemporary);
    fn_802E0AA0(line, UnkDebugColor_802E0E84(255, 255, 0, 0), 5.0f);
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "TURN 0:L 1:R", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "TURN_SPEED m/s", 0, 0, 0, 50.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {4, "UP_SPEED m/s", 0, 0, 0, 20.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {4, "CYLINDER_R m", 0, 0, 0, 20.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "CYLINDER_Y m", 0, 0, 0, 40.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {4, "CYLINDER_OFFSET_Y m", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "LAUNCH_X deg", 0, 0, 0, -30.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "LAUNCH_Y deg", 0, 0, 0, 0.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "LAUNCH_SPEED m/s", 0, 0, 0, 100.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {4, "NO_CONTROL sec", 0, 0, 0, 3.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80523E08 = {
    "TORNADO", fn_800DE434, fn_800DE430, Tornado::Create, 0, 0, 0x834, 1, 0, 10, s_paramInfo,
};
