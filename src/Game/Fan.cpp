/* Fan::Fan (class name from RTTI; the gadget's descriptors are "FAN" and "FAN STAGE", which differ
 * in the first argument the constructor passes to fn_8015F800 with FAN.BIN): a fan with a cylinder
 * or box probe. While the fan runs, the character infos its probe touches are pushed along the
 * fan's axis by the parameter "power" (for an upward fan, less the higher they are above it), and,
 * when fn_800B7938 is false for the character info nearest to the fan, every character info its
 * probe touches whose word at 0x50 is 1 is put in a stream (Player::StreamCommand; m_target keeps
 * the last one) that UpdateBody ends again (behaviour inferred). File name inferred from
 * the namespace. Method names marked "(name inferred)" are not original. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first).
 *
 * The probe's collision callback is a boost::function object built with
 * boost::bind(&Fan::OnCollision, this, _1); its manager, invoker and copy functions come from the
 * boost headers.
 *
 * NonMatching (see configure.py): UpdateBody, CreateProbe and IsInRange differ. */
#include <cmath>
#include "Game/Fan.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliBase.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkCharInfo_800999C0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace Fan {

static const char *s_modelFileName = "FAN.BIN";
const char *Fan::s_name = "Fan";

/* What the hit callbacks receive (placeholder name). */
struct UnkHitContext_80103098 {
    UnkHitContext_80103098() : m_params(0), m_hit(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::weak_ptr<CharInfo> m_target; /* the character info the callback pushed */
    /* 0x10 */ FanParams *m_params;
    /* 0x14 */ bool m_hit; /* set when the callback pushed a character info */
};
CHECK_SIZEOF(UnkHitContext_80103098, 0x18);

/* The hit callback while the fan runs: pushes the character info along the fan's axis by the
 * parameter "power"; for an upward fan the push is scaled by (top - height) / top, where height is
 * the character's height above the fan (at least 0) and top is twice "Z" plus 5 times the cosine
 * of the float at 0x98 of lbl_8057D6A0 (behaviour inferred). */
static void fn_80103D44(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80103098 *context = static_cast<UnkHitContext_80103098 *>(arg);
    boost::shared_ptr<GadgetCharInfo> charInfo(context->m_charInfo.lock());

    if (charInfo.get() == 0) {
        return;
    }

    Fn8040Vector3 acceleration(0.0f, 1.0f, 0.0f);
    Fn8040Matrix34 matrix;
    Fn8040Vector3 rotated;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, charInfo->m_rotation.y);
    fn_8040B884(&matrix, charInfo->m_rotation.x);
    fn_8040B80C(&matrix, charInfo->m_rotation.z);
    fn_8040B67C(&rotated, &matrix, &acceleration);
    fn_800091D0(&acceleration, &rotated);
    fn_8040CE38(&acceleration, context->m_params->power);

    if (context->m_params->type == 0) {
        float above = hit->m_position.y - charInfo->m_position.y;
        if (above < 0.0f) {
            above = 0.0f;
        }
        float top = 2.0f * context->m_params->z + 5.0f * std::cos(lbl_8057D6A0.field_0x98);
        fn_8040CE38(&acceleration, (top - above) / top);
    }

    hit->fn_801DC980(new AccelerateCommand(acceleration, 4));
    context->m_hit = true;
    context->m_target = hit;
}

/* The hit callback that starts a character's stream: clears the velocity of a character info
 * whose word at 0x50 is 1 and passes it a Player::StreamCommand (behaviour inferred). */
static void fn_80103C4C(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80103098 *context = static_cast<UnkHitContext_80103098 *>(arg);

    if (context != 0 && hit->field_0x50 == 1) {
        hit->fn_801DC980(new ClrVelocityCommand(4));
        hit->fn_801DC980(new Player::StreamCommand(true, 4));
        context->m_hit = true;
        context->m_target = hit;
    }
}

Fan::Fan(Task *parent, UnkGadgetOwner_800C9CD0 *owner, int modelArg) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x10, 4);
    CopyFromOwner(owner);

    m_model = fn_8015F800(modelArg, s_modelFileName);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80160A84(0, 1);
    m_motion = m_model->fn_801602B4(0);

    CreateProbe();
    UpdateModelMatrix();

    if (m_params.disp == 0) {
        m_model->fn_80162790();
    }
}

