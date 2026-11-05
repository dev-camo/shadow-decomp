/* ElecSecurityCodeGenerator (class name from RTTI; the gadget's descriptor is "ELEC_SECURITY"): the
 * gadget that starts the ElecSecurityCode objects of the electric circuit area. File name inferred
 * from the class. Method names marked "(name inferred)" are not original. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/ElecSecurityCodeGenerator.h"
#include "Game/DebugDisp.h"
#include "Game/ElecSecurityCode.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkOwnerSet_80260E48.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

const char *ElecSecurityCodeGenerator::s_name = "ElecSecurityCodeGenerator";

/* The owners whose kind is 0x7D4, the ELEC_SECURITY descriptor's id (inferred: the stage's
 * gadgets of this type), created when the gadget type is loaded. */
static UnkOwnerSet_80260E48 *s_ownerSet;

ElecSecurityCodeGenerator::ElecSecurityCodeGenerator(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CB88(0x57, 4, "ElecSecurityCodeGenerator");
    m_loadedType = m_params.type;
    m_model = 0;
    CopyFromOwner(m_owner);
    UpdateModelMatrix();
    m_state = 0;
    s_ownerSet->AddOwner(owner);
}

ElecSecurityCodeGenerator::~ElecSecurityCodeGenerator() {
    std::list<ElecSecurityCode *>::iterator it;

    for (it = m_codes.begin(); it != m_codes.end(); it++) {
        delete *it;
    }

    s_ownerSet->RemoveOwner(m_owner);
}

void ElecSecurityCodeGenerator::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (IsCharInfoInRange()) {
            boost::shared_ptr<CharInfo> nearest =
                fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();
            Fn8040Vector3 direction;
            Fn8040Vector3 difference;

            fn_8000AD64(&difference, &nearest->m_position, &m_charInfo->m_position);
            fn_8040CEE8(&direction, &difference);

            Fn8040Vector2 rotation = fn_8040F760(&direction);

            m_codes.push_back(
                new ElecSecurityCode(m_loadedType, m_charInfo->m_position, this, rotation, 0));
            m_state = 1;
        }
        break;

    case 1:
        if (m_codes.size() == 0) {
            if (s_ownerSet->RemoveOwner(m_owner) && m_comId != 0) {
                fn_801690D0(fn_800CB69C(), m_comId, 1);
            }

            if (field_0x3C.get() != 0) {
                if (lbl_8057D6A0.field_0xA8 == 200) {
                    fn_8020642C(field_0x3C, 0x31);
                } else {
                    fn_8020642C(field_0x3C, 0x32);
                }
            }

            fn_8015A794(this);
            m_flags |= 1;
        } else {
            std::list<ElecSecurityCode *>::iterator it;

            /* As shipped, the it++ after an erase skips the code that follows a deleted one, and
             * after deleting the last code it steps from end() round to the first code. */
            for (it = m_codes.begin(); it != m_codes.end(); it++) {
                if (!(*it)->fn_80156C78(dt)) {
                    delete *it;
                    it = m_codes.erase(it);
                }
            }
        }
        break;
    }
}

void ElecSecurityCodeGenerator::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);

    if (m_model != 0) {
        if (m_params.type != m_loadedType) {
            m_model->fn_80162804();
            m_loadedType = m_params.type;
            m_model->fn_80162A68(m_loadedType, 0);
            m_params.type = m_loadedType;
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }
    }
}

void ElecSecurityCodeGenerator::fn_800CBC1C(float) {
    if (m_model == 0) {
        m_model = new GadgetModelInfo(1, "SECURITY.BIN");
        m_loadedType = m_params.type;
        m_model->fn_80162A68(m_loadedType, 0);
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.range);
}

void ElecSecurityCodeGenerator::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
}

void ElecSecurityCodeGenerator::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        m_model->fn_801633DC(&matrix);
    }
}

bool ElecSecurityCodeGenerator::IsCharInfoInRange() {
    boost::shared_ptr<CharInfo> nearest =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (nearest.get() != 0) {
        float rangeSquared = m_params.range * m_params.range;
        Fn8040Vector3 offset;

        fn_8040CD74(&offset, &m_charInfo->m_position, &nearest->m_position);
        return fn_8040CF2C(&offset) <= rangeSquared;
    }

    return false;
}

/* The descriptor's first hook: queues SECURITY.BIN for loading and creates the owner set for the
 * kind 0x7D4, the descriptor's id. */
extern "C" void fn_80157C2C(void) {
    fn_8006B474("SECURITY.BIN", 1);
    s_ownerSet = new UnkOwnerSet_80260E48(0x7D4);
}

/* The descriptor's second hook: destroys the set. */
extern "C" void fn_80157C04(void) {
    delete s_ownerSet;
}

void ElecSecurityCodeGenerator::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecSecurityCodeGenerator(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "range", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052AAF0 = {
    "ELEC_SECURITY",
    fn_80157C2C,
    fn_80157C04,
    ElecSecurityCodeGenerator::Create,
    0,
    0,
    0x7D4,
    1,
    0,
    10,
    s_paramInfo,
};
