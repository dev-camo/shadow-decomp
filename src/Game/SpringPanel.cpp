/* The SpringPanel gadget (behaviour in the class comment). File name inferred from the class.
 * Method names marked "(name inferred)" in the header are not original. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first).
 *
 * The command handler and the model's collision callback are boost::function objects built with
 * boost::bind; their managers, invokers and copy functions come from the boost headers. */
/* Matching workaround: the unit's .data holds the base lists of the type records of TNumEdit's
 * and CharaColliBase's derived classes, which these two headers emit through their stand-in
 * inline functions (see TNumEdit.h). TNumEdit.h comes before CharaColliBaseCharInfo.h, in the
 * order their names appear in the unit's .rodata. Revisit when the original inline functions or
 * their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/SpringPanel.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* What the hit callback needs to know about the panel (placeholder name): its character info,
 * its parameters and the position the character is placed at, and, as output, whether the
 * callback launched a character. */
struct UnkHitContext_801590DC {
    UnkHitContext_801590DC() : m_params(0), m_position(0), m_hit(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ const SpringPanelParams *m_params;
    /* 0x0C */ const Fn8040Vector3 *m_position;
    /* 0x10 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_801590DC, 0x14);

/* Launch speeds for the levels 0 to 15; the level in between is interpolated (name inferred). */
static const float s_launchSpeeds[16] = {150.0f,  300.0f,  450.0f,  600.0f,  750.0f,  900.0f,
                                         1050.0f, 1200.0f, 1350.0f, 1500.0f, 1650.0f, 1800.0f,
                                         1950.0f, 2100.0f, 2250.0f, 2400.0f};

/* The three colours that UpdateColor cycles through (names inferred). */
static UnkDebugColor_802E0E84 s_cycleColor0(0xFF, 0x33, 0x00, 0x00);
static UnkDebugColor_802E0E84 s_cycleColor1(0xFF, 0x7F, 0x00, 0x00);
static UnkDebugColor_802E0E84 s_cycleColor2(0xFF, 0x99, 0x33, 0x00);

/* The callback that fn_8015AA84 calls for each character info it takes from the hits. When the
 * panel's character info has expired it returns at once; otherwise it sends the character the
 * commands that take its control, place it, turn it and launch it, then sets m_hit. Each command
 * is allocated and passed to the character's fn_801DC980; a failed allocation passes a null
 * pointer, and m_hit is set all the same. The level is not range-checked: its integer part
 * (truncated toward zero) indexes the table, and the speed is interpolated towards the next entry
 * only when that index is less than 15 (a signed compare, so a negative index is interpolated
 * too). */
static void fn_8015A260(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_801590DC *context = static_cast<UnkHitContext_801590DC *>(arg);

    boost::shared_ptr<GadgetCharInfo> panelInfo = context->m_charInfo.lock();

    if (panelInfo.get() == 0) {
        return;
    }

    hit->fn_801DC980(new Player::IgnoreInputCommand(1, context->m_params->noControlTime));
    hit->fn_801DC980(new ClrVelocityCommand(4));
    hit->fn_801DC980(new SetPositionCommand(*context->m_position, 4));

    /* The character keeps its x and z rotations and takes the panel's y rotation. */
    UnkRotation3 angles = hit->m_rotation;

    angles.y = panelInfo->m_rotation.y;
    hit->fn_801DC980(new SetAngleCommand(angles, 2, 4));

    float level = context->m_params->level;
    int index = (int)level;
    float speed = s_launchSpeeds[index];

    if (index < 15) {
        /* The step to the next entry is formed in a variable of its own in two statements
         * (spelling inferred): the original keeps the entry and the step in one register while
         * the index is converted, and reuses one stack slot for its conversions, which the step
         * written inside the last expression does not give. */
        float step = s_launchSpeeds[index + 1];
        step -= speed;
        speed += step * (level - index);
    }

    /* The launch direction: the panel's y axis turned by its x and z rotations. */
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B884(&matrix, panelInfo->m_rotation.x);
    fn_8040B80C(&matrix, panelInfo->m_rotation.z);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

    direction = matrix * direction;
    fn_8040CE38(&direction, speed);

    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));
    hit->fn_801DC980(new Player::LaunchCommand(0, 4));

