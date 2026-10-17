/* ElecSmallPillar (class name from RTTI): a static gadget of the electric circuit area that shows
 * the SMALLPILLAR.BIN model at the owner's position and rotation. File name inferred from the
 * class. Method names marked "(name inferred)" are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

class ElecSmallPillar : public Gadget {
  public:
    ElecSmallPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecSmallPillar();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the pillar under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation and rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ GadgetModelInfo *m_model;
    /* 0x3C */ u32 field_0x3C;
};
CHECK_SIZEOF(ElecSmallPillar, 0x40);

const char *ElecSmallPillar::s_name = "ElecSmallPillar";

ElecSmallPillar::ElecSmallPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecSmallPillar");
    m_model = new GadgetModelInfo(1, "SMALLPILLAR.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    CopyFromOwner(m_owner);
    UpdateModelMatrix();
}

ElecSmallPillar::~ElecSmallPillar() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecSmallPillar::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    UpdateModelMatrix();
}

void ElecSmallPillar::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecSmallPillar::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecSmallPillar::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_80230FF0(void) {
    fn_8006B474("SMALLPILLAR.BIN", 1);
}

extern "C" void fn_80230FEC(void) {}

void ElecSmallPillar::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecSmallPillar(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80546F98 = {
    "ELEC_SMALLPILLAR", fn_80230FF0, fn_80230FEC, ElecSmallPillar::Create, 0, 0, 0x7E7, 1, 0, 10, 0,
};
