/* SetItem (class names from RTTI; the gadget's descriptor is "ITEM_BOX"): item boxes. SetItemBoxTask
 * places a box of the type its "item" parameter selects; fn_8010B824 places boxes for other
 * objects, each kept by an ItemBoxTask. A box is an ItemBox: the model 0xD7, the icon of the item
 * it holds and a character info and a Target through which characters use it; for a character
 * that pulls a box in, the item is requested through a VacuumItemBox (behaviour inferred). File
 * name inferred from the class. Method names marked "(name inferred)" in the header are not
 * original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The character info's command handler, the draw layer's callback, the model's collision callback
 * and the attack object's callback are boost::function objects built with
 * boost::bind(&ItemBox::method, this, _1); their managers, invokers and copy functions come from the
 * boost headers. */
#include "Game/ItemBox.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/Player.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8005AE20.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_80109AF8.h"
#include "Game/fn_80169BE4.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The number of the box's model: ItemBox's and VacuumItemBox's constructors load it and the
 * descriptor's load hook (fn_8010BBC0) adds it to the resource list (name inferred). */
static const int kItemBoxModelId = 0xD7;

namespace SetItem {

const char *ItemBoxTask::s_name = "ItemBoxTask";

const char *SetItemBoxTask::s_name = "SetItemBoxTask";

} // namespace SetItem

/* fn_8010DE60's first argument to fn_8015F6EC for each item type 0-9 (Ring5, Ring10 and Ring20
 * share 0 and differ in the count; name inferred). */
static const s32 s_getItemIds[] = {0, 0, 0, 1, 2, 3, 4, 9, 6, 5};

/* fn_8010DE60's argument to fn_8020642C for each item type 0-9 (meaning unknown; name
 * inferred). */
static const s32 s_useIds[] = {4, 5, 6, 8, 8, 8, 8, 7, 8, 8};

/* The context of fn_8010DE60 (placeholder name: the binary does not name it). Its constructor
 * (0x8010B648) clears item and done. */
struct GiveItemContext {
    GiveItemContext() : item(0), done(false) {}

    /* 0x00 */ Fn8040Vector3 position; /* where the effect 0xA9 starts */
    /* 0x0C */ s32 item;
    /* Set by fn_8010DE60 once it has processed an item request; it does not mean that the
     * character received the item. */
    /* 0x10 */ bool done;
};
CHECK_SIZEOF(GiveItemContext, 0x14);

/* Requests the item for a character info (placeholder name): the callback that fn_8015B004 calls
 * for each qualifying hit in the collision results (see ItemBox::OnCollision), also called
 * directly with a single character info. Unless context's done is set, it passes the item's
 * s_getItemIds entry and a count (5, 10 or 20 for items 0, 1 and 2, 1 for the others) to
 * fn_8015F6EC, the item's s_useIds entry to fn_8020642C, the context's position and 0xA9 to
 * fn_8015D794 (an effect) and the sound 0xE01C to fn_801D560C; then, when the character info's
 * fn_80078B44 is true and fn_800B7AD0 is false, it sends it a Player::HomingJumpCommand with a
 * zero vector; then it sets done, whether or not the command's allocation succeeded and whatever
 * the callees did with the request (behaviour inferred). */
static void fn_8010DE60(boost::shared_ptr<CharInfo> hit, void *context) {
    GiveItemContext *give = static_cast<GiveItemContext *>(context);

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
        fn_8020642C(hit, s_useIds[give->item]);
        fn_8015D794(&give->position, 0xA9);
        fn_801D560C(fn_80053164(), 0xE01C, 0, 0);

        if (hit->fn_80078B44() && !fn_800B7AD0(hit)) {
            /* The original stores the zero vector before the allocation (0x8010DF70-0x8010DF80);
             * this source stores it after the allocation (see configure.py). */
            hit->fn_801DC980(new Player::HomingJumpCommand(Fn8040Vector3(0.0f, 0.0f, 0.0f), 4));
        }

        give->done = true;
    }
}

