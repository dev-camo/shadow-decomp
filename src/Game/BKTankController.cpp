/* BKTankController::Controller (described in Game/BKTankController.h) and the path control it
 * owns, whose class is defined here. File name inferred from the namespace. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first).
 *
 * The command handler is a boost::function object built with
 * boost::bind(&Controller::OnCommand, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/BKTankController.h"
#include "Game/CharCommand.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetCommand.h"
#include "Game/Mission.h"
#include "Game/PathCtrl.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkRailRoute_8054B8E0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_80177CF8.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

static const char *s_name = "BKTank::Controller";

/* The routes that the BK_TANK object follows in turn: two path numbers each, the second used
 * while the word at 0x8 is 1. A SwitchCommand toggles that word of the route it names. */
static const int ROUTE_COUNT = 4;
static UnkRailRoute_8054B8E0 s_routes[ROUTE_COUNT] = {
    {100, 100, 0, {1, 0, 0, 0}},
    {101, 101, 0, {1, 0, 0, 0}},
    {102, 102, 0, {1, 0, 0, 0}},
    {104, 104, 0, {1, 0, 0, 0}},
};

/* The 0x70-byte object that BKTankController::Controller owns at 0x38 (placeholder name; the
 * original class is unknown): where the BK_TANK object is on the paths of the route table. Its
 * constructor (0x80322450) constructs a SonicteamUSA::System::PathCtrl at its start and sets the
 * words at 0x58 to 0x6C. The PathCtrl is a member, not a base class: the controller's destructor
 * passes -1 to PathCtrl's destructor for it (0x80321A98), as it does for the members it destroys,
 * and 0 to the destructor of its own Task base (0x80321AF0); with PathCtrl as a base class the
 * compiler passes 0 there. Member and method names are inferred from the code that uses them. */
class UnkPathControl_80322450 {
  public:
    UnkPathControl_80322450();

    /* Does nothing when the mission that fn_800740E4 returns for 2 exists and its word at 0xC is
     * 2. Otherwise selects the route and calls SetupPath; then, whether or not the path exists, a
     * fraction from 0 to 1 sets the distance to that fraction of what fn_800183D8 returns for the
     * path. */
    void SetRoute(int route, float fraction);
    /* Goes on to the next route. Returns false when there is none (the route number stays one
     * past the last) or when SetupPath returns false for it. */
    bool NextRoute();
    /* Does nothing and returns false while fn_80169CD0 returns true or when charInfo has expired.
     * Otherwise takes speed * dt + 0.5 * acceleration * dt * dt as the step and changes the speed
     * by acceleration * dt (added while the target speed is higher, subtracted while it is
     * lower); when that passes the target speed, the speed becomes the target speed and the step
     * that speed times dt. Adds the step to the distance and moves the character info's position
     * and rotation to that distance along the path with fn_8001C91C, whose result it returns. */
    bool Update(float dt, boost::weak_ptr<CharInfo> charInfo);
    /* Sets the speed and the target speed to 150 and the acceleration to 0, then sets up the path
     * of the current route: the route's second path number when its switch word is 1, otherwise
     * its first. When that path exists, clears the distance and returns true. */
    bool SetupPath();
    /* 0x00 */ SonicteamUSA::System::PathCtrl m_path;
    /* 0x58 */ s32 m_route;      /* index into the route table */
    /* 0x5C */ float m_distance; /* along the current path */
    /* 0x60 */ float m_speed;
    /* 0x64 */ float m_targetSpeed;
    /* 0x68 */ float m_acceleration; /* see Update */
    /* 0x6C */ float field_0x6C; /* the constructor clears it; nothing else in the file uses it */
};
CHECK_SIZEOF(UnkPathControl_80322450, 0x70);

UnkPathControl_80322450::UnkPathControl_80322450()
    : m_route(0), m_distance(0.0f), m_speed(150.0f), m_targetSpeed(150.0f), m_acceleration(0.0f),
      field_0x6C(0.0f) {
    SetupPath();
}

