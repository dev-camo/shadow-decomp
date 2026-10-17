/* ElecWakka (class name from RTTI): a gadget of the electric circuit area that shows the WAKKA.BIN
 * model turning about its vertical axis by half a turn per unit of dt. File name inferred from the
 * class. Method names marked "(name inferred)" are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

class ElecWakka : public Gadget {
  public:
    ElecWakka(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecWakka();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation and y rotation (name inferred). */
    void UpdateModelMatrix();
    /* Turns the character info about its y axis by half a turn per unit of dt (name inferred). */
    void Rotate(float dt);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ GadgetModelInfo *m_model;
    /* 0x3C */ u32 field_0x3C;
};
CHECK_SIZEOF(ElecWakka, 0x40);

const char *ElecWakka::s_name = "ElecWakka";

ElecWakka::ElecWakka(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecWakka");
    m_model = new GadgetModelInfo(1, "WAKKA.BIN");
    m_model->fn_80162A68(0, 0);
    CopyFromOwner(m_owner);
    UpdateModelMatrix();
}

ElecWakka::~ElecWakka() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecWakka::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    Rotate(dt);
    UpdateModelMatrix();
}

void ElecWakka::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
    }
}

void ElecWakka::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecWakka::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        MakeTranslationMatrix(&translation, m_charInfo->m_position);
        fn_8040C148(&matrix, &translation);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecWakka::Rotate(float dt) {
    float turns = 0.5f;

    m_charInfo->m_rotation.y += dt * (2.0f * lbl_805FAD38 * turns);
}

extern "C" void fn_80231398(void) {
    fn_8006B474("WAKKA.BIN", 1);
}

extern "C" void fn_80231394(void) {}

void ElecWakka::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecWakka(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80547000 = {
    "ELEC_WAKKA", fn_80231398, fn_80231394, ElecWakka::Create, 0, 0, 0x7E5, 1, 0, 10, 0,
};
