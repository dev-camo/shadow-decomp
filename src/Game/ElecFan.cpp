/* ElecFan (class name from RTTI; the gadget's descriptor is "ELEC_FAN"): a fan that blows the
 * nearest character info upwards while it stands inside a cylinder above the fan (the parameters
 * "height" and "radius") and the fan runs. A fan with a com id runs while that switch is on. Each
 * frame the fan runs it also starts an ElecFanDraw task, which draws a rising, fading clump at a
 * random point of the rim (behaviour inferred). File name inferred from the class. Method names
 * marked "(name inferred)" are not original. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first); the ElecFanDraw functions come first in the
 * original's .text, so they are defined last. */
#include <cmath>
#include "Game/ElecFan.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/Player.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80037A80.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_801F8040.h"
#include "Game/fn_8040_random.h"
#include "rw/rpworld.h"
#include "boost/bind.hpp"

const char *ElecFan::s_name = "ElecFan";

ElecFan::ElecFan(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CD90(0x10, 4);
    CopyFromOwner(m_owner);
}

ElecFan::~ElecFan() {
    boost::shared_ptr<CharInfo> target = m_target.lock();

    if (target.get() != 0 && fn_800B7938(target)) {
        target->fn_801DC980(new Player::StreamCommand(false, 4));
    }
}

void ElecFan::UpdateBody(float dt) {
    m_state.Reset(0);

    if (m_comId != 0) {
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state.Set(2);
        } else {
            m_state.Reset(2);
        }
    } else {
        m_state.Set(2);
    }

    if (m_state.Test(2)) {
        BlowCharacter();
    }

    boost::shared_ptr<CharInfo> target = m_target.lock();

    if (target.get() != 0 && fn_800B7938(target)) {
        Fn8040Vector3 offset = target->m_position - m_charInfo->m_position;

        if (offset.x * offset.x + offset.z * offset.z > m_params.radius * m_params.radius ||
            ((offset.y < 0.0f || m_params.height < offset.y) && fn_800B7AD0(target))) {
            target->fn_801DC980(new Player::StreamCommand(false, 4));
            m_target.reset();
        } else if (m_params.height < offset.y) {
            Player::PlayerBase *player = dynamic_cast<Player::PlayerBase *>(target.get());

            if (player != 0) {
                target->fn_801DC980(new AccelerateCommand(player->field_0xE8 * -30.0f, 4));
            }
        }
    }

    if (m_state.Test(2)) {
        float angle = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
        Fn8040Vector3 position;

        /* The original never sets position.y: the draw task's height is the character info's
         * height plus whatever the stack held there (kept as in the original). */
        fn_801F8040(&position.x, &position.z, angle, m_params.radius);
        AddVector(&position, m_charInfo->m_position);
        new ElecFanDraw(fn_801EB304(fn_8001C4F8(), 11), position, angle, m_params.radius,
                        0.5f * m_params.height);
    }
}

void ElecFan::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecFan::fn_800CBC1C(float) {
    Fn8040Vector3 offset;
    UnkRotation3 rotation;

    rotation.Set(0.5f * lbl_805FAD38, 0.0f, 0.0f);
    offset.Set(0.0f, 0.5f * m_params.height, 0.0f);
    const Fn8040Vector3 &center = m_charInfo->m_position + offset;
    fn_802E0C60(&center, &rotation, 0, 0.5f * m_params.height, m_params.radius);
}