bool UnkPathControl_80322450::SetupPath() {
    m_targetSpeed = 150.0f;
    m_speed = 150.0f;
    m_acceleration = 0.0f;

    const UnkRailRoute_8054B8E0 &route = s_routes[m_route];
    s32 pathNo = route.field_0x8 == 1 ? route.field_0x4 : route.field_0x0;

    if (fn_8001C7B4(fn_8001B494(), &m_path, pathNo)) {
        m_distance = 0.0f;
        fn_80018304(&m_path);
        return true;
    }
    return false;
}

bool UnkPathControl_80322450::Update(float dt, boost::weak_ptr<CharInfo> charInfo) {
    if (fn_80169CD0()) {
        return false;
    }

    if (boost::shared_ptr<CharInfo> info = charInfo.lock()) {
        float step = m_speed * dt + m_acceleration * dt * dt * 0.5f;

        if (m_targetSpeed > m_speed) {
            m_speed += m_acceleration * dt;
            if (m_speed > m_targetSpeed) {
                m_speed = m_targetSpeed;
                step = m_speed * dt;
            }
        } else if (m_targetSpeed < m_speed) {
            m_speed -= m_acceleration * dt;
            if (m_speed < m_targetSpeed) {
                m_speed = m_targetSpeed;
                step = m_speed * dt;
            }
        }
        m_distance += step;
        fn_80018304(&m_path);
        return fn_8001C91C(fn_8001B494(), &m_path, 0, &info->m_position, m_distance,
                           &info->m_rotation, 1);
    }
    return false;
}

bool UnkPathControl_80322450::NextRoute() {
    m_route++;
    if (m_route >= ROUTE_COUNT) {
        return false;
    }
    return SetupPath();
}

void UnkPathControl_80322450::SetRoute(int route, float fraction) {
    boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

    if (mission.get() != 0 && mission->Field0xCEquals(2)) {
        return;
    }

    m_route = route;
    SetupPath();
    if (0.0f <= fraction && fraction <= 1.0f) {
        m_distance = fraction * fn_800183D8(&m_path);
    }
}

namespace BKTankController {

Controller::Controller(Task *parent, boost::shared_ptr<PathObject> object) : Task(parent) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x92, 4);
    fn_801DCD8C(fn_800DDF20(), m_charInfo);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Controller::OnCommand, this, _1));
    m_object = object;
    m_state = STATE_LOADING;
    m_waitTime = 0.0f;
    m_pathControl = 0;
    m_flags.Clear();
}

Controller::~Controller() {
    fn_801DCCF4(fn_800DDF20(), m_charInfo);

    if (m_pathControl != 0) {
        delete m_pathControl;
        m_pathControl = 0;
    }

    if (m_object.get() != 0) {
        m_object->UnkSlot3();
        m_object.reset();
    }
}

void Controller::Update(float dt) {
    m_waitTime -= dt;
    if (m_waitTime < 0.0f) {
        m_waitTime = 0.0f;
    }

    if (lbl_8057D6A0.field_0xA8.field_0x20) {
        return;
    }

    switch (m_state) {
    case STATE_LOADING:
        if (fn_80012408(fn_800122D4(), 1) == 3 && fn_80177CF8() == 1) {
            m_object->Initialize();
            m_state = STATE_MOVING;
            m_pathControl = new UnkPathControl_80322450();

            boost::weak_ptr<CharInfo> charInfo = m_object->GetCharInfo();

            m_pathControl->Update(dt, charInfo);
            m_object->UpdateModelMatrix();
        }
        break;
    case STATE_MOVING: {
        if (m_object.get() == 0 || m_waitTime > 0.0f) {
            break;
        }

        bool pathEnded = false;

        if (!m_flags.Test(0)) {
            boost::weak_ptr<CharInfo> charInfo = m_object->GetCharInfo();

            pathEnded = m_pathControl->Update(dt, charInfo);
        }
        m_object->UpdateModelMatrix();
        m_object->Update(dt);

        if (pathEnded && !m_pathControl->NextRoute()) {
            boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

            if (mission.get() != 0) {
                fn_8016B270(mission.get());
            }
            m_state = STATE_DONE;
        }

        if (m_object->IsFinished()) {
            m_state = STATE_FINISHED;
        }
        break;
    }
    case STATE_FINISHED:
        if (m_object.get() != 0) {
            m_object->Update(dt);
        }
        {
            boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

            if (mission.get() != 0) {
                mission->fn_8016B224(1, 0);
            }
            m_state = STATE_DONE;
        }
        break;
    case STATE_DONE:
        if (m_object.get() != 0) {
            m_object->Update(dt);
        }
        break;
    }
}

