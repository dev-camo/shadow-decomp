/* The ElecBar gadget and its bars (classes and behaviour described in Game/ElecBar.h). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first), so the bars' methods, defined after the descriptor, come first
 * in the file's code.
 *
 * Each bar's probe callback and command handler are boost::function objects built with
 * boost::bind(&ElecBarSub::OnCollision, this, _1) and boost::bind(&ElecBarSub::OnCommand, this,
 * _1); their managers, invokers and copy functions come from the boost headers. */
#include "Game/ElecBar.h"
#include "Game/CharaColliBase.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/Task.h"
#include "Game/UnkCharInfo_800999C0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

const char *ElecBar::s_name = "ElecBar";

ElecBar::ElecBar(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_rotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    ReadOwner(owner);
    m_barCount = m_params.numberOfBars;
    CreateBars();
}

ElecBar::~ElecBar() {
    DestroyBars();
}

void ElecBar::UpdateBody(float dt) {
    std::set<ElecBarSub *>::iterator it;

    for (it = m_bars.begin(); it != m_bars.end(); it++) {
        (*it)->Update(dt);
    }
}

void ElecBar::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);

    std::set<ElecBarSub *>::iterator it;

    for (it = m_bars.begin(); it != m_bars.end(); it++) {
        (*it)->CopyFromParent();
    }

    if (m_barCount != m_params.numberOfBars) {
        DestroyBars();
        m_barCount = m_params.numberOfBars;
        CreateBars();
    }
}

/* Updates every bar, then draws the box the bars move in (behaviour inferred). */
void ElecBar::fn_800CBC1C(float dt) {
    std::set<ElecBarSub *>::iterator it;

    for (it = m_bars.begin(); it != m_bars.end(); it++) {
        (*it)->Update(dt);
    }

    Fn8040Vector3 center = m_position;
    Fn8040Vector3 halfSize;

    halfSize.x = 0.5f * (10.0f * m_params.scale);
    if (m_params.type == 0) {
        center.y += 0.5f * m_params.offset;
        halfSize.y = 0.5f * m_params.offset;
        halfSize.z = 2.0f;
    } else {
        center.z += 0.5f * -m_params.offset;
        halfSize.y = 2.0f;
        halfSize.z = 0.5f * m_params.offset;
    }

    fn_802E0E5C(&center, &halfSize, &m_rotation, 0);
}

void ElecBar::CreateBars() {
    for (int i = 0; i < m_barCount; i++) {
        ElecBarSub *bar = new ElecBarSub(this);

        if (bar != 0) {
            bar->m_phase = i * (1.0f / m_params.numberOfBars);
            m_bars.insert(bar);
        }
    }
}

void ElecBar::DestroyBars() {
    std::set<ElecBarSub *>::iterator it;

    for (it = m_bars.begin(); it != m_bars.end(); it++) {
        delete *it;
    }

    m_bars.clear();
}

extern "C" void fn_80152434(void) {
    fn_8006B474("BAR.BIN", 1);
}

extern "C" void fn_80152430(void) {}

