/* The BlockChange gadget (class described in Game/BlockChange.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/BlockChange.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Player.h"
#include "Game/UnkObject_8003A52C.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

const char *BlockChange::s_name = "BlockChange";

extern "C" void fn_800ED9E4(void) {}

extern "C" void fn_800ED9E0(void) {}

void BlockChange::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BlockChange(fn_801EB304(fn_8001C4F8(), 11), owner);
}

BlockChange::BlockChange(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Task(parent) {
    m_name = s_name;
    if (owner != 0) {
        m_owner = owner;
        fn_800C9CD0(m_owner, this);
    }
    ReadOwner(m_owner);
    m_probe = fn_800C7EA8(fn_80021B08(), &m_position, 0xC0100, 10, 0x35, lbl_805EE698,
                          &lbl_805E07A0, m_params.radius);
}

BlockChange::~BlockChange() {
    if (m_probe.get() != 0) {
        fn_800C78B0(fn_80021B08(), &m_probe);
    }
    if (m_owner != 0) {
        fn_800C9BE8(m_owner);
        m_owner = 0;
    }
}

void BlockChange::ProcessHits(float) {
    CollisionResultList *hits = m_probe->GetHits();
    UnkProbeIterator_80027374 iterator;

    for (iterator = hits->fn_8001EE04(); iterator.node != hits->fn_8001EDEC();
         fn_80027374(&iterator, 0)) {
        boost::weak_ptr<CharaColliBase> target = fn_800587A0(&iterator.node->target);

        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            boost::weak_ptr<CharInfo> ownerRef = fn_800C4568(fn_80021B08(), collider);

            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->field_0x5C == 1) {
                    Player::Player *player = dynamic_cast<Player::Player *>(charInfo.get());

                    if (player != 0 && player->field_0x54 >= 0) {
                        UnkObject_8003A52C *object = lbl_8051CE88[player->field_0x54];

                        if (object != 0) {
                            for (int i = 0; i < 4; i++) {
                                if (m_params.blocks[i] >= 0) {
                                    if (m_params.onOff != 0) {
                                        fn_8003AAB0(object, m_params.blocks[i], 1);
                                    } else {
                                        fn_8003AAB0(object, m_params.blocks[i], 2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void BlockChange::Update(float dt) {
    if (m_owner != 0) {
        if (fn_800C9D0C(m_owner) && fn_800C9D14(m_owner)) {
            /* The original reads the position twice: here and again in ReadOwner (0x800ED490,
             * 0x800ED4A0). */
            fn_800C9F14(m_owner, &m_position);
            ReadOwner(m_owner);
            m_probe->SetPosition(&m_position);
            m_probe->SetSize(m_params.radius, -2, 0.0f, 0.0f);
            fn_800C9CF8(m_owner);
        }

        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
            return;
        }
        if (fn_800C996C(m_owner)) {
            m_flags |= 1;
            return;
        }
    }
    ProcessHits(dt);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Type", 0, 0, 0, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "On/Off", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "Radius", 0, 0, 0, 10.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {1, "BLOCK", 0xFFFFFFFF, 0xFFFFFFFF, 99, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "BLOCK", 0xFFFFFFFF, 0xFFFFFFFF, 99, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "BLOCK", 0xFFFFFFFF, 0xFFFFFFFF, 99, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "BLOCK", 0xFFFFFFFF, 0xFFFFFFFF, 99, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80524BD4 = {
    "BlockChange", fn_800ED9E4, fn_800ED9E0, BlockChange::Create, 0, 0, 0x59, 1, 0, 10, s_paramInfo,
};
