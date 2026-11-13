/* The DarkSpinEntrance gadget (class described in Game/DarkSpinEntrance.h). File name inferred from
 * the class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/DarkSpinEntrance.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

const char *DarkSpinEntrance::s_name = "DarkSpinEntrance";

/* Three angles (in radians) built at startup; the constructor passes them to fn_800C7A84. */
static UnkRotation3 lbl_80584068(0.5f * lbl_805FAD38, 0.0f, 0.0f);

DarkSpinEntrance::DarkSpinEntrance(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_radius, sizeof(m_radius));

    /* The original calls fn_8015CB88 twice with the same arguments. It keeps the first result in
     * this local until the constructor returns, and nothing reads it. */
    boost::shared_ptr<GadgetCharInfo> charInfo = fn_8015CB88(0x86, 4, "DarkSpinEntrance");
    m_charInfo = fn_8015CB88(0x86, 4, "DarkSpinEntrance");
    m_charInfo->field_0x30 = 2;

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 2.5f, m_radius, 0x40000,
                    10, 0, &lbl_805E07A0, &lbl_80584068, 0);
    m_handle = fn_8015B8C4(probe);
    m_handle->ClearFlags(2);
    ReadOwner(m_owner);
}

DarkSpinEntrance::~DarkSpinEntrance() {}

void DarkSpinEntrance::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);
}

void DarkSpinEntrance::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateProbe();
}

void DarkSpinEntrance::fn_800CBC1C(float) {
    fn_80258910();
}

void DarkSpinEntrance::UpdateProbe() {
    m_handle->m_probe->SetSize(m_radius, -2, 0.0f, 2.5f);
}

void DarkSpinEntrance::fn_80258910() {
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());

        Fn8040Matrix34 matrix(fn_8040B034(m_charInfo->m_position));
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 position = matrix * Fn8040Vector3(0.0f, 0.0f, 0.5f * -m_radius);
        fn_802E0D6C(&position, &m_charInfo->m_rotation, 2, 0.5f * m_radius, 1.25f);
    }
}

void DarkSpinEntrance::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_radius, sizeof(m_radius));
}

extern "C" void fn_802588B4(void) {}

extern "C" void fn_802588B0(void) {}

void DarkSpinEntrance::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DarkSpinEntrance(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "radius", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80549ED8 = {
    "DARKSPINENTRANCE", fn_802588B4, fn_802588B0, DarkSpinEntrance::Create, 0, 0, 0x61, 1, 0, 10,
    s_paramInfo,
};
