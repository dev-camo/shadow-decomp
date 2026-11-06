/* The warp holes (class names from RTTI; the gadget's descriptor is "WARP_HOLE"): SetWarpHole is
 * the hole and ManagerTask the task that warps a character to the partner hole (behaviour in the
 * class comments). File name inferred from the namespace. Method names marked "(name inferred)"
 * in the header are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first).
 *
 * The character info's command handler is a boost::function object built with
 * boost::bind(&SetWarpHole::OnCommand, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include <cstring>
#include "Game/WarpHole.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80044254.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkManager_80117514.h"
#include "Game/UnkObject_8003A52C.h"
#include "Game/UnkObject_800BADE8.h"
#include "Game/UnkSingleton.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_800B8000.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace WarpHole {

/* Walks the owners with the com id of the search's owner and, for the first other owner of kind
 * 0x1F (also the id in this file's descriptor; that both mean the warp hole is inferred), copies
 * that owner's position and the user into the search and sets m_found. Nothing happens when search
 * is null or has no owner (behaviour inferred; the original name is unknown). */
static void FindPartner(boost::shared_ptr<CharInfo> user, UnkPartnerSearch_8012A028 *search) {
    if (search != 0 && search->m_owner != 0) {
        s32 comId;

        fn_800C9E50(search->m_owner, &comId);

        for (UnkGadgetOwner_800C9CD0 *owner = fn_800CB560(fn_800C9B84(), comId); owner != 0;
             owner = owner->m_next) {
            if (owner != search->m_owner && fn_800C9D24(owner) == 0x1F) {
                fn_800C9F14(owner, &search->m_position);
                search->m_user = user;
                search->m_found = true;
                return;
            }
        }
    }
}

const char *SetWarpHole::s_name = "SetWarpHole";

SetWarpHole::SetWarpHole(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x1B, 4, "SetWarpHole");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SetWarpHole::OnCommand, this, _1));
    ReadOwner(m_owner);
    m_effect = fn_8015D794(&m_charInfo->m_position, 0xE1);

    {
        boost::shared_ptr<CharaColliBase> probe = fn_800C7DEC(
            fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0, 10, 0, &lbl_805E07A0, 20.0f);

        m_handle = fn_8015B8C4(probe);
    }

    UpdateEffect();
}

SetWarpHole::~SetWarpHole() {
    StopEffect();
}

void SetWarpHole::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);

    switch (m_state) {
    case 0:
        m_state = 3;
        break;
    /* Matching workaround: case 3 is listed without a body. 3 is a real state (cases 0 and 4 store
     * it and OnCommand acts in it), and the original's dispatch compares with 3 and branches to the
     * end of the switch (0x8012A218-0x8012A21C); without the label the function is 39 instructions
     * instead of 40. Revisit if the original turns out to do something in this state. */
    case 3:
        break;
    case 4:
        /* The original loads m_comId into a saved register before it calls GetInstance
         * (0x8012A24C); this source loads it after the call (see configure.py). */
        if (!UnkWarpTable_80129504::GetInstance()->m_busy[m_comId]) {
            m_state = 3;
        }
        break;
    case 5:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void SetWarpHole::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateEffect();
}

void SetWarpHole::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    fn_8040CE60(&field_0x4C);
}

void SetWarpHole::UpdateEffect() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        fn_80425E4C(effect.get(), &m_charInfo->m_position);
    }
}

void SetWarpHole::StopEffect() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
}

void SetWarpHole::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0xD:
        if (m_state == 3) {
            if (boost::shared_ptr<CharInfo> user =
                    static_cast<UseCommand &>(command).m_user.lock()) {
                UnkPartnerSearch_8012A028 search;

                search.m_self = m_charInfo;
                search.m_owner = m_owner;
                FindPartner(user, &search);

                if (search.m_found) {
                    boost::shared_ptr<Player::Player> player =
                        boost::dynamic_pointer_cast<Player::Player>(user);

                    if (player.get() != 0) {
                        ManagerTask::Create(search.m_position, player, m_comId);
                    }
                }
            }
        }
        break;
    }
}

void UnkWarpTable_80129504::fn_80129E38() {
    memset(m_busy, 0, sizeof(m_busy));
}

void UnkWarpTable_80129504::fn_80129E10() {
    memset(m_busy, 0, sizeof(m_busy));
}

const char *ManagerTask::s_name = "ManagerTask";

ManagerTask::ManagerTask(Task *parent, const Fn8040Vector3 &position,
                         const boost::shared_ptr<Player::Player> &user, int comId)
    : Task(parent) {
    m_name = s_name;
    m_position = position;
    m_user = user;
    m_comId = comId;
    m_state = 0;

    if (m_comId >= 0) {
        UnkWarpTable_80129504::GetInstance()->m_busy[m_comId] = 1;
        m_warpFlags.Set(0);
    }

    user->fn_801DC980(new ClrVelocityCommand(4));
    user->fn_801DC980(new Player::StartExternalControlCommand(0, 4));

    fn_80203970(&lbl_8057D6A0.field_0xA8, true);

    UnkObject_800BADE8 *object = fn_800BADE8();

    fn_801E2F28(object);
    fn_801E2EDC(object);
    object->field_0x30 = 0;
    m_warpFlags.Reset(1);
}

