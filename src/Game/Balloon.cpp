/* The BALLOON gadget (classes described in Game/Balloon.h): Balloon::Balloon gives the item its
 * "item" parameter selects to the character that touches it, attacks it or pulls it in, and
 * Balloon::VacuumBalloon is its model while a character pulls it in. File name inferred from the
 * classes' namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The character info's command handler, the model's collision callback and the attack object's
 * callback are boost::function objects built with boost::bind(&Balloon::method, this, _1); their
 * managers, invokers and copy functions come from the boost headers. */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses them. TNumEdit.h comes first, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). Revisit when the original inline functions or their
 * real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Balloon.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The number of the balloon's model: the constructor loads it and the descriptor's load hook
 * (fn_800E6AC8) adds it to the resource list (name inferred). */
static const int kBalloonModelId = 0xC2;

/* The give-item callback's first argument to fn_8015F6EC for each item 0-7 (Ring5, Ring10 and
 * Ring20 share 0 and differ in the count; name inferred). */
static const s32 s_getItemIds[] = {0, 0, 0, 1, 2, 3, 4, 9};

/* The scale the VacuumBalloon's model shrinks to (name inferred), set at startup by this file's
 * static initialiser. */
static const float s_endScale = lbl_805F72B0 / 15.0f;

namespace Balloon {

const char *Balloon::s_name = "Balloon";

} // namespace Balloon

/* The give-item callback's context (placeholder name: the binary does not name it). Its
 * constructor (0x800E67B0) clears item and done. The context of the ITEM_BOX gadget's give-item
 * callback has the same layout, and its constructor (0x8010B648) is a separate, byte-identical
 * copy. Separate copies do not show whether the original had one class or two, so their original
 * relationship is unknown; each file keeps its own placeholder type. */
struct UnkItemContext_800E67B0 {
    UnkItemContext_800E67B0() : item(0), done(false) {}

    /* 0x00 */ Fn8040Vector3 position; /* where the effect 0xA9 starts */
    /* 0x0C */ s32 item;
    /* Set by fn_800E8438 once it has processed an item request; it does not mean that the
     * character received the item. */
    /* 0x10 */ bool done;
};
CHECK_SIZEOF(UnkItemContext_800E67B0, 0x14);

/* The give-item callback (placeholder name): the callback that fn_8015AA84 calls for each
 * qualifying character info of the collision results (see Balloon::OnCollision), also called
 * directly with a single character info. Unless context's done is set, it passes the item's
 * s_getItemIds entry and a count (5, 10 or 20 for items 0, 1 and 2, 1 for the others) to
 * fn_8015F6EC, which sends the character info a Player::GetItemCommand (a null command when its
 * allocation fails), the context's position and 0xA9 to fn_8015D794 (an effect) and the sound
 * 0xE01D to fn_801D560C; then it sets done, whatever the callees did with the request (behaviour
 * inferred). */
static void fn_800E8438(boost::shared_ptr<CharInfo> hit, void *context) {
    UnkItemContext_800E67B0 *give = static_cast<UnkItemContext_800E67B0 *>(context);

    if (!give->done) {
        int count = 1;

        if (give->item == 0) {
            count = 5;
        } else if (give->item == 1) {
            count = 10;
        } else if (give->item == 2) {
            count = 20;
        }

        fn_8015F6EC(hit, s_getItemIds[give->item], count, 1);
        fn_8015D794(&give->position, 0xA9);
        fn_801D560C(fn_80053164(), 0xE01D, 0, 0);

        give->done = true;
    }
}

namespace Balloon {

Balloon::Balloon(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(8, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Balloon::OnCommand, this, _1));

    m_model = fn_8015F9D8(0, kBalloonModelId);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&Balloon::OnCollision, this, _1));

    boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161334("col");

    if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Balloon::OnAttack, this, _1));
    }

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x880);

    ReadOwner(m_owner);
    UpdateModels();
    m_state = STATE_START;
}

Balloon::~Balloon() {
    fn_801E7108(m_target);
}