void Controller::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x301: {
        GadgetCommand::MoveStartCommand *moveStart =
            dynamic_cast<GadgetCommand::MoveStartCommand *>(&command);

        if (moveStart != 0) {
            m_waitTime = 0.0f;
            m_flags.Reset(0);
        }
        break;
    }
    case 0x302: {
        GadgetCommand::MoveEndCommand *moveEnd =
            dynamic_cast<GadgetCommand::MoveEndCommand *>(&command);

        if (moveEnd != 0) {
            m_waitTime = moveEnd->m_waitTime;
            if (m_waitTime <= 0.0f) {
                m_flags.Set(0);
            }
        }
        break;
    }
    case 9: {
        SwitchCommand *switchCommand = dynamic_cast<SwitchCommand *>(&command);

        if (switchCommand != 0 && m_pathControl != 0) {
            s32 route = switchCommand->m_route;

            if (route >= 0 && route < ROUTE_COUNT) {
                s_routes[route].field_0x8 = s_routes[route].field_0x8 == 1 ? 0 : 1;
            }
        }
        break;
    }
    case 6: {
        AccelerateCommand *accelerate = dynamic_cast<AccelerateCommand *>(&command);

        if (accelerate != 0) {
            if (m_pathControl != 0) {
                m_pathControl->m_targetSpeed = accelerate->m_acceleration.x;
                float time = accelerate->m_acceleration.z;

                /* When time is <= 0, the speed becomes the target speed at once. Otherwise the
                 * acceleration becomes (target speed - speed) / time. */
                if (time <= 0.0f) {
                    m_pathControl->m_speed = m_pathControl->m_targetSpeed;
                    m_pathControl->m_acceleration = 0.0f;
                } else {
                    m_pathControl->m_acceleration =
                        (m_pathControl->m_targetSpeed - m_pathControl->m_speed) / time;
                }
            }
            m_flags.Reset(0);
        }
        break;
    }
    case 2: {
        boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

        if (mission.get() != 0 && mission->Field0xCEquals(2)) {
            return;
        }

        SetPositionCommand *setPosition = dynamic_cast<SetPositionCommand *>(&command);

        if (setPosition != 0) {
            int route = static_cast<int>(setPosition->m_position.x);
            float fraction = setPosition->m_position.y;

            if (m_pathControl != 0) {
                m_pathControl->m_targetSpeed = 150.0f;
                m_pathControl->m_speed = 150.0f;
                m_pathControl->m_acceleration = 0.0f;

                if (m_pathControl->m_route <= route) {
                    m_pathControl->SetRoute(route, fraction);
                } else {
                    route = m_pathControl->m_route;
                    if (route >= ROUTE_COUNT) {
                        route = ROUTE_COUNT - 1;
                        if (m_state == STATE_DONE) {
                            m_state = STATE_MOVING;
                        }
                    }
                    m_pathControl->SetRoute(route, 0.0f);
                }
            }
            m_flags.Reset(0);
        }
        break;
    }
    }
}

} // namespace BKTankController