namespace SetItem {

ItemBox::ItemBox(int type, float delay) {
    m_type = type;
    m_icon.m_item = m_type;
    m_charInfo = fn_8015CB88(0x17, 4, "ItemBox");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ItemBox::OnCommand, this, _1));

    m_model = fn_8015F9D8(0, kItemBoxModelId);
    m_model->fn_80162A68(1000, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));

    m_layer = fn_8004E24C(fn_80046178()->field_0x8);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ItemBox::OnDraw, this, _1));

    LoadIcon();

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xC90);
    m_target->SetField20(fn_8005AE20(5.0f));

    m_state = 0;
    m_delay = delay;
    m_flags.Clear();
    m_flags.Set(0);
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here. */
#pragma auto_inline on
ItemBox::~ItemBox() {
    fn_801E7108(m_target);
}
#pragma auto_inline reset

void ItemBox::Update(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    m_delay -= dt;

    if (!m_flags.Test(2) && m_delay < 0.0f) {
        m_model->m_collisionCallback =
            ColliDetectCallback(boost::bind(&ItemBox::OnCollision, this, _1));

        boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161334("col");

        if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&ItemBox::OnAttack, this, _1));
        }

        m_flags.Set(2);
    }

    switch (m_state) {
    case 0:
        if (m_type == kItemRoulette) {
            m_icon.m_item = 0;
            m_state = 2;
        } else {
            m_state = 1;
        }
        LoadIcon();
        break;
    case 2:
        m_icon.m_timer -= dt;
        if (m_icon.m_timer <= 0.0f) {
            m_icon.m_timer = 0.5f;
            m_icon.m_item++;
            if (m_icon.m_item > 8) {
                m_icon.m_item = 0;
            }
            LoadIcon();
        }
        break;
    /* Cases 3 and 4 are separate: the original has a copy of the same block for each
     * (0x8010CF2C and 0x8010CF3C); one shared case body gives a single block. */
    case 3:
        m_flags.Set(1);
        break;
    case 4:
        m_flags.Set(1);
        break;
    }
}

bool ItemBox::IsFarFromPlayers(float squaredDistance) {
    return fn_80169BE4(&m_charInfo->m_position, squaredDistance);
}

void ItemBox::SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation) {
    m_charInfo->m_position = *position;
    m_charInfo->m_rotation = *rotation;
    UpdateModels();
}

void ItemBox::UpdateModels() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }

    m_icon.m_position = m_charInfo->m_position;
    m_icon.m_rotation = m_charInfo->m_rotation;
    m_icon.m_timer = 0.0f;

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

void ItemBox::OnCollision(ColliDetectArgs &args) {
    if (m_flags.Test(0) && (m_state == 1 || m_state == 2)) {
        GiveItemContext context;

        context.position = m_charInfo->m_position;
        context.item = m_icon.m_item;
        fn_8015B004(&args, fn_8010DE60, &context);

        if (context.done) {
            m_state = 3;
        }
    }
}

void ItemBox::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_flags.Test(0) && (m_state == 1 || m_state == 2)) {
        boost::weak_ptr<CharaColliAttack> attackRef(param.field_0x0);

        if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                if (attacker->fn_80078B34()) {
                    GiveItemContext context;

                    context.position = m_charInfo->m_position;
                    context.item = m_icon.m_item;
                    fn_8010DE60(attacker, &context);
                    m_state = 3;
                    param.field_0x54 = 3;
                }
            }
        }
    }
}

/* The icon model of each item, -1 for none (name inferred). */
static const s32 s_iconModelIds[] = {0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x73,
                                     0x70, 0x71, 0x74, 0x72, -1};

void ItemBox::LoadIcon() {
    if (s_iconModelIds[m_icon.m_item] != -1) {
        m_icon.m_clump =
            static_cast<RpClump *>(fn_8001277C(fn_800122D4(), 0, s_iconModelIds[m_icon.m_item]));
        m_icon.field_0x4 = fn_8001277C(fn_800122D4(), 0, 0x68);
    } else {
        m_icon.m_clump = 0;
        m_icon.field_0x4 = 0;
    }
}

