/* The Switch gadget (class described in Game/Switch.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the model and to the attack object are boost::function objects built
 * with boost::bind(&Switch::method, this, _1); their manager, invoker and copy functions come from
 * the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Switch.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

namespace Switch {

const char *Switch::s_name = "Switch";

Switch::Switch(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner), m_type(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(9, 4);

    m_model = fn_8015F9D8(0, 0xE1);
    m_model->fn_80162A68(0, 0);
    m_onModel = fn_8015F9D8(0, 0xE1);
    m_onModel->fn_80162A68(1, 0);
    m_offModel = fn_8015F9D8(0, 0xE1);
    m_offModel->fn_80162A68(2, 0);

    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->field_0x10 = ColliDetectCallback(boost::bind(&Switch::OnTouch, this, _1));
    m_model->field_0x1C = ColliDetectCallback(boost::bind(&Switch::OnRelease, this, _1));

    boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(0);

    if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Switch::OnAttack, this, _1));
    }

    m_target = fn_801E72DC(0x800);
    m_comId = 0;
    ReadOwner(m_owner);
    UpdateModelMatrix();
    ShowState();
}

Switch::~Switch() {
    fn_801E7108(m_target);
}

void Switch::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    m_flags.Reset(0);
}

void Switch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
    }
}

void Switch::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
    fn_800C9DBC(owner, &m_type, sizeof(m_type));
    m_flags.Clear();
}

void Switch::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

    if (m_model.get() != 0) {
        m_model->fn_801633DC(&matrix);
    }

    if (m_onModel.get() != 0) {
        m_onModel->fn_801633DC(&matrix);
    }

    if (m_offModel.get() != 0) {
        m_offModel->fn_801633DC(&matrix);
    }

    if (m_target.get() != 0) {
        Fn8040Vector3 position = m_charInfo->m_position;

        position.y += 5.0f;
        fn_800CCE38(m_target.get(), &position);
    }
}

void Switch::OnTouch(ColliDetectArgs &args) {
    if (m_flags.Test(0)) {
        return;
    }

    switch (m_type) {
    case TYPE_ON_OFF:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            fn_801690D0(fn_800CB69C(), m_comId, 0);
        } else {
            fn_801690D0(fn_800CB69C(), m_comId, 1);
        }

        fn_801D54F4(fn_80053164(), 0xE016, &m_charInfo->m_position, 0, 1);
        break;

    /* Matching workaround: the two cases repeat the same code, as the original does (it has a
     * copy of the block for each, at 0x80123428 and 0x8012346C); one block under both labels is
     * 20 words shorter. Revisit if the two cases turn out to differ in a way this code hides. */
    case TYPE_ON_TOUCH:
        if (fn_8016904C(fn_800CB69C(), m_comId) != 1) {
            fn_801690D0(fn_800CB69C(), m_comId, 1);
            fn_801D54F4(fn_80053164(), 0xE016, &m_charInfo->m_position, 0, 1);
        }
        break;

    case TYPE_ON_ALWAYS:
        if (fn_8016904C(fn_800CB69C(), m_comId) != 1) {
            fn_801690D0(fn_800CB69C(), m_comId, 1);
            fn_801D54F4(fn_80053164(), 0xE016, &m_charInfo->m_position, 0, 1);
        }
        break;
    }

    ShowState();
    m_flags.Set(0);
}

void Switch::OnRelease(ColliDetectArgs &args) {
    /* Matching workaround: the switch lists all three values of the "type" parameter, the same
     * three that OnTouch switches on (OnAttack switches on TYPE_ON_OFF and TYPE_ON_ALWAYS), and
     * the TYPE_ON_OFF and TYPE_ON_ALWAYS cases are empty: a release changes nothing for those
     * types. The empty cases are there only because they give the original's comparison tree
     * (0x80123340-0x80123350: cmpwi 1, a beq to the TYPE_ON_TOUCH code, then bge, b and a
     * second b past the switch). With the TYPE_ON_TOUCH case alone the function is 0x50 bytes
     * against the original's 0x58 (2 words shorter), and an if is 0x4C. Revisit if another
     * form gives the same tree. */
    switch (m_type) {
    case TYPE_ON_OFF:
        break;
    case TYPE_ON_TOUCH:
        fn_801690D0(fn_800CB69C(), m_comId, 0);
        break;
    case TYPE_ON_ALWAYS:
        break;
    }

    ShowState();
}

void Switch::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_flags.Test(0)) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x240) == 0) {
            /* Matching workaround: 5 is stored again here, as in the original (stw at 0x80123204
             * and again at 0x80123268); without this store OnAttack is 2 words shorter. Revisit
             * when the original form of this check is known. */
            param.field_0x54 = 5;
            return;
        }
    }

    param.field_0x54 = 1;

    switch (m_type) {
    case TYPE_ON_OFF:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            fn_801690D0(fn_800CB69C(), m_comId, 0);
        } else {
            fn_801690D0(fn_800CB69C(), m_comId, 1);
        }
        break;

    case TYPE_ON_ALWAYS:
        fn_801690D0(fn_800CB69C(), m_comId, 1);
        break;
    }

    ShowState();
    m_flags.Set(0);
}

void Switch::ShowState() {
    if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
        m_onModel->fn_8016271C();
        m_offModel->fn_80162790();
    } else {
        m_onModel->fn_80162790();
        m_offModel->fn_8016271C();
    }
}

} // namespace Switch

extern "C" void fn_8012315C(void) {
    fn_8006B50C(0xE1, 1);
}

extern "C" void fn_80123158(void) {}

void Switch::Switch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Switch(parent, owner);
}

/* The names of the values of the "type" parameter. */
static const char *s_typeLabels[3] = {"type : OnOff", "type : OnTouch", "type : OnAlways"};

/* The label callback of the descriptor's parameter (behaviour inferred). */
static const char *fn_801230D4(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_801230D4, 0},
    {0},
};

GadgetTypeInfo lbl_80527B40 = {
    "SWITCH", fn_8012315C, fn_80123158, Switch::Switch::Create, 0, 0, 0x15, 1, 0, 10, s_paramInfo,
};
