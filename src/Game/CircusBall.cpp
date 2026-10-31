/* CircusBall::CircusBall (class name from RTTI; the gadget's descriptor is "CIRCUS_BALL"): a
 * ball of the circus stage that, when a character touches it, takes the character's control for
 * a time and launches it along a direction given by the gadget's angle with a speed that the
 * gadget's level selects (behaviour inferred). The parameter "type" tells whether the ball starts
 * on its own (normal) or waits for a stage switch (key). File name inferred from the class.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_801E72DC.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkObject_801695B8;
class UnkObject_80169BB0;

extern "C" {
/* Creates the object that the ball keeps at 0x40 (placeholder names; behaviour inferred from the
 * calls): fn_801697F0 advances it each frame, fn_801697CC tells whether it has already dealt with
 * the character info, and fn_8016952C releases it. */
boost::shared_ptr<UnkObject_801695B8> fn_801695B8(void);
void fn_801697F0(UnkObject_801695B8 *object);
bool fn_801697CC(UnkObject_801695B8 *object, const boost::weak_ptr<CharInfo> &target);
void fn_8016952C(boost::shared_ptr<UnkObject_801695B8> &object);
}

/* The gadget parameters in the order of the descriptor's table below. */
struct CircusBallParams {
    CircusBallParams() : type(0), level(0.0f), noControlTime(0.0f), angle(0.0f) {}

    /* 0x0 */ s32 type;            /* 0 normal, 1 key */
    /* 0x4 */ float level;         /* selects the launch speed */
    /* 0x8 */ float noControlTime; /* seconds the character cannot be controlled */
    /* 0xC */ float angle;         /* degrees */
};
CHECK_SIZEOF(CircusBallParams, 0x10);

/* What the hit callback needs to know about the ball (placeholder name): the handle that remembers
 * the characters already dealt with, the ball's character info, its parameters and its position,
 * and, as output, whether anybody was hit. */
struct UnkHitContext_8012F724 {
    UnkHitContext_8012F724() : m_params(0), m_position(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const CircusBallParams *m_params;
    /* 0x14 */ const Fn8040Vector3 *m_position;
    /* 0x18 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_8012F724, 0x1C);

/* Calls callback with context for each character info in the collision results (behaviour
 * inferred). It passes the context to the callback without reading it, so this prototype gives the
 * context this unit's type. */
extern "C" void fn_8015B004(ColliDetectArgs *args,
                            void (*callback)(boost::shared_ptr<CharInfo> hit,
                                             UnkHitContext_8012F724 *context),
                            UnkHitContext_8012F724 *context);

/* Creates an object of the given kind for target and returns a weak pointer to it (behaviour
 * inferred; the target is not used by the body, and the hit callback discards the result). */
extern "C" boost::weak_ptr<UnkObject_80169BB0> fn_80169BB0(const boost::weak_ptr<CharInfo> &target,
                                                           int kind);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {150.0f,  300.0f,  450.0f,  600.0f,  750.0f,  900.0f,
                                         1050.0f, 1200.0f, 1350.0f, 1500.0f, 1650.0f, 1800.0f,
                                         1950.0f, 2100.0f, 2250.0f, 2400.0f};

/* The callback that fn_8015B004 calls for each character info in the collision results: sends a
 * character that has not been dealt with the commands that take its control and launch it. */
static void fn_80130364(boost::shared_ptr<CharInfo> hit, UnkHitContext_8012F724 *context) {
    boost::shared_ptr<GadgetCharInfo> ballInfo = context->m_charInfo.lock();

    if (ballInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new Player::IgnoreInputCommand(0, context->m_params->noControlTime));
    hit->fn_801DC980(new ClrVelocityCommand(4));
    hit->fn_801DC980(new SetPositionCommand(context->m_position, 4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        speed += (s_launchSpeeds[index + 1] - speed) * (level - index);
    }

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, ballInfo->m_rotation.y);
    fn_8040B884(&matrix, -context->m_params->angle * (lbl_805FAD38 / 180.0f));

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);
    Fn8040Vector3 directionTemporary;

    fn_8040B67C(&directionTemporary, &matrix, &direction);
    fn_800091D0(&direction, &directionTemporary);
    fn_8040CE38(&direction, speed);

    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
    hit->fn_801DC980(new Player::LaunchCommand(0, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 5);

    fn_801D560C(fn_80053164(), 0x742E, 0, 0);
    context->m_hit = true;
}

namespace CircusBall {

class CircusBall : public Gadget {
  public:
    CircusBall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CircusBall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation, parameters and key id from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info and the object at 0x48 at the model's origin (name
     * inferred). */
    void UpdateModelMatrix();
    /* Attaches the character info to the model's "ball" node, installs the collision callback and
     * creates the object at 0x48 (name inferred). */
    void StartCollision();
    /* The model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x48 */ boost::shared_ptr<UnkObject_801E72DC> m_object;
    /* 0x50 */ UnkResource_80012700 *m_resource;
    /* 0x54 */ CircusBallParams m_params;
    /* 0x64 */ Fn8040Vector3 m_position;
    /* 0x70 */ s32 m_state; /* 0 not started, 1 waiting for the switch, 2 active (inferred) */
    /* 0x74 */ s32 m_key;   /* the id of the stage switch the ball waits for (inferred) */
};
CHECK_SIZEOF(CircusBall, 0x78);

const char *CircusBall::s_name = "CircusBall";

CircusBall::CircusBall(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x41, 4, "CircusBall");
    ReadOwner(m_owner);
    m_resource = fn_80012700(fn_800122D4(), 1, "CIRCUSBALL.BIN");
    m_model = fn_8015F800(1, "CIRCUSBALL.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    fn_8040CE60(&m_position);
    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

CircusBall::~CircusBall() {
    fn_8016952C(m_handle);
    fn_801E7108(m_object);
}

void CircusBall::StartCollision() {
    m_model->fn_80161F08("ball", boost::weak_ptr<CharInfo>(m_charInfo));
    /* The model's collision callback is at 0x4 of GadgetModelInfo (inferred from the assignment
     * the original makes to that address). */
    *reinterpret_cast<ColliDetectCallback *>(m_model->field_0x4) =
        ColliDetectCallback(boost::bind(&CircusBall::OnCollision, this, _1));
    m_object = fn_801E72DC(0x400);
}

void CircusBall::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        /* A normal ball is active at once, a key ball waits for its stage switch. */
        if (m_params.type == 0) {
            m_state = 2;
            StartCollision();
        } else {
            m_state = 1;
            m_model->fn_80162790();
        }
        break;

    case 1:
        if (fn_8016904C(fn_800CB69C(), m_key) == 1) {
            m_model->fn_8016271C();
            m_model->fn_80160A84(0, 1);
            m_state = 2;
            StartCollision();

            UnkResourceRecord_80208848 *record =
                fn_80208888(m_resource, fn_801626D8(m_model.get()));

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }
        }
        break;

    case 2:
        break;
    }