Fan::~Fan() {
    if (!m_effect.expired()) {
        if (boost::shared_ptr<Effect::System::EffectElement> effect = m_effect.lock()) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }

    boost::shared_ptr<CharInfo> target(m_target.lock());

    if (target.get() != 0 && fn_800B7938(target)) {
        target->fn_801DC980(new Player::StreamCommand(false, 4));
    }
}

void Fan::UpdateBody(float dt) {
    m_state.Reset(0);

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }

    UpdateRunning(dt);

    switch (m_step) {
    case 0:
        m_step = 1;
        break;
    case 1: {
        boost::shared_ptr<CharInfo> target(m_target.lock());

        if (target.get() != 0 && fn_800B7938(target)) {
            bool release = true;

            if (m_params.type == 0) {
                if (IsInRange(&target->m_position, &m_charInfo->m_position)) {
                    release = false;

                    if (!m_state.Test(0)) {
                        Player::PlayerBase *player =
                            dynamic_cast<Player::PlayerBase *>(target.get());

                        if (player != 0) {
                            Fn8040Vector3 acceleration;
                            fn_8040CDA8(&acceleration, &player->field_0xE8, -30.0f);
                            player->fn_801DC980(new AccelerateCommand(acceleration, 4));
                        }
                    }
                }

                if (!m_state.Test(0) && fn_800B7AD0(target)) {
                    release = true;
                }
            } else if (m_params.type == 1) {
                if (m_state.Test(0)) {
                    release = false;
                }
            }

            if (release) {
                target->fn_801DC980(new Player::StreamCommand(false, 4));
                m_target.reset();
            }
        }
        break;
    }
    case 2:
        /* Matching workaround: an empty case label whose body is unknown. The original's dispatch
         * ends in an unreachable branch (UpdateBody+0x7C, 0x80103760), which is what this empty arm
         * compiles to. Measured with the original compiler: case 2 gives the original's dispatch
         * exactly (UpdateBody 140 words); no arm or an empty default drops the branch (139 words);
         * case 3 gives a different compare sequence (141 words). The gadget never stores 2.
         * Revisit when the third step value's body is found. */
        break;
    }

    UpdateModelMatrix();
}

void Fan::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 form = m_params.form;
    s32 disp = m_params.disp;

    CopyFromOwner(owner);
    UpdateModelMatrix();

    if (form != m_params.form) {
        m_handle.reset();
        CreateProbe();
    }

    if (disp != m_params.disp) {
        if (m_params.disp != 0) {
            m_model->fn_8016271C();
        } else {
            m_model->fn_80162790();
        }
    }

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }

    if (m_handle.get() != 0) {
        m_handle->m_probe->SetSize(m_params.x, -2, m_params.y, m_params.z);

        switch (m_params.form) {
        case 1: {
            Fn8040Vector3 offset(0.0f, m_params.y, 0.0f);
            m_handle->m_probe->UnkSlot17(&offset, -2);
            break;
        }
        case 0: {
            Fn8040Vector3 offset(0.0f, m_params.z, 0.0f);
            m_handle->m_probe->UnkSlot17(&offset, -2);
            break;
        }
        }
    }
}

void Fan::fn_800CBC1C() {
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }
}

void Fan::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_step = 0;
    m_timer = 0.0f;
    m_speedStep = 0.0f;
    m_speed = 1.0f;
    m_state.Clear();
    m_state.Set(2);

    if (m_params.secActive > 0.0f && m_params.secNegative > 0.0f) {
        m_state.Set(3);
        m_timer = m_params.secActive;
    }

    if (m_params.type == 0) {
        m_charInfo->m_rotation.x = 0.0f;
    } else {
        m_charInfo->m_rotation.x = 90.0f * (lbl_805FAD38 / 180.0f);
    }

    if (m_params.type == 1 && lbl_8057D6A0.field_0xA8 == 0x1F5) {
        m_state.Set(4);
    }
}

void Fan::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        m_model->fn_801633DC(&matrix);

        if (!m_effect.expired()) {
            if (boost::shared_ptr<Effect::System::EffectElement> effect = m_effect.lock()) {
                fn_80425E68(effect.get(), &matrix);
            }
        }
    }
}

