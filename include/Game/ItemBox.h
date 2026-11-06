#ifndef GAME_ITEMBOX_H
#define GAME_ITEMBOX_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/VacuumObject.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommand;
struct RpClump;

namespace SetItem {

/* What an item box draws inside itself (placeholder name: the binary does not name it). Its
 * constructor (0x8010DE30) clears every member except field_0x4. */
struct ItemBoxIcon {
    ItemBoxIcon()
        : m_clump(0), m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_item(0),
          m_timer(0.0f) {}

    /* The model of the item shown, or null for none (ItemBox::LoadIcon). */
    /* 0x00 */ RpClump *m_clump;
    /* The resource 0x68 (fn_8001277C), loaded along with m_clump; ItemBox::OnDraw draws only
     * when both are loaded and does not use this one otherwise (type unknown). */
    /* 0x04 */ void *field_0x4;
    /* 0x08 */ Fn8040Vector3 m_position;
    /* 0x14 */ UnkRotation3 m_rotation;
    /* 0x20 */ s32 m_item;    /* the item shown (an index into ItemBox::s_itemNames) */
    /* 0x24 */ float m_timer; /* the roulette's countdown to the next item (see ItemBox::Update) */
};
CHECK_SIZEOF(SetItem::ItemBoxIcon, 0x28);

/* SetItem::ItemBox (class name from the RTTI of boost's checked_deleter<SetItem::ItemBox>, which
 * owns it through shared pointers): an item box holding one of the items in s_itemNames, with the
 * model 0xD7, a character info whose command handler reacts to commands 0x14, 0x1A and 0x1B, a
 * draw layer that draws the item's icon and a Target. A box of type 10 (Roulette) cycles through
 * items 0-8 while m_state is 2 (see Update). While bit 0 of m_flags is set and m_state is 1 or 2,
 * a qualifying character info that touches the box, or the owner of an accepted attack on it, has
 * the item shown requested for it through fn_8010DE60 (see OnCollision and OnAttack for the
 * conditions); a VacuumCommand makes a VacuumItemBox for the item instead (see OnCommand).
 * Behaviour inferred; member names and method names are inferred from the code. */
class ItemBox {
  public:
    /* delay goes to m_delay (see Update). */
    ItemBox(int type, float delay);
    ~ItemBox();

    /* Advances the model by dt (when there is one) and subtracts dt from m_delay. In the first
     * update in which m_delay is below 0 (bit 2 of m_flags still clear), it sets the model's
     * collision callback and, when the model has the attack object "col", that object's callback,
     * and sets bit 2. Then, in the same update, it acts on m_state (name inferred): state 0 becomes
     * 1, or 2 with the item shown set to 0 for a Roulette box, and loads the icon; state 1 does
     * nothing; in state 2 it subtracts dt from the icon's timer and, when the result is <= 0,
     * resets the timer to 0.5 and advances the item shown by one (from 8 back to 0) and loads its
     * icon, so the item advances at most once per update; states 3 and 4 set bit 1 of m_flags. */
    void Update(float dt);
    /* Returns fn_80169BE4 of the character info's position (name inferred): false when the
     * squared distance to the vector that fn_80010244 returns for index 0, or also for index 1
     * while the byte at 0xB2 of lbl_8057D6A0 is set, is below squaredDistance. Whether those
     * vectors are player positions is not established. */
    bool IsFarFromPlayers(float squaredDistance);
    /* Moves the character info and updates the model, the icon and the Target (name inferred).
     * rotation is not const (inferred from the load order): the original loads each of its
     * components only after storing the previous one, as this compiler does when the stores may
     * change the source (with a const pointer the second component is loaded before the first
     * store). */
    void SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation);
    /* Places the model at the character info's matrix (when there is a model), copies the
     * character info's position and rotation to the icon, clears the icon's timer and moves the
     * Target (when there is one) to the position (name inferred). */
    void UpdateModels();
    /* The model's collision callback (name inferred): while bit 0 of m_flags is set and m_state
     * is 1 or 2, passes fn_8010DE60 and a context with the item shown to fn_8015B004, which calls
     * it for each hit whose collision object and character info still exist and whose character
     * info's fn_80078B2C is true. The first such call requests the item and sets the context's
     * done, which makes the later calls do nothing; when done is set, m_state becomes 3. */
    void OnCollision(ColliDetectArgs &args);
    /* The attack callback (name inferred): sets the parameter's word at 0x54 to 5; then, while bit
     * 0 of m_flags is set and m_state is 1 or 2, when the attack object and its owner still exist
     * and the owner's fn_80078B34 is true, requests the item shown for the owner (fn_8010DE60) and
     * sets m_state and the parameter's word to 3. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Loads the icon model of the item shown, or clears both icon models when the item has none
     * (name inferred). */
    void LoadIcon();
    /* The draw layer's callback (name inferred): when both icon models are loaded and a sphere of
     * radius 10 at the icon's position is in the current camera's view, turns the icon by the
     * angles of fn_80109B80, moves it to the icon's position and draws it. */
    void OnDraw(RednerDrawArgs &args);
    /* The character info's command handler (name inferred): command 0x1A clears bit 0 of m_flags
     * and, when it is set, bit 0 of the Target's flags; command 0x1B sets bit 0 of m_flags and,
     * when it is clear, bit 0 of the Target's flags; command 0x14 (a VacuumCommand), unless the
     * box is taken, makes a VacuumItemBox for the item (the item shown for a Roulette box) and
     * sets m_state to 4. */
    void OnCommand(CharCommand &command);