void Balloon::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        /* The roundtrip and circle blocks are the same apart from the state; the original has
         * a separate copy of each (0x800E70C0 and 0x800E70F0). */
        if (m_params.type == TYPE_STRAIGHT) {
            m_velocity = Fn8040Vector3(m_params.x, m_params.y, m_params.z);

            float speed = fn_8040CF50(m_velocity);

            if (m_params.sec != 0.0f) {
                speed /= m_params.sec;
            }

            m_velocity = fn_8040CEE8(m_velocity);
            fn_8040CE38(&m_velocity, speed);
            m_state = STATE_STRAIGHT;
            m_timer = 0.0f;
        } else if (m_params.type == TYPE_ROUNDTRIP) {
            m_angularSpeed = m_params.sec != 0.0f ? 360.0f / m_params.sec : 0.0f;
            m_state = STATE_ROUNDTRIP;
            m_timer = 0.0f;
        } else {
            m_angularSpeed = m_params.sec != 0.0f ? 360.0f / m_params.sec : 0.0f;
            m_state = STATE_CIRCLE;
            m_timer = 0.0f;
        }
        break;

    case STATE_STRAIGHT:
        m_timer += dt;

        if (m_timer > m_params.sec) {
            m_state = STATE_DONE;
        } else {
            Fn8040Vector3 offset = m_velocity;

            fn_8040CE38(&offset, m_timer);

            Fn8040Vector3 origin;

            fn_800C9F14(m_owner, &origin);
            m_charInfo->m_position = origin + offset;
        }
        break;

    case STATE_ROUNDTRIP: {
        m_timer += dt;

        if (m_timer > m_params.sec) {
            m_timer = 0.0f;
        }

        float angle = m_angularSpeed * m_timer;
        Fn8040Vector3 end(m_params.x, m_params.y, m_params.z);
        Fn8040Vector3 origin;

        fn_800C9F14(m_owner, &origin);
        fn_8040CFF8(&end, &origin);

        Fn8040Vector3 offset = end - origin;
        float halfLength = 0.5f * fn_8040CF50(offset);

        offset = fn_8040CEE8(offset);
        fn_8040CE38(&offset, halfLength - halfLength * std::cos(fn_8000AB90(angle)));
        m_charInfo->m_position = offset + origin;
        break;
    }

    case STATE_CIRCLE: {
        m_timer += dt;

        if (m_timer > m_params.sec) {
            m_timer = 0.0f;
        }

        float angle = m_angularSpeed * m_timer;
        Fn8040Vector3 offset(0.0f, 0.0f, m_params.radius);
        Fn8040Vector3 origin;

        fn_800C9F14(m_owner, &origin);

        Fn8040Matrix34 matrix = lbl_805E0720;

        fn_8040B59C(&matrix, origin);
        fn_8040B848(&matrix, DegreesToRadians(angle));
        /* The call by name is inferred (the inline operator* reaches the same function): the
         * original loads m_charInfo before the call (USA (GUPE8P) 0x800E72EC), the order this
         * compiler gives a call by name and not the operator; see fn_8040_vector_types.h. */
        m_charInfo->m_position = fn_8040B67C(matrix, offset);
        break;
    }

    case STATE_DONE:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }

    UpdateModels();
}

void Balloon::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModels();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();

        if (m_params.type == TYPE_CIRCLE) {
            UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

            rotation.x = DegreesToRadians(90.0f);
            fn_802E0C60(m_charInfo->m_position, rotation, 0, 5.0f, m_params.radius);
        } else {
            Fn8040Vector3 start = m_charInfo->m_position;
            Fn8040Vector3 end(m_params.x, m_params.y, m_params.z);

            fn_8040CFF8(&end, &start);
            fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
        }
    }
}

void Balloon::OnCollision(ColliDetectArgs &args) {
    UnkItemContext_800E67B0 context;

    context.position = m_charInfo->m_position;
    context.item = m_params.item;
    fn_8015AA84(&args, fn_800E8438, &context);

    if (context.done) {
        m_state = STATE_DONE;
    }
}

void Balloon::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_STRAIGHT || m_state == STATE_ROUNDTRIP || m_state == STATE_CIRCLE) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x240) == 0) {
                param.field_0x54 = 5;
                return;
            }
        }

        param.field_0x54 = 3;

        /* Matching workaround: the weak pointer is a local of its own block. The original
         * destroys it (0x800E6E50) before it stores state 4; declared in the enclosing block it
         * is destroyed after that store (5 words differ). Revisit if another form gives the
         * original's order. */
        {
            boost::weak_ptr<CharaColliAttack> attackRef(param.field_0x0);

            if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
                if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                    UnkItemContext_800E67B0 context;

                    context.position = m_charInfo->m_position;
                    context.item = m_params.item;
                    fn_800E8438(attacker, &context);
                }
            }
        }

        m_state = STATE_DONE;
    } else {
        param.field_0x54 = 5;
    }
}

void Balloon::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_START;
}

void Balloon::UpdateModels() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

} // namespace Balloon

/* Creates a VacuumBalloon with the model and the item, puts it at position with the given
 * rotation and keeps it with a VacuumObjectTask (placeholder name; behaviour and the const pointer
 * spellings inferred). When the VacuumBalloon's allocation fails, the shared pointer is empty and
 * the two virtual calls are still made through it (as in the original); the VacuumObjectTask is
 * constructed only when its own allocation succeeds. */
static void fn_800E63E8(VacuumCommand *command, boost::shared_ptr<GadgetModelInfo> model, int item,
                        const Fn8040Vector3 *position, const UnkRotation3 *rotation);

namespace Balloon {

void Balloon::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14:
        if (m_model.get() != 0) {
            m_model->fn_80161880();
            fn_800E63E8(static_cast<VacuumCommand *>(&command), m_model, m_params.item,
                        &m_charInfo->m_position, &m_charInfo->m_rotation);
            m_model.reset();
        }
        m_state = STATE_DONE;
        break;
    }
}

} // namespace Balloon

