/* ElecCircuit (class name from RTTI): a gadget of the electric circuit puzzle that shows the
 * CIRCUIT.BIN model. File name inferred from the class. Method names marked "(name inferred)" are
 * not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"

class UnkManager_800CB69C;

extern "C" {
/* Preloads a resource file (behaviour inferred). */
void fn_8006B474(const char *fileName, int arg1);
UnkManager_800CB69C *fn_800CB69C(void);
u32 fn_8016904C(UnkManager_800CB69C *manager, u8 id);
}

class ElecCircuit : public Gadget {
  public:
    ElecCircuit(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCircuit();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and byte setting into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation and rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ GadgetModelInfo *m_model;
    /* 0x3C */ u8 field_0x3C; /* copied from the owner; the circuit id it waits for (inferred) */
    /* 0x40 */ s32 m_state;   /* 0 new, 1 after a reset from the owner, 3 switched (inferred) */
    /* 0x44 */ u32 field_0x44;
};

const char *ElecCircuit::s_name = "ElecCircuit";

ElecCircuit::ElecCircuit(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecCircuit");
    m_model = new GadgetModelInfo(1, "CIRCUIT.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    CopyFromOwner(m_owner);
    m_state = 0;
    UpdateModelMatrix();
}

ElecCircuit::~ElecCircuit() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecCircuit::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (fn_8016904C(fn_800CB69C(), field_0x3C) == 1) {
            m_state = 3;
            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_model->fn_80162A68(1, 0);
        } else {
            m_state = 1;
        }
        break;
    case 1:
        if (fn_8016904C(fn_800CB69C(), field_0x3C) == 1) {
            m_state = 3;
            m_model->fn_80161880();
            m_model->fn_80162804();
            m_model->fn_80162A68(1, 0);
            m_model->fn_80160C28(0);
            m_model->fn_80160B18(0);
        }
        break;
    case 2: /* nothing to do in state 2 */
        break;
    }

    UpdateModelMatrix();
}

void ElecCircuit::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    m_state = 1;
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecCircuit::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo.px->m_position);
    fn_800C9ED4(owner, &m_charInfo.px->m_rotation);
    fn_800C9E60(owner, &field_0x3C);
}

void ElecCircuit::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo.px->m_position);
        fn_8040B848(&matrix, m_charInfo.px->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_801F7910(void) {
    fn_8006B474("CIRCUIT.BIN", 1);
}

extern "C" void fn_801F790C(void) {}

void ElecCircuit::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCircuit(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_805422C0 = {
    "ELEC_CIRCUIT", fn_801F7910, fn_801F790C, ElecCircuit::Create, 0, 0, 0x7D0, 1, 0, 10, 0,
};