void Fan::OnCollision(ColliDetectArgs &args) {
    if (m_state.Test(2)) {
        boost::shared_ptr<CharInfo> target(fn_800A8A7C(fn_80074194(), &m_charInfo->m_position));

        if (target.get() != 0 && !fn_800B7938(target)) {
            UnkHitContext_80103098 context;

            context.m_charInfo = m_charInfo;
            context.m_params = &m_params;
            fn_8015AA84(&args, fn_80103C4C, &context);

            if (context.m_hit) {
                m_target = context.m_target;
            }
        }
    }

    if (m_state.Test(2)) {
        UnkHitContext_80103098 context;

        context.m_charInfo = m_charInfo;
        context.m_params = &m_params;
        fn_8015AA84(&args, fn_80103D44, &context);

        if (context.m_hit) {
            m_state.Set(0);
        }
    }
}

void Fan::CreateProbe() {
    Fn8040Vector3 halfExtentsTemporary(1.0f, 1.0f, 1.0f);
    Fn8040Vector3 halfExtents;

    fn_800091EC(&halfExtents, &halfExtentsTemporary);

    boost::shared_ptr<CharaColliBase> probe;

    switch (m_params.form) {
    case 1:
        probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), halfExtents,
                            0x40100, 10, 0, &lbl_805E07A0, &lbl_805E07B8, 0);
        break;
    case 0: {
        /* The original stores all three components twice: zero, then the angles. */
        Fn8040Vector3 rotation(0.0f, 0.0f, 0.0f);

        rotation.Set(90.0f * (lbl_805FAD38 / 180.0f), 0.0f, 0.0f);
        probe = fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 1.0f, 1.0f,
                            0x40100, 10, 0, &lbl_805E07A0, &rotation, 0);
        break;
    }
    }

    m_handle = fn_8015B8C4(probe);
    m_handle->field_0x0.fn_800F5980(ColliDetectCallback(boost::bind(&Fan::OnCollision, this, _1)));

    m_handle->m_probe->SetSize(m_params.x, -2, m_params.y, m_params.z);

    switch (m_params.form) {
    case 1: {
        Fn8040Vector3 offset(0.0f, m_params.y, 0.0f);
        m_handle->m_probe->UnkSlot17(&offset, -2);
        break;
    }
    case 0: {
        Fn8040Vector3 offset(0.0f, m_params.z, 0.0f);
        m_handle->m_probe->UnkSlot17(&offset, -2);
        break;
    }
    }

    fn_8015BDC4(m_handle.get(), 1, 2);
}

void Fan::UpdateRunning(float dt) {
    if (m_params.id >= 0) {
        if (fn_8016904C(fn_800CB69C(), m_params.id) == 1) {
            m_state.Set(2);
        } else {
            m_state.Reset(2);
        }
    }

    if (m_state.Test(2)) {
        if (m_effect.expired()) {
            Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);

            m_effect = fn_8015D7DC(&m_charInfo->m_position, &direction, 0xD9);
        }

        if (!m_motion.expired()) {
            m_speedStep += 0.01f;
            m_speed += m_speedStep;
            if (m_speed > 1.0f) {
                m_speed = 1.0f;
            }

            if (boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock()) {
                stMtnRequestFrameSpeed(motion.get(), 0, m_speed);
            }

            if (m_state.Test(4)) {
                fn_801D53C4(fn_80053164(), 0x101A, 0, this, 2, 0, 2);
            } else {
                fn_801D53C4(fn_80053164(), 0x101A, &m_charInfo->m_position, this, 3, 0, 5);
            }
        }
    } else {
        if (!m_effect.expired()) {
            if (boost::shared_ptr<Effect::System::EffectElement> effect = m_effect.lock()) {
                effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            }

            m_effect.reset();
        }

        if (!m_motion.expired()) {
            m_speedStep += 0.01f;
            m_speed -= m_speedStep;
            if (m_speed < 0.0f) {
                m_speed = 0.0f;
            }

            if (boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock()) {
                stMtnRequestFrameSpeed(motion.get(), 0, m_speed);
            }
        }
    }

    if (m_state.Test(3)) {
        m_timer -= dt;

        if (m_timer <= 0.0f) {
            if (m_state.Test(2)) {
                m_state.Reset(2);
                m_timer = m_params.secNegative;
                m_speedStep = 0.0f;
            } else {
                m_state.Set(2);
                m_timer = m_params.secActive;
                m_speedStep = 0.0f;
            }
        }
    }
}