void ElecBar::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecBar(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static const char *s_typeLabels[2] = {"type< Vertical   >", "type< Horizontal >"};

/* The label callback of the descriptor's first parameter (behaviour inferred). */
static const char *fn_801523B0(UnkParamCursor_800CBC20 *cursor) {
    switch (cursor->m_index) {
    case 0:
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_801523B0, 0},
    {4, "scale", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "offset", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "moving second", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "hiding second", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "appearing ratio", 0, 0, 0, 0.1f, 0.0f, 1.0f, fn_800CBC20, 0},
    {4, "synchronize", 0, 0, 0, 0.0f, 0.0f, 1.0f, fn_800CBC20, 0},
    {1, "number of bars", 1, 1, 9, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052A6E8 = {
    "ELEC_BAR", fn_80152434, fn_80152430, ElecBar::Create, 0, 0, 0x7D3, 1, 0, 10, s_paramInfo,
};

/* What the hit callback receives (placeholder name). */
struct UnkHitContext_80150BA8 {
    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ boost::weak_ptr<UnkSegment_80020A20> m_line;
};
CHECK_SIZEOF(UnkHitContext_80150BA8, 0x10);

/* The bars' hit callback: sends a Player::TouchLineCommand with the bar's character info and line
 * to a player that is not in the state fn_800B7828 tests, unless the player's segment (the
 * vectors at 0xC of fn_80078CEC's results for 0 and 1, or the first point alone when they are less
 * than 0.1 apart) is more than 12 from the bar's line (squared distance above 144). When the bar's
 * line has expired, the distance test is skipped and the command is still sent (behaviour
 * inferred). */
static void fn_80152164(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80150BA8 *context = static_cast<UnkHitContext_80150BA8 *>(arg);

    if (hit.get() != 0 && context != 0 && !fn_800B7828(hit)) {
        Player::PlayerBase *player = dynamic_cast<Player::PlayerBase *>(hit.get());

        if (player != 0) {
            boost::shared_ptr<UnkSegment_80020A20> line = context->m_line.lock();

            if (line.get() != 0) {
                float squaredDistance;

                Fn8040Vector3 first = fn_80078CEC(player, 0)->field_0xC;
                Fn8040Vector3 second = fn_80078CEC(player, 1)->field_0xC;
                Fn8040Vector3 span = first - second;
                if (fn_8040CF50(&span) < 0.1f) {
                    squaredDistance = fn_8003C660(&first, line.get(), 0);
                } else {
                    UnkSegment_80020A20 segment(first, second);

                    squaredDistance = fn_8003C4C4(&segment, line.get(), 0, 0);
                }

                if (squaredDistance > 144.0f) {
                    return;
                }
            }

            hit->fn_801DC980(new Player::TouchLineCommand(
                boost::weak_ptr<CharInfo>(context->m_charInfo), context->m_line, 4));
        }
    }
}

ElecBarSub::ElecBarSub(ElecBar *parent) : m_parent(parent) {
    m_charInfo = fn_8015CB88(0x38, 4, "ElecBar");
    m_charInfo->m_position = m_parent->m_position;
    m_charInfo->m_rotation = m_parent->m_rotation;

    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ElecBarSub::OnCommand, this, _1));

    m_model = new GadgetModelInfo(1, "BAR.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_8016239C("mat1");
    m_model->fn_8016239C("mat2");

    UnkRotation3 rotation(0.0f, 0.5f * lbl_805FAD38, 0.0f);
    boost::shared_ptr<CharaColliBase> probe = fn_800C78D4(
        fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x40000, 10, 0, &lbl_805E07A0,
        rotation, 0, 0.5f * (10.0f * m_parent->m_params.scale), 2.0f, 0.5f);
    m_handle = fn_8015B8C4(probe);
    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&ElecBarSub::OnCollision, this, _1)));

    m_charInfo->m_rotation.x = m_charInfo->m_rotation.z = 0.0f;
    m_line = boost::shared_ptr<UnkSegment_80020A20>(new UnkSegment_80020A20);

    m_offset = 0.0f;
    m_state = 3;
    UpdateModelMatrix();
}

ElecBarSub::~ElecBarSub() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

/* Sets the four colour bytes of both material states to value. */
static inline void SetMaterialColors(UnkModelAtomicState *first, UnkModelAtomicState *second,
                                     u8 value) {
    first->color.field_0x0 = value;
    first->color.field_0x1 = value;
    first->color.field_0x2 = value;
    first->color.field_0x3 = value;
    second->color.field_0x0 = value;
    second->color.field_0x1 = value;
    second->color.field_0x2 = value;
    second->color.field_0x3 = value;
}