    /* Whether the box has been used up (bit 1 of m_flags; name inferred). */
    bool IsTaken() const {
        return m_flags.Test(1);
    }

    /* Item types: indices into s_itemNames (names inferred). */
    enum {
        kItemRoulette = 10, /* "Roulette" */
        kItemCount = 11
    };

    /* The item names, in type order (name inferred). */
    static const char *s_itemNames[kItemCount];

    /* 0x00 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x10 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x18 */ boost::shared_ptr<Target> m_target;
    /* 0x20 */ ItemBoxIcon m_icon;
    /* 0x48 */ s32 m_type;
    /* 0 until the first update, which sets 1 (2 for a Roulette box, whose item shown becomes 0)
     * and loads the icon; 3 once OnCollision or OnAttack has processed an item request (which
     * does not guarantee that the character received the item) and 4 after a VacuumCommand. The
     * next update after 3 or 4 sets bit 1 of m_flags. */
    /* 0x4C */ s32 m_state;
    /* 0x50 */ float m_delay; /* seconds until the callbacks are set */
    /* Bit 0 is set by the constructor (see OnCommand), bit 1 marks the box as taken and bit 2 is
     * set once the callbacks are set. */
    /* 0x54 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(SetItem::ItemBox, 0x58);

/* SetItem::VacuumItemBox (class name from RTTI, with VacuumObject as its only base): the box's item
 * while a character pulls it in (inferred from the name). Its constructor passes m_item to the
 * model 0xD7's fn_80162A68; Update places the model, when there is one, at the object's position
 * and rotation, and fn_80238180 requests the item for a character info (behaviour inferred). */
class VacuumItemBox : public VacuumObject {
  public:
    VacuumItemBox(VacuumCommand *command, int item);
    virtual ~VacuumItemBox();
    /* Advances the base object, then, when the model is not null, places the model at the
     * object's position and rotation and advances it by dt. */
    virtual void Update(float dt);
    /* Locks fn_80238058's weak pointer and, when it is not null, passes that character info and a
     * context with the object's position (fn_802382B0) and m_item to fn_8010DE60. */
    virtual void fn_80238180();

    /* 0x08 */ s32 m_item;
    /* 0x0C */ boost::shared_ptr<GadgetModelInfo> m_model;
};
CHECK_SIZEOF(SetItem::VacuumItemBox, 0x14);

/* SetItem::ItemBoxTask (class name from RTTI, with Task as its only base): the task that keeps an
 * item box that fn_8010B824 places. Each update, when it has a box, it finishes if the box's
 * IsFarFromPlayers(1000000.0f) is true (nothing that fn_80169BE4 checks is closer than 1000 units),
 * otherwise if the box is taken, and otherwise updates the box; without a box it does nothing.
 * Member names are inferred from the code. */
class ItemBoxTask : public Task {
  public:
    ItemBoxTask(Task *parent, boost::shared_ptr<ItemBox> box);
    virtual ~ItemBoxTask();
    virtual void Update(float dt);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<ItemBox> m_box;
};
CHECK_SIZEOF(SetItem::ItemBoxTask, 0x30);

/* SetItem::SetItemBoxTask (class name from RTTI; the gadget's descriptor is "ITEM_BOX"): the gadget
 * that places an item box of the type its "item" parameter selects. Member names and the names of
 * the non-virtual methods are inferred from the code. */
class SetItemBoxTask : public Gadget {
  public:
    SetItemBoxTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetItemBoxTask();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position and rotation from the owner into the character info, copies the
     * owner's parameter block into m_itemParam when its size is 4 (fn_800C9DBC; otherwise
     * m_itemParam keeps its value) and copies m_itemParam to m_item (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Moves the box to the character info's position and rotation (name inferred). */
    void UpdateBox();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<ItemBox> m_box;
    /* Holds the box's position and rotation (m_position, m_rotation), which ReadOwner fills and
     * UpdateBox passes on. */
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ s32 m_itemParam; /* the "item" parameter */
    /* 0x44 */ s32 m_item;      /* m_itemParam as last read */
};
CHECK_SIZEOF(SetItem::SetItemBoxTask, 0x48);

} // namespace SetItem

extern "C" {
/* Returns the name of the item type, or null when type is not between 0 and 10 (behaviour
 * inferred; the original name is unknown). The functions at 0x800F6580 and 0x800F9670 call it. */
const char *fn_8010B7F8(int type);
/* Places an item box of the type at position with the given rotation, kept by an ItemBoxTask; the
 * box waits delay seconds before it sets its callbacks. Does nothing when type is not between 0
 * and 10 (behaviour inferred; the original name is unknown). The function at 0x8033E898 calls it.
 * rotation is passed on to ItemBox::SetTransform (not const, inferred from that function's load
 * order). */
void fn_8010B824(int type, const Fn8040Vector3 *position, UnkRotation3 *rotation, float delay);
}

#endif