bool Fan::IsInRange(Fn8040Vector3 *position, Fn8040Vector3 *center) {
    bool inRange = false;

    switch (m_params.form) {
    case 1: {
        Fn8040Vector3 offset(position->x, 0.0f, position->z);
        Fn8040Vector3 flatCenter(center->x, 0.0f, center->z);
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 inverse;
        Fn8040Vector3 local;

        fn_8040CE04(&offset, &flatCenter);
        fn_8040B6F8(&matrix);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040BEF8(&inverse, &matrix);
        fn_8040C148(&matrix, &inverse);
        fn_8040B67C(&local, &matrix, &offset);
        fn_800091D0(&offset, &local);

        float halfWidth = 7.5f + m_params.x;
        float halfDepth = 7.5f + m_params.z;

        if (-halfWidth <= offset.x && offset.x <= halfWidth && -halfDepth <= offset.z &&
            offset.z <= halfDepth) {
            inRange = true;
        }
        break;
    }
    case 0: {
        Fn8040Vector3 flatPosition(position->x, 0.0f, position->z);
        Fn8040Vector3 flatCenter(center->x, 0.0f, center->z);
        Fn8040Vector3 offset;

        fn_8040CD74(&offset, &flatPosition, &flatCenter);

        if (fn_8040CF50(&offset) <= 7.5f + m_params.x) {
            inRange = true;
        }
        break;
    }
    }

    return inRange;
}

/* The names of the values of the "type", "form" and "Id" parameters, and the labels of "X", "Y"
 * and "Z" for each form. */
static const char *s_typeLabels[2] = {"type : UpperWay", "type : SideWay"};
static const char *s_formLabels[2] = {"form : Cylinder", "form : Box"};
static const char *s_xLabels[2] = {"radius", "width"};
static const char *s_yLabels[2] = {"Not in use", "height"};
static const char *s_zLabels[2] = {"height", "depth"};
static const char *s_idLabels[2] = {"Id -> Not Available", "Id -> Available"};

/* The label callback of the descriptor's "type", "form", "X", "Y", "Z" and "Id" entries: the
 * labels of "X", "Y" and "Z" follow the form last shown (behaviour inferred). */
static const char *fn_80102328(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_form = 0;

    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    } else if (cursor->m_index == 1) {
        s_form = cursor->m_value;
        return s_formLabels[s_form];
    } else if (cursor->m_index == 2) {
        return s_xLabels[s_form];
    } else if (cursor->m_index == 3) {
        return s_yLabels[s_form];
    } else if (cursor->m_index == 4) {
        return s_zLabels[s_form];
    } else if (cursor->m_index == 9) {
        if (cursor->m_value >= 0) {
            return s_idLabels[1];
        }
        return s_idLabels[0];
    }

    return 0;
}

} // namespace Fan

extern "C" void fn_80102300(void) {
    fn_8006B474("FAN.BIN", 1);
}

extern "C" void fn_801022FC(void) {}

void Fan::Fan::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Fan(parent, owner, 0);
}

extern "C" void fn_80102278(void) {
    fn_8006B474("FAN.BIN", 1);
}

extern "C" void fn_80102274(void) {}

void Fan::Fan::CreateStage(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Fan(parent, owner, 1);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, Fan::fn_80102328, 0},
    {1, "form", 0, 0, 1, 0.0f, 0.0f, 0.0f, Fan::fn_80102328, 0},
    {4, "X", 0, 0, 0, 0.0f, 0.0f, 65536.0f, Fan::fn_80102328, 0},
    {4, "Y", 0, 0, 0, 0.0f, 0.0f, 65536.0f, Fan::fn_80102328, 0},
    {4, "Z", 0, 0, 0, 0.0f, 0.0f, 65536.0f, Fan::fn_80102328, 0},
    {4, "power", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "sec_active", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "sec_negative", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "disp", 1, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "Id", 0xFFFFFFFF, 0xFFFFFFFF, 0xFF, 0.0f, 0.0f, 0.0f, Fan::fn_80102328, 0},
    {0},
};

GadgetTypeInfo lbl_80525E24 = {
    "FAN", fn_80102300, fn_801022FC, Fan::Fan::Create, 0, 0, 0x1A, 1, 0, 10, s_paramInfo,
};

GadgetTypeInfo lbl_80525E48 = {
    "FAN STAGE", fn_80102278, fn_80102274, Fan::Fan::CreateStage, 0, 0, 0x2594,
    1,           0,           10,          s_paramInfo,
};
