/* ElecBomb (class name from RTTI; the gadget's descriptor is "ELEC_BOMB"): a bomb of the electric
 * circuit area that shows ELECBOMB.BIN. When a character comes near, it plays a sound and switches
 * parts of the model; on the next frame it explodes: it passes -1 to its target's fn_80167448,
 * calls the actor that fn_800740E4 returns for 0, releases its object, stores state 2 in the
 * owner's ElecBombData and calls fn_800C985C, fn_8000D664 and fn_8034DE74 (behaviour inferred).
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/ElecBomb.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* The result type is CircusBall's placeholder for the result of fn_80169BB0: both results are
 * destroyed with the same weak pointer destructor (0x800767FC). */
class UnkObject_80169BB0;

extern "C" {
/* Returns a weak pointer to an object (purpose and arguments not understood); the bomb ignores
 * the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DE74(int arg0, int arg1, float arg2);
}

const char *ElecBomb::s_name = "ElecBomb";

/* The part callback (placeholder name): for a part whose bit (1 << its word at 0x8) is set in the
 * mask passed as the user data, calls fn_80038F94 on its clump when bit 0x8000 of the mask is set
 * and fn_80038FC0 otherwise (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_80263F38(UnkModelPart_8015FA78 *part, void *userData);

ElecBomb::ElecBomb(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x56, 4, "ElecBomb");
    ReadOwner(owner);

    if (owner->m_adapterState == 0) {
        ElecBombData *data = new ElecBombData;

        fn_800C9930(owner, data);
        data->m_state = 0;
        m_state = 0;
    } else {
        m_state = dynamic_cast<ElecBombData *>(m_owner->m_adapterState)->m_state;
    }

    m_model = new GadgetModelInfo(1, "ELECBOMB.BIN");
    m_model->fn_80162A68(0, 0);

    if (m_state == 0) {
        if (m_model != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x4));
        m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x8003));
        m_object = fn_802B4254(0, 0);
        fn_802B4498(m_object.get(), 1200.0f);
        fn_802B4510(m_object.get(), 1);
    } else {
        m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x3));
        m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x8004));
    }

    UpdateModelMatrix();
}

ElecBomb::~ElecBomb() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecBomb::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (IsCharacterNear()) {
            m_state = 1;
            m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x3));
            m_model->fn_8015FA78(fn_80263F38, reinterpret_cast<void *>(0x8004));
            fn_801D560C(fn_80053164(), 0x7248, 0, 0);
        }
        break;
    case 1: {
        m_target->fn_80167448(-1);
        {
            boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

            if (actor.get() != 0) {
                actor->UnkSlot7(1, 0);
            }
        }
        m_object.reset();
        m_state = 2;

        ElecBombData *data = dynamic_cast<ElecBombData *>(m_owner->m_adapterState);

        data->m_state = 2;
        fn_800C985C(m_owner);
        fn_8000D664(fn_80009548(), 0, 0.35f, 2.2f, 10.0f);
        fn_8034DE74(8, -2, 2.0f);
        break;
    }
    case 2:
        break;
    }

    UpdateModelMatrix();
}

void ElecBomb::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    m_state = 0;
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
    }
}

void ElecBomb::fn_800CBC1C(float) {
    if (m_model != 0) {
        fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, 150.0f);
    }
}

bool ElecBomb::IsCharacterNear() {
    boost::shared_ptr<CharInfo> character;

    character = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();
    if (character.get() != 0) {
        Fn8040Vector3 difference;

        fn_8040CD74(&difference, &m_charInfo->m_position, &character->m_position);
        if (fn_8040CF2C(&difference) <= 22500.0f) {
            fn_8020642C(character, 0x35);
            return true;
        }
        return false;
    }
    return false;
}

void ElecBomb::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecBomb::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_object.get() != 0) {
        Fn8040Vector3 above(0.0f, 170.0f, 0.0f);
        Fn8040Vector3 position;

        fn_8040CDD0(&position, &above, &m_charInfo->m_position);
        fn_802B44C8(m_object.get(), &position);
    }
}

/* Matching workaround: the callback returns part both inside the masked branch and at the end.
 * The original's 23 instruction words need that layout. With one trailing return, 9 words
 * change: the shift constant 1 goes into r3 after part is saved instead of into r5 before it
 * (USA 0x80263F40), the field load uses r31 instead of r3, and the masked-out branch can no
 * longer skip the move of part back into r3 (in the original, r3 still holds part there), so it
 * targets +0x44 instead of +0x48 and the epilogue order changes. A guard-and-return form gives
 * 24 words. Revisit when the original form of these part callbacks is found (the ElecSearchLight
 * callback has the same pattern). */
static UnkModelPart_8015FA78 *fn_80263F38(UnkModelPart_8015FA78 *part, void *userData) {
    u32 mask = reinterpret_cast<u32>(userData);

    if ((1 << part->field_0x8) & mask) {
        if (mask & 0x8000) {
            fn_80038F94(part->clump);
        } else {
            fn_80038FC0(part->clump);
        }
        return part;
    }
    return part;
}

extern "C" void fn_80263F0C(void) {
    fn_8006B474("ELECBOMB.BIN", 1);
}

extern "C" void fn_80263F08(void) {}

void ElecBomb::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecBomb(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8054BFBC = {
    "ELEC_BOMB", fn_80263F0C, fn_80263F08, ElecBomb::Create, 0, 0, 0x7DC, 1, 0, 10, 0,
};