void ElecFan::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9E60(owner, &m_comId);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void ElecFan::BlowCharacter() {
    if (m_state.Test(2)) {
        boost::shared_ptr<CharInfo> target(fn_800A8A7C(fn_80074194(), &m_charInfo->m_position));

        if (target.get() != 0) {
            float dy = target->m_position.y - m_charInfo->m_position.y;

            if (0.0f <= dy && dy < m_params.height) {
                float dx = target->m_position.x - m_charInfo->m_position.x;
                float dz = target->m_position.z - m_charInfo->m_position.z;

                if (dx * dx + dz * dz <= m_params.radius * m_params.radius) {
                    if (!fn_800B7938(target)) {
                        target->fn_801DC980(new ClrVelocityCommand(4));
                        target->fn_801DC980(new Player::StreamCommand(true, 4));
                    }

                    Fn8040Vector3 acceleration(0.0f, 1.0f, 0.0f);
                    fn_8040CE38(&acceleration, m_params.power);

                    float above = target->m_position.y - m_charInfo->m_position.y;
                    if (above < 0.0f) {
                        above = 0.0f;
                    }
                    float top = 5.0f * std::cos(lbl_8057D6A0.field_0x98) + m_params.height;
                    fn_8040CE38(&acceleration, (top - above) / top);
                    target->fn_801DC980(new AccelerateCommand(acceleration, 4));

                    m_target = target;
                    m_state.Set(0);
                    fn_801D53C4(fn_80053164(), 0x7245, 0, this, 2, 0, 5);
                }
            }

            if (!m_state.Test(0)) {
                Fn8040Vector3 segmentEnd = target->m_position;
                segmentEnd.y += m_params.height;

                UnkSegment_80020A20 segment(target->m_position, segmentEnd);
                /* The original measures from the character info's own position to a segment that
                 * starts there, so the distance is always 0 (kept as in the original). */
                float distance = fn_8003C63C(&target->m_position, &segment, 0);
                if (distance < 10.0f + m_params.radius) {
                    fn_801D53C4(fn_80053164(), 0x7245, 0, this, 2,
                                (s8)(-127.0f * ((distance - m_params.radius) / 10.0f)), 5);
                }
            }
        }
    }
}

extern "C" void fn_80266708(void) {
    fn_8006B474("DE0403DENSIRYUU.DFF", 0);
}

extern "C" void fn_80266704(void) {}

void ElecFan::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecFan(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "height", 0, 0, 0, 15.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {4, "radius", 0, 0, 0, 25.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {4, "power", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054C1C0 = {
    "ELEC_FAN", fn_80266708, fn_80266704, ElecFan::Create, 0, 0, 0x7DA, 1, 0, 10, s_paramInfo,
};

const char *ElecFanDraw::s_name = "ElecFanDraw";

ElecFanDraw::ElecFanDraw(Task *parent, Fn8040Vector3 position, float angle, float radius,
                         float halfHeight)
    : Task(parent), m_radius(radius), m_halfHeight(halfHeight), m_position(position),
      m_angle(angle) {
    m_name = s_name;
    m_clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403DENSIRYUU.DFF"));
    m_layer = fn_8004DFB0(fn_80046178()->field_0x28, "ElecFan");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ElecFanDraw::OnDraw, this, _1));
    m_timer = 0.0f;

    float extent = 300.0f + halfHeight;
    m_boundRadius = std::sqrt(radius * radius + extent * extent);
}

ElecFanDraw::~ElecFanDraw() {}

void ElecFanDraw::Update(float dt) {
    m_timer += dt;
    if (m_timer >= 0.333333f) {
        m_flags |= 1;
    }
}

void ElecFanDraw::OnDraw(RednerDrawArgs &args) {
    RwSphere sphere;

    sphere.radius = m_boundRadius;
    sphere.center.x = m_position.x;
    sphere.center.y = m_position.y + m_halfHeight;
    sphere.center.z = m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        RwRGBA color = {255, 255, 255, 255};

        /* The clump fades out from 0.25 until the end of the task's time (0.333333) and rises 600
         * units over that time: the original's constants are the float values of these
         * expressions. */
        if (m_timer >= 0.25f) {
            float level = 1.0f - (m_timer - 0.25f) / (0.333333f - 0.25f);
            if (level < 0.0f) {
                level = 0.0f;
            } else if (level > 1.0f) {
                level = 1.0f;
            }
            color.red = color.green = color.blue = 255.0f * level;
        }
        fn_80037B84(m_clump, &color);

        RwV3d position = {m_position.x, (600.0f / 0.333333f) * m_timer + m_position.y,
                          m_position.z};
        RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);

        fn_800E9CEC(frame, rwCOMBINEREPLACE, m_angle);
        fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
        fn_80456BC4(m_clump);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
    }
}