extern "C" void fn_800E6AC8(void) {
    fn_8006B50C(kBalloonModelId, 1);
}

extern "C" void fn_800E6AC4(void) {}

void Balloon::Balloon::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Balloon(parent, owner);
}

/* The names of the values of the "type" and "item" parameters, and the labels of "radius", "X",
 * "Y" and "Z" for each type. */
static const char *s_typeLabels[3] = {"straight", "roundtrip", "circle"};
static const char *s_itemLabels[8] = {
    "item : Ring5",       "item : Ring10",        "item : Ring20",   "item : Barrier",
    "item : Heatbarrier", "item : Magnetbarrier", "item : Unbeaten", "item : Blank",
};
static const char *s_radiusLabels[3] = {"Not Used", "Not Used", "radius"};
static const char *s_xLabels[3] = {"X", "X", "Not Used"};
static const char *s_yLabels[3] = {"Y", "Y", "Not Used"};
static const char *s_zLabels[3] = {"Z", "Z", "Not Used"};

/* The label callback of the descriptor's "type", "item", "radius", "X", "Y" and "Z" entries: the
 * labels of "radius", "X", "Y" and "Z" follow the value of the last "type" label request
 * (behaviour inferred). */
static const char *fn_800E6990(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_type = 0;

    if (cursor->m_index == 0) {
        s_type = cursor->m_value;
        return s_typeLabels[s_type];
    } else if (cursor->m_index == 1) {
        return s_itemLabels[cursor->m_value];
    } else if (cursor->m_index == 3) {
        return s_radiusLabels[s_type];
    } else if (cursor->m_index == 4) {
        return s_xLabels[s_type];
    } else if (cursor->m_index == 5) {
        return s_yLabels[s_type];
    } else if (cursor->m_index == 6) {
        return s_zLabels[s_type];
    }

    return 0;
}

namespace Balloon {

VacuumBalloon::VacuumBalloon(VacuumCommand *command, boost::shared_ptr<GadgetModelInfo> model,
                             int item)
    : VacuumObject(fn_80237EDC(command), command), m_item(item) {
    m_model = model;
}

void VacuumBalloon::Update(float dt) {
    VacuumObject::Update(dt);

    if (m_model.get() != 0) {
        Fn8040Vector3 position = fn_802382B0();
        UnkRotation3 rotation = fn_80238248();
        float progress = fn_8023800C();

        progress = fn_80070460(progress, 0.0f, 1.0f);

        float scale = 1.0f - (1.0f - s_endScale) * progress;
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, position);
        fn_8040B848(&matrix, rotation.y);
        fn_8040B884(&matrix, rotation.x);
        fn_8040B80C(&matrix, rotation.z);
        fn_8040B720(&matrix, scale, scale, scale);
        m_model->fn_801633DC(&matrix);
        m_model->fn_80162FB0(dt);
    }
}

void VacuumBalloon::fn_80238180() {
    boost::shared_ptr<CharInfo> charInfo = fn_80238058().lock();

    if (charInfo.get() != 0) {
        UnkItemContext_800E67B0 context;

        context.position = fn_802382B0();
        context.item = m_item;
        /* Matching workaround: the context's constructor has already cleared done, but the
         * original stores 0 to it again here (0x800E6770); without this store the function is 8
         * bytes shorter. Revisit if the original context type is found. */
        context.done = false;
        fn_800E8438(charInfo, &context);
    }
}

} // namespace Balloon

static void fn_800E63E8(VacuumCommand *command, boost::shared_ptr<GadgetModelInfo> model, int item,
                        const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    /* Matching workaround: the new-expression is a statement of its own. The original destroys
     * the copy of model passed to the constructor at the end of that statement (0x800E6458),
     * before it builds the shared pointer; written inside the shared pointer's initialiser, that
     * copy lives to the end of the declaration (12 words differ). Revisit if another form gives
     * the original's order. */
    Balloon::VacuumBalloon *balloon = new Balloon::VacuumBalloon(command, model, item);
    boost::shared_ptr<VacuumObject> object = boost::shared_ptr<VacuumObject>(balloon);

    object->fn_802382E0(position);
    object->fn_80238268(rotation);
    new VacuumObjectTask(object);
}

namespace Balloon {

VacuumBalloon::~VacuumBalloon() {}

} // namespace Balloon

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800E6990, 0},
    {1, "item", 0, 0, 7, 0.0f, 0.0f, 0.0f, fn_800E6990, 0},
    {4, "sec", 0, 0, 0, 0.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "radius", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800E6990, 0},
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800E6990, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800E6990, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800E6990, 0},
    {0},
};

GadgetTypeInfo lbl_80524708 = {
    "BALLOON", fn_800E6AC8, fn_800E6AC4, Balloon::Balloon::Create, 0, 0, 0x13, 1,
    0,         10,          s_paramInfo,
};
