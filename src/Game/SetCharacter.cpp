/* The SET_CHARACTOR gadget. The file name and the class's unnamed namespace come from the RTTI
 * name "@unnamed@SetCharacter_cpp@::SetCharacterTask". Method names marked "(name inferred)" are
 * not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/Gadget.h"
#include "Game/CharInfo.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Npc.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_8006F754.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/fn_800B8000.h"
#include "Game/fn_80076218.h"
#include "Game/fn_802285A0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using Player::Npc::Npc;
using Player::Npc::ShowCommand;

namespace {

/* The gadget parameters in the order of the descriptor's table, whose entry names ("Characotr",
 * as spelt there, and "ContactRange(m)") give the member names. */
struct SetCharacterParams {
    /* 0x0 */ s32 character;
    /* 0x4 */ float contactRange;
};
CHECK_SIZEOF(SetCharacterParams, 0x8);

/* SetCharacterTask (class name from RTTI): in the first update in which the word at 0x8 of the
 * manager that fn_80074470 returns is 3, the gadget tries to place the character that its
 * "Characotr" parameter selects at its owner's position and rotation (SetCharacter), and then sets
 * Task flag 0x01 whether or not it placed it (behaviour inferred). It has no members of its
 * own. */
class SetCharacterTask : public Gadget {
  public:
    SetCharacterTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {}
    /* Passes the owner to fn_800C9AD4 when there is one. */
    virtual ~SetCharacterTask();
    /* When the word at 0x8 of fn_80074470's manager is 3, runs SetCharacter with the owner and
     * sets Task flag 0x01; otherwise it does nothing. */
    virtual void UpdateBody(float dt);
    /* Does nothing. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the character from the owner's parameters (fn_800C9DBC copies them only when the
     * owner's parameter size is 8). It returns at once when the weak pointer that fn_800740E4
     * gives for the id that fn_802285A0 returns for the character has not expired and its actor's
     * word at 0xC is 2. Otherwise it takes the weak pointer that fn_8007488C returns for the
     * character, replaces it with fn_80076218's result when it has expired, and passes it to
     * PlaceCharacter unless that one has expired too (name and behaviour inferred). */
    void SetCharacter(UnkGadgetOwner_800C9CD0 *owner);
    /* Does nothing unless the character info still exists, is a Player::Npc::Npc (dynamic cast)
     * and its fn_80073708 returns false. Then it reads the owner's parameters again (copied only
     * when their size is 8), passes the character info, the owner's position and its rotation to
     * fn_800B8000, passes 10 times "ContactRange(m)" to the character's fn_80073770 and, when the
     * byte at 0xB1 of lbl_8057D6A0 is non-zero, also to its fn_80073838. Last it passes the new
     * command ShowCommand(0, 4) to the character's fn_801DC980 (null when the allocation fails;
     * the call still happens) (name and behaviour inferred). */
    void PlaceCharacter(const boost::weak_ptr<CharInfo> &charInfo, UnkGadgetOwner_800C9CD0 *owner);
};
CHECK_SIZEOF(SetCharacterTask, 0x30);

SetCharacterTask::~SetCharacterTask() {
    if (m_owner != 0) {
        fn_800C9AD4(m_owner);
    }
}

void SetCharacterTask::UpdateBody(float dt) {
    if (fn_80074470()->field_0x8 == 3) {
        SetCharacter(m_owner);
        m_flags |= 1;
    }
}

void SetCharacterTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {}

void SetCharacterTask::SetCharacter(UnkGadgetOwner_800C9CD0 *owner) {
    SetCharacterParams params;

    fn_800C9DBC(owner, &params, sizeof(params));

    int character = params.character;

    /* The weak pointer to the actor is released before the character info is looked up (its
     * destructor call at 0x80119C9C). */
    {
        boost::weak_ptr<UnkActor_800740E4> actor =
            fn_800740E4(fn_8006D6A8(), fn_802285A0(character));

        if (!actor.expired() && actor.lock()->field_0xC == 2) {
            return;
        }
    }

    boost::weak_ptr<CharInfo> charInfo = fn_8007488C(fn_8006F754(), character);

    if (charInfo.expired()) {
        charInfo = fn_80076218(character);
    }

    if (!charInfo.expired()) {
        PlaceCharacter(charInfo, owner);
    }
}

void SetCharacterTask::PlaceCharacter(const boost::weak_ptr<CharInfo> &charInfo,
                                      UnkGadgetOwner_800C9CD0 *owner) {
    boost::shared_ptr<Npc> npc = boost::dynamic_pointer_cast<Npc>(charInfo.lock());

    if (npc.get() == 0) {
        return;
    }

    if (!npc->fn_80073708()) {
        SetCharacterParams params;

        fn_800C9DBC(owner, &params, sizeof(params));

        Fn8040Vector3 position;
        UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

        fn_800C9F14(owner, &position);
        fn_800C9ED4(owner, &rotation);
        fn_800B8000(charInfo, &position, &rotation);
        npc->fn_80073770(10.0f * params.contactRange);

        if (lbl_8057D6A0.field_0xA8.field_0x9 != 0) {
            npc->fn_80073838(10.0f * params.contactRange);
        }

        npc->fn_801DC980(new ShowCommand(0, 4));
    }
}

} // namespace

extern "C" void fn_80119A88(void) {}

extern "C" void fn_80119A84(void) {}

void SetCharacterTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetCharacterTask(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The label callback of the "Characotr" parameter: the character's name from fn_802285D4 for
 * the value (behaviour inferred). */
static const char *fn_801199EC(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return fn_802285D4(cursor->m_value);
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Characotr", 1, 1, 19, 0.0f, 0.0f, 0.0f, fn_801199EC, 0},
    {4, "ContactRange(m)", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80527018 = {
    "SET_CHARACTOR", fn_80119A88, fn_80119A84, SetCharacterTask::Create, 0, 0, 0x190, 1, 0, 10,
    s_paramInfo,
};