ManagerTask::~ManagerTask() {
    if (m_warpFlags.Test(0)) {
        UnkWarpTable_80129504::GetInstance()->m_busy[m_comId] = 0;
    }

    if (fn_80074470()->field_0x8 != 6) {
        /* Japan and PAL test fn_80342284 here; the USA build has no copy of that function and tests
         * fn_802E5810 instead. */
#if defined(VERSION_GUPE8P)
        if (fn_802E5810(UnkSingleton<UnkManager_80117514>::Instance())) {
#else
        if (fn_80342284() != 1) {
#endif
            fn_80203970(&lbl_8057D6A0.field_0xA8, false);
        }
    }

    /* The original loads the 1 after the call (0x80129BD8); this source loads it into a saved
     * register before the call, here and at the start of Update (see configure.py). */
    fn_800BADE8()->field_0x30 = 1;
}

void ManagerTask::Update(float dt) {
    if (!fn_802E5810(UnkSingleton<UnkManager_80117514>::Instance())) {
        fn_800BADE8()->field_0x30 = 1;
    }

    switch (m_state) {
    case 0:
        fn_800446D0(fn_80044254(), 1.0f, 1, 1);
        m_state = 1;
        fn_801D560C(fn_80053164(), 0xE014, 0, 0);
        break;
    case 1:
        if (fn_80044254()->field_0x14 == 1) {
            boost::shared_ptr<Player::Player> user = m_user.lock();

            if (user.get() != 0) {
                fn_800B8000(m_user, &m_position, &user->m_rotation);

                if (lbl_8051CE88[0] != 0) {
                    lbl_8051CE88[0]->field_0x40 = 2;
                }
            }

            m_state = 2;
            m_counter = 0;
        }
        break;
    case 2:
        if (m_counter == 1) {
            UnkObject_8003A52C *object = lbl_8051CE88[0];

            if (object != 0) {
                fn_8003A52C(object);
                object->field_0x40 = 2;
            }

            m_state = 3;
        }
        m_counter++;
        break;
    case 3: {
        boost::shared_ptr<Player::Player> user = m_user.lock();
        UnkObject_8003A52C *object = lbl_8051CE88[0];

        /* Matching workaround: the original evaluates both tests and combines them with a bitwise
         * or (0x80129810); the logical || form branches between them instead (9 of 214 instructions
         * differ). Revisit when the original conditions or the expression behind the or. are
         * recovered. */
        if (object != 0 && !(object->field_0x978 | (object->field_0x40 != 0))) {
            if (user.get() != 0) {
                user->fn_801DC980(new Player::EndExternalControlCommand(4));
            }

            fn_800447B8(fn_80044254(), 0.0f, 1);
            fn_801D560C(fn_80053164(), 0xE015, 0, 0);
            m_state = 5;
            m_warpFlags.Set(1);
        } else if (user.get() != 0) {
            fn_800B8000(m_user, &m_position, &user->m_rotation);
        }
        break;
    }
    case 5:
        if (!m_warpFlags.Test(0)) {
            m_flags |= 1;
        }
        break;
    }

    if (m_warpFlags.Test(1) && m_warpFlags.Test(0) && !m_user.expired()) {
        if (fn_800B7AD0(m_user.lock())) {
            m_flags |= 1;
        }
    }
}

void ManagerTask::Create(const Fn8040Vector3 &position,
                         const boost::shared_ptr<Player::Player> &user, int comId) {
    new ManagerTask(fn_801EB304(fn_8001C4F8(), 11), position, user, comId);
}

void ManagerTask::Create(const Fn8040Vector3 &position,
                         const boost::shared_ptr<Player::Player> &user) {
    Create(position, user, -1);
}

} // namespace WarpHole

extern "C" void fn_80129594(void) {
    WarpHole::UnkWarpTable_80129504::GetInstance()->fn_80129E38();
}

WarpHole::UnkWarpTable_80129504 *WarpHole::UnkWarpTable_80129504::fn_80129504() {
    static UnkWarpTable_80129504 s_table;

    return &s_table;
}

WarpHole::UnkWarpTable_80129504 *WarpHole::UnkWarpTable_80129504::GetInstance() {
    static UnkWarpTable_80129504 *s_instance;

    if (s_instance == 0) {
        s_instance = fn_80129504();
    }

    return s_instance;
}

extern "C" void fn_801294AC(void) {
    WarpHole::UnkWarpTable_80129504::GetInstance()->fn_80129E10();
}

void WarpHole::SetWarpHole::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetWarpHole(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_805280DC = {
    "WARP_HOLE", fn_80129594, fn_801294AC, WarpHole::SetWarpHole::Create, 0, 0, 0x1F, 1, 0, 10, 0,
};
