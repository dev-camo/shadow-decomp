/* ElecKyuu (class name from RTTI; the gadget's descriptor is "ELEC_KYUU"): a gadget of the electric
 * circuit area. It loads the clump of DE0403KYUU1.DFF, turns about its y axis and, through the
 * callback it hands to a draw layer, draws that clump 16 times around its y axis when it is in
 * view. It also creates a probe sphere around its character info (behaviour inferred). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/ElecKyuu.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800E9CEC.h"
#include "rw/rpworld.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

const char *ElecKyuu::s_name = "ElecKyuu";

ElecKyuu::ElecKyuu(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecKyuu");
    m_clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "DE0403KYUU1.DFF"));
    m_layer = fn_8004DFB0(fn_80046178()->field_0x8, "ElecKyuu");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ElecKyuu::OnDraw, this, _1));

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x20202, 10, 0,
                    &lbl_805E07A0, 302.5f);
    m_handle = fn_8015B8C4(probe);
    CopyFromOwner(m_owner);
}

ElecKyuu::~ElecKyuu() {}

void ElecKyuu::OnDraw(RednerDrawArgs &args) {
    RwV3d position = {m_charInfo->m_position.x, m_charInfo->m_position.y, m_charInfo->m_position.z};
    RwSphere sphere;

    sphere.radius = 302.5f;
    sphere.center = position;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);
        float angleStep = 2.0f * lbl_805FAD38 / 16.0f;

        for (int i = 0; i < 16; i++) {
            fn_800E9CEC(frame, rwCOMBINEREPLACE, i * angleStep + m_charInfo->m_rotation.y);
            fn_804875B0(frame, &position, rwCOMBINEPOSTCONCAT);
            fn_80456BC4(m_clump);
        }
    }
}

void ElecKyuu::UpdateBody(float dt) {
    Rotate(dt);
}

void ElecKyuu::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecKyuu::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecKyuu::Rotate(float dt) {
    m_charInfo->m_rotation.y += dt * (2.0f * lbl_805FAD38 / 3.0f);
}

extern "C" void fn_80230594(void) {
    fn_8006B474("DE0403KYUU1.DFF", 0);
}

extern "C" void fn_80230590(void) {}

void ElecKyuu::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecKyuu(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80546F34 = {
    "ELEC_KYUU", fn_80230594, fn_80230590, ElecKyuu::Create, 0, 0, 0x84A, 1, 0, 10, 0,
};
