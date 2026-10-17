/* ElecPillar (class name from RTTI): a static gadget of the electric circuit area that shows the
 * PILLAR.BIN model at the owner's position and rotation. File name inferred from the class. Method
 * names marked "(name inferred)" are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

class ElecPillar : public Gadget {
  public:
    ElecPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecPillar();
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
CHECK_SIZEOF(ElecPillar, 0x40);

const char *ElecPillar::s_name = "ElecPillar";

ElecPillar::ElecPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecPillar");
    m_model = new GadgetModelInfo(1, "PILLAR.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    CopyFromOwner(m_owner);
    UpdateModelMatrix();
}

ElecPillar::~ElecPillar() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecPillar::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    UpdateModelMatrix();
}

void ElecPillar::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecPillar::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecPillar::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        MakeTranslationMatrix(&translation, m_charInfo->m_position);
        fn_8040C148(&matrix, &translation);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_8021C604(void) {
    fn_8006B474("PILLAR.BIN", 1);
}

extern "C" void fn_8021C600(void) {}

void ElecPillar::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecPillar(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_805459A8 = {
    "ELEC_PILLAR", fn_8021C604, fn_8021C600, ElecPillar::Create, 0, 0, 0x7E3, 1, 0, 10, 0,
};
