/* The BREAK_BLAST gadget (class described in Game/BlastBreak.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The attack callback is a boost::function object built with
 * boost::bind(&BlastBreak::OnAttack, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/BlastBreak.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *BlastBreak::s_name = "BlastBreak";

BlastBreak::BlastBreak(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(4, 4, "BlastBreak");
    m_model = fn_8015F800(0, "BLASTBREAK.BIN");
    m_model->fn_80162A68(0, 0);

    if (m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo))) {
        boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161334("col");

        if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&BlastBreak::OnAttack, this, _1));
        }
    }

    ReadOwner(owner);
    UpdateModelMatrix();
}

BlastBreak::~BlastBreak() {}

void BlastBreak::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_RESET:
        m_state = STATE_WAITING;
        break;
    case STATE_HIT:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void BlastBreak::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_8016108C();
}

void BlastBreak::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = STATE_RESET;
}

void BlastBreak::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

void BlastBreak::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x10) == 0) {
                param.field_0x54 = 5;
                return;
            }

            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                fn_8020642C(attacker, 0x17);
            }

            param.field_0x54 = 3;
            m_state = STATE_HIT;
            return;
        }
    }

    param.field_0x54 = 5;
}

extern "C" void fn_800EC99C(void) {
    fn_8006B474("BLASTBREAK.BIN", 1);
}

extern "C" void fn_800EC998(void) {}

void BlastBreak::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BlastBreak(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_805249F4 = {
    "BREAK_BLAST", fn_800EC99C, fn_800EC998, BlastBreak::Create, 0, 0, 0x1C, 1, 0, 10, 0,
};