void ItemBox::OnDraw(RednerDrawArgs &args) {
    if (m_icon.m_clump == 0 || m_icon.field_0x4 == 0) {
        return;
    }

    RwSphere sphere;

    sphere.radius = 10.0f;
    sphere.center.x = m_icon.m_position.x;
    sphere.center.y = m_icon.m_position.y;
    sphere.center.z = m_icon.m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        RwFrame *frame = static_cast<RwFrame *>(m_icon.m_clump->object.parent);
        UnkRotation3 angles = fn_80109B80(fn_80009548(), 0);

        fn_80109AF8(frame, rwCOMBINEREPLACE, angles.z);
        fn_800E9C64(frame, rwCOMBINEPOSTCONCAT, angles.x);
        fn_800E9CEC(frame, rwCOMBINEPOSTCONCAT, angles.y);
        fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
        fn_80456BC4(m_icon.m_clump);
    }
}

} // namespace SetItem

/* Creates a VacuumItemBox for the item, puts it at position with the given rotation and keeps it
 * with a VacuumObjectTask (placeholder name; behaviour inferred). */
static void fn_8010B348(VacuumCommand *command, int item, const Fn8040Vector3 *position,
                        const UnkRotation3 *rotation);

namespace SetItem {

void ItemBox::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x1A:
        m_flags.Reset(0);
        if (m_target->TestFlags(1)) {
            m_target->ClearFlags(1);
        }
        break;
    case 0x1B:
        m_flags.Set(0);
        if (!m_target->TestFlags(1)) {
            m_target->SetFlags(1);
        }
        break;
    case 0x14:
        if (!IsTaken()) {
            int item = m_type;

            if (item == kItemRoulette) {
                item = m_icon.m_item;
            }
            /* The original copies the command pointer to r3 before the m_type test (0x8010C1D8);
             * this source copies it after the test (see configure.py). */
            fn_8010B348(static_cast<VacuumCommand *>(&command), item, &m_charInfo->m_position,
                        &m_charInfo->m_rotation);
            m_state = 4;
        }
        break;
    }
}

ItemBoxTask::ItemBoxTask(Task *parent, boost::shared_ptr<ItemBox> box) : Task(parent) {
    m_name = s_name;
    m_box = box;
}

ItemBoxTask::~ItemBoxTask() {}

void ItemBoxTask::Update(float dt) {
    if (m_box.get() != 0) {
        /* The two finishing conditions have separate bodies: the original has a copy of the
         * flag store for each (0x8010BFE8 and 0x8010C014); one condition joined with || gives a
         * single store. SetWeapon's WeaponTask has the same shape. */
        if (m_box->IsFarFromPlayers(1000000.0f)) {
            m_flags |= 1;
        } else if (m_box->IsTaken()) {
            m_flags |= 1;
        } else {
            m_box->Update(dt);
        }
    }
}

SetItemBoxTask::SetItemBoxTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_itemParam(0) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x17, 4, "ItemBox");
    ReadOwner(owner);
    m_box = boost::shared_ptr<ItemBox>(new ItemBox(m_item, 0.0f));
    UpdateBox();
}

SetItemBoxTask::~SetItemBoxTask() {}

void SetItemBoxTask::UpdateBody(float dt) {
    if (m_box.get() != 0) {
        if (m_box->IsTaken()) {
            if (m_owner != 0) {
                fn_800C9B34(m_owner);
            }
            m_flags |= 1;
        } else {
            m_box->Update(dt);
        }
    }
}

void SetItemBoxTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousItem = m_item;

    ReadOwner(owner);
    UpdateBox();

    if (previousItem != m_item) {
        m_box.reset();
        m_box = boost::shared_ptr<ItemBox>(new ItemBox(m_item, 0.0f));
    }
}

void SetItemBoxTask::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_itemParam, sizeof(m_itemParam));
    m_item = m_itemParam;
}

void SetItemBoxTask::UpdateBox() {
    if (m_box.get() != 0) {
        m_box->SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
    }
}

} // namespace SetItem

extern "C" void fn_8010BBC0(void) {
    fn_8006B50C(kItemBoxModelId, 1);
}

extern "C" void fn_8010BBBC(void) {}