    UpdateModelMatrix();
}

void CircusBall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from the ball along its launch direction (behaviour inferred). */
        Fn8040Vector3 start(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 1000000.0f, 0.0f);
        Fn8040Vector3 startTemporary;
        Fn8040Vector3 endTemporary;
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, -m_params.angle * (lbl_805FAD38 / 180.0f));
        fn_8040B67C(&startTemporary, &matrix, &start);
        fn_800091D0(&start, &startTemporary);
        fn_8040B67C(&endTemporary, &matrix, &end);
        fn_800091D0(&end, &endTemporary);
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void CircusBall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_key);
    m_state = 0;
}

void CircusBall::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);

        Fn8040Vector3 origin(0.0f, 0.0f, 0.0f);
        Fn8040Vector3 transformed;

        fn_8040B67C(&transformed, &matrix, &origin);
        fn_800091D0(&m_position, &transformed);

        if (m_object.get() != 0) {
            fn_800CCE38(m_object.get(), &m_position);
        }
    }
}

void CircusBall::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8012F724 context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    context.m_position = &m_position;
    fn_8015B004(&args, fn_80130364, &context);

    if (context.m_hit && m_model.get() != 0) {
        m_model->fn_80160A84(0, 2);
    }
}

} // namespace CircusBall

extern "C" void fn_8012F6F8(void) {
    fn_8006B474("CIRCUSBALL.BIN", 1);
}

extern "C" void fn_8012F6F4(void) {}

void CircusBall::CircusBall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new CircusBall(parent, owner);
}

/* The names of the values of the "type" parameter. */
static const char *s_typeLabels[2] = {"type : normal", "type : key"};

/* The label callback of the descriptor's first parameter (behaviour inferred). */
static const char *fn_8012F674(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_8012F674, 0},
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "angle", 0, 0, 0, 0.0f, 0.0f, 360.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528744 = {
    "CIRCUS_BALL", fn_8012F6F8, fn_8012F6F4, CircusBall::CircusBall::Create, 0, 0, 0xC80, 1, 0, 10,
    s_paramInfo,
};