void ElecBarSub::Update(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    float period = m_parent->m_period;
    float shift = period * (m_parent->m_params.synchronize + m_phase);
    float time = lbl_8057D6A0.field_0x94 + shift;

    m_time = time - period * (int)(time / period);

    if (m_time < m_parent->m_fadeTime) {
        if (m_state == 2) {
            fn_801D54F4(fn_80053164(), 0x7239, &m_charInfo->m_position, 0, 1);
        }
        m_state = 0;

        UnkModelAtomicState *first = m_model->fn_801621F0("mat1");
        UnkModelAtomicState *second = m_model->fn_801621F0("mat2");

        m_offset = m_parent->m_speed * m_time;

        u8 value =
            255.0f * (m_offset / (m_parent->m_params.offset * m_parent->m_params.appearingRatio));

        SetMaterialColors(first, second, value);
    } else if (m_time < m_parent->m_params.movingSecond) {
        if (m_state != 1) {
            m_handle->ClearFlags(2);
            m_handle->field_0x0.fn_800F5980(
                GadgetColliDetectCallback(boost::bind(&ElecBarSub::OnCollision, this, _1)));

            UnkModelAtomicState *first = m_model->fn_801621F0("mat1");
            UnkModelAtomicState *second = m_model->fn_801621F0("mat2");

            SetMaterialColors(first, second, 0xFF);
        }
        m_state = 1;

        m_offset = m_parent->m_speed * m_time;

        float fadeLength = m_parent->m_params.offset * m_parent->m_params.appearingRatio;
        float remaining = fn_8000A980(m_parent->m_params.offset - m_offset);

        if (remaining < fadeLength) {
            UnkModelAtomicState *first = m_model->fn_801621F0("mat1");
            UnkModelAtomicState *second = m_model->fn_801621F0("mat2");

            u8 value = 255.0f * (remaining / fadeLength);

            SetMaterialColors(first, second, value);
        }
    } else {
        if (m_state != 2) {
            m_handle->SetFlags(2);

            boost::shared_ptr<CharInfo> rider = m_rider.lock();

            if (rider.get() != 0) {
                rider->fn_801DC980(new Player::StartActionCommand(4));
            }

            UnkModelAtomicState *first = m_model->fn_801621F0("mat1");
            UnkModelAtomicState *second = m_model->fn_801621F0("mat2");

            SetMaterialColors(first, second, 0);
        }
        m_state = 2;
    }

    UpdateModelMatrix();
    fn_8015BE9C(m_handle.get(), dt);
}

void ElecBarSub::CopyFromParent() {
    m_charInfo->m_position = m_parent->m_position;
    m_charInfo->m_rotation = m_parent->m_rotation;
}

void ElecBarSub::UpdateModelMatrix() {
    Fn8040Vector3 right(5.0f, 0.0f, 0.0f);
    Fn8040Vector3 left(-5.0f, 0.0f, 0.0f);
    Fn8040Vector3 offset;

    if (m_parent->m_params.type == 0) {
        Fn8040Vector3 up(0.0f, m_offset, 0.0f);

        offset = up;
    } else {
        Fn8040Vector3 back(0.0f, 0.0f, -m_offset);

        offset = back;
    }

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B59C(&matrix, &offset);

    Fn8040Vector3 inset(3.75f, 0.0f, 0.0f);
    const Fn8040Vector3 &start = right * m_parent->m_params.scale - inset;
    UnkSegment_80020A20 *line = m_line.get();
    line->start = matrix * start;

    Fn8040Vector3 endInset(3.75f, 0.0f, 0.0f);
    const Fn8040Vector3 &end = left * m_parent->m_params.scale + endInset;
    line = m_line.get();
    line->end = matrix * end;

    fn_8040B720(&matrix, m_parent->m_params.scale, 0.75f, 0.75f);
    if (m_model != 0) {
        m_model->fn_801633DC(&matrix);
    }

    m_handle->m_probe->UnkSlot17(&offset, -2);
}

void ElecBarSub::OnCommand(CharCommand &command) {
    if (command.m_type == 11) {
        RideCommand *ride = dynamic_cast<RideCommand *>(&command);

        if (ride != 0) {
            m_rider = ride->m_rider;
        }
    } else if (command.m_type == 12) {
        m_rider.reset();
    }
}

void ElecBarSub::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_80150BA8 context;

    context.m_charInfo = m_charInfo;
    context.m_line = m_line;
    fn_8015AA84(&args, fn_80152164, &context);
}
