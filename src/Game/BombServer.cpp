/* BombServer (class name from RTTI; the gadget's descriptor is "BombServer"). File name inferred
 * from the class. Method names marked "(name inferred)" in the header are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/BombServer.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800EDA3C.h"
#include "Game/fn_801072C4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The radius of the probe sphere (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_probeRadius(20.0f);

const char *BombServer::s_name = "BombServer";

BombServer::BombServer(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(1) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x5D, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&BombServer::OnCommand, this, _1));
    ReadOwner(owner);
    m_model = fn_8015F800(1, "BOMBSERVER.BIN");
    SetupModel();
}

BombServer::~BombServer() {}

extern "C" void fn_803158AC(void) {
    fn_8006B474("BOMBSERVER.BIN", 1);
}

extern "C" void fn_803158A8(void) {}

void BombServer::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BombServer(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void BombServer::UpdateBody(float dt) {
    switch (m_state) {
    case 1: {
        /* Set to false when a character info in the probe sphere blocks the round (inferred). */
        bool clear = true;
        UnkProbeList_80021FE4 *hits = m_probe->GetHits();

        if (hits != 0) {
            for (UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
                 iterator.node != hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
                boost::shared_ptr<CharInfo> charInfo =
                    fn_800C4568(fn_80021B08(), fn_800587A0(&iterator.node->target)).lock();

                if (charInfo.get() != 0) {
                    if (charInfo->fn_80078B2C()) {
                        clear = false;
                        break;
                    }
                    /* 0xE and 0x4E are the ids that the Bomb and HealingUnit constructors
                     * (0x800F0BD4, 0x80109484) pass to GadgetCharInfo's: an item already in the
                     * sphere blocks the next round (inferred). */
                    if (charInfo->field_0x50 == 0x4E || charInfo->field_0x50 == 0xE) {
                        clear = false;
                        break;
                    }
                }
            }
        }

        if (clear) {
            m_timer -= dt;
            if (m_timer < 0.0f) {
                m_probe->UnkSlot20(0x10302, -1);
                m_target->fn_80167448(1);
                fn_801D54F4(fn_80053164(), 0x764F, &m_charInfo->m_position, 0, 1);
                m_timer = 1.0f;
                m_state = 2;
            }
        }
        break;
    }

    case 2:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_probe->UnkSlot2(-1);
            if (m_params.type == 0) {
                fn_800EDA3C(&m_charInfo->m_position, &m_charInfo->m_rotation,
                            boost::weak_ptr<CharInfo>(m_charInfo), fn_800C9938(m_owner));
                fn_801D54F4(fn_80053164(), 0x7632, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801072C4(&m_charInfo->m_position, &m_charInfo->m_rotation,
                            boost::weak_ptr<CharInfo>(m_charInfo), fn_800C9938(m_owner));
                fn_801D54F4(fn_80053164(), 0x7633, &m_charInfo->m_position, 0, 1);
            }
            m_timer = 0.5f;
            m_state = 3;
        }
        break;

    case 3:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            fn_801D54F4(fn_80053164(), 0x7634, &m_charInfo->m_position, 0, 1);
            m_state = 0;
        }
        break;
    }

    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void BombServer::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    BombServerParams previous = m_params;

    ReadOwner(owner);

    if (m_params.type != previous.type) {
        m_model->fn_80162804();
        m_state = 1;
        m_timer = 0.0f;
        SetupModel();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void BombServer::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_timer = 0.0f;
}

void BombServer::SetupModel() {
    m_model->fn_80162A68(m_params.type, 0);
    UpdateModelMatrix();

    m_target = fn_80166AF4();

    /* The holder is destroyed before the call to fn_80167180. */
    {
        UnkWeakHolder_800CC6C8 weakTarget;

        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }

    m_target->fn_80167180(2);

    m_probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x50100, 8, 0,
                          &lbl_805E07A0, s_probeRadius.value);
    if (m_state != 1) {
        m_probe->UnkSlot2(-1);
    }
}

void BombServer::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
}

void BombServer::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0xD:
        if (m_state == 0 || m_state == 3) {
            m_state = 1;
            m_probe->UnkSlot1(-1);
            m_probe->UnkSlot20(0x50100, -1);
            m_timer = 4.0f;
        }
        break;
    }
    command.m_flags |= 2;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Type 0:Bomb 1:HealUnit", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055BFB4 = {
    "BombServer", fn_803158AC, fn_803158A8, BombServer::Create, 0, 0, 0x1006, 1, 0, 10, s_paramInfo,
};