    /* Passes sound 0x7238 and the position the character was placed at to fn_801D54F4, and
     * kind 5 to fn_8034DF48, discarding its result (behaviour inferred). */
    fn_801D54F4(fn_80053164(), 0x7238, context->m_position, 0, 1);
    fn_8034DF48(5, -2);
    context->m_hit = true;
}

const char *SpringPanel::s_name = "SpringPanel";

SpringPanel::SpringPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_charInfo = fn_8015CB88(5, 4, "SpringPanel");
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SpringPanel::OnCommand, this, _1));

    m_model = new GadgetModelInfo(1, "SPRINGPANEL.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_8016239C("mat");
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&SpringPanel::OnCollision, this, _1));

    fn_8040CE60(&m_position);
    m_object = fn_801E72DC(0x400);

    ReadOwner(m_owner);
    UpdateModelMatrix();
}

SpringPanel::~SpringPanel() {
    fn_801E7108(m_object);

    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void SpringPanel::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    UpdateModelMatrix();
    UpdateColor();
}

void SpringPanel::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        /* Queues a debug line from (0, 10, 0) to (0, 1000000, 0) in the panel's frame: its
         * position, turned by its x and z rotations (behaviour inferred). */
        Fn8040Vector3 start(0.0f, 10.0f, 0.0f);
        Fn8040Vector3 end(0.0f, 1000000.0f, 0.0f);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        start = matrix * start;
        end = matrix * end;
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void SpringPanel::fn_800CBC1C(float) {
    UpdateColor();
}

void SpringPanel::UpdateColor() {
    UnkModelAtomicState *state = m_model->fn_801621F0("mat");
    float time = lbl_8057D6A0.field_0x94;
    float t = time - (int)time;

    if (t < 0.5f) {
        fn_80154C6C(state, t, s_cycleColor0, s_cycleColor1, 0.0f, 0.5f);
    } else if (t < 0.8333333f) {
        fn_80154C6C(state, t, s_cycleColor1, s_cycleColor2, 0.5f, 0.8333333f);
    } else if (t < 1.0f) {
        fn_80154C6C(state, t, s_cycleColor2, s_cycleColor0, 0.8333333f, 1.0f);
    }
}

void SpringPanel::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void SpringPanel::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);

        Fn8040Vector3 top(0.0f, 20.0f, 0.0f);

        m_position = matrix * top;

        if (m_object.get() != 0) {
            fn_800CCE38(m_object.get(), m_position);
        }
    }
}

void SpringPanel::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_801590DC context;

    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    context.m_position = &m_position;
    fn_8015AA84(&args, fn_8015A260, &context);

    if (context.m_hit && m_model != 0) {
        m_model->fn_80161308(1.0f);
    }
}

void SpringPanel::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x1A:
        if (m_object->TestFlags(1)) {
            m_object->ClearFlags(1);
        }
        break;

    case 0x1B:
        if (!m_object->TestFlags(1)) {
            m_object->SetFlags(1);
        }
        break;
    }
}

extern "C" void fn_80159018(void) {
    fn_8006B474("SPRINGPANEL.BIN", 1);
}

extern "C" void fn_80159014(void) {}

void SpringPanel::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SpringPanel(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "level", 0, 0, 0, 0.0f, 0.0f, 15.0f, fn_800CBC20, 0},
    {4, "nocontrol time (sec)", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052ADE8 = {
    "ELEC_SPRING", fn_80159018, fn_80159014, SpringPanel::Create, 0, 0, 0x7DE, 1, 0, 10,
    s_paramInfo,
};