void SetItem::SetItemBoxTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetItemBoxTask(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The name of the item type, or null when type is not between 0 and 10 (name inferred).
 * fn_8010B7F8 is a separate function with the same body; fn_8010BB24 has the check expanded
 * (0x8010BB30-0x8010BB5C, measured: calling fn_8010B7F8 there gives 14 words instead of 17). */
static inline const char *GetItemName(int type) {
    if (type >= 0 && type < SetItem::ItemBox::kItemCount) {
        return SetItem::ItemBox::s_itemNames[type];
    }
    return 0;
}

/* The label callback of the "item" parameter: the name of the item type the value selects, or null
 * (behaviour inferred). */
static const char *fn_8010BB24(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return GetItemName(cursor->m_value);
    }

    return 0;
}

extern "C" void fn_8010B824(int type, const Fn8040Vector3 *position, UnkRotation3 *rotation,
                            float delay) {
    if (type >= 0 && type < SetItem::ItemBox::kItemCount) {
        boost::shared_ptr<SetItem::ItemBox> box =
            boost::shared_ptr<SetItem::ItemBox>(new SetItem::ItemBox(type, delay));

        box->SetTransform(position, rotation);

        /* The parent is looked up before the allocation (as the original does, 0x8010B8A4), so
         * the lookup happens even when the allocation fails: fn_8001C4F8 constructs and registers
         * its static object on its first call, and written in the new-expression the lookup would
         * be skipped when the allocation returns null. */
        Task *parent = fn_801EB304(fn_8001C4F8(), 11);

        new SetItem::ItemBoxTask(parent, box);
    }
}

extern "C" const char *fn_8010B7F8(int type) {
    return GetItemName(type);
}

namespace SetItem {

VacuumItemBox::VacuumItemBox(VacuumCommand *command, int item)
    : VacuumObject(fn_80237EDC(command), command), m_item(item) {
    m_model = fn_8015F9D8(0, kItemBoxModelId);
    m_model->fn_80162A68(m_item, 0);
}

void VacuumItemBox::Update(float dt) {
    VacuumObject::Update(dt);

    Fn8040Vector3 position = fn_802382B0();
    UnkRotation3 rotation = fn_80238248();

    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, position);
        fn_8040B848(&matrix, rotation.y);
        fn_8040B884(&matrix, rotation.x);
        fn_8040B80C(&matrix, rotation.z);
        m_model->fn_801633DC(&matrix);
        m_model->fn_80162FB0(dt);
    }
}

void VacuumItemBox::fn_80238180() {
    boost::shared_ptr<CharInfo> charInfo = fn_80238058().lock();

    if (charInfo.get() != 0) {
        GiveItemContext context;

        context.position = fn_802382B0();
        context.item = m_item;
        /* The context's constructor has already cleared done, but the original stores 0 to it
         * again here (0x8010B608); without this store the function is 8 bytes shorter. */
        context.done = false;
        fn_8010DE60(charInfo, &context);
    }
}

} // namespace SetItem

static void fn_8010B348(VacuumCommand *command, int item, const Fn8040Vector3 *position,
                        const UnkRotation3 *rotation) {
    boost::shared_ptr<VacuumObject> object =
        boost::shared_ptr<VacuumObject>(new SetItem::VacuumItemBox(command, item));

    object->fn_802382E0(position);
    object->fn_80238268(rotation);
    new VacuumObjectTask(object);
}

namespace SetItem {

VacuumItemBox::~VacuumItemBox() {}

} // namespace SetItem

static GadgetParamInfo s_paramInfo[] = {
    {1, "item", 0, 0, 10, 0.0f, 0.0f, 0.0f, fn_8010BB24, 0},
    {0},
};

GadgetTypeInfo lbl_805265B0 = {
    "ITEM_BOX", fn_8010BBC0, fn_8010BBBC, SetItem::SetItemBoxTask::Create, 0, 0, 0x12, 1,
    0,          10,          s_paramInfo,
};

const char *SetItem::ItemBox::s_itemNames[SetItem::ItemBox::kItemCount] = {
    "Ring5",    "Ring10", "Ring20", "Barrier", "Heatbarrier", "Magnetbarrier",
    "Unbeaten", "Blank",  "1UP",    "Recover", "Roulette",
};
