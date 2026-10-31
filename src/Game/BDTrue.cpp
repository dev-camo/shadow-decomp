/* BDTrue::BDTrue (class name from RTTI): a gadget, in the namespace BDTrue, that shows the
 * BDTRUE.BIN model at the owner's position, turned about the y axis by an angle that it reads
 * every frame from fn_80109B80. File name inferred from the class. Method names marked "(name
 * inferred)" are not original. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

static const char *s_modelFile = "BDTRUE.BIN";

namespace BDTrue {

class BDTrue : public Gadget {
  public:
    BDTrue(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BDTrue();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation, turned by the y angle of the vector
     * that fn_80109B80 returns (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ s32 m_state; /* 0 until the first update after construction or a reset (inferred) */
    /* 0x4C */ u32 field_0x4C;
};
CHECK_SIZEOF(BDTrue, 0x50);

const char *BDTrue::s_name = "BDTrue";

BDTrue::BDTrue(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8C, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80160A84(0, 1);
    if (m_model.get() != 0) {
        m_target = fn_80166AF4();

        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
    if (m_target.get() != 0) {
        m_target->fn_80167010(-1);
    }
    UpdateModelMatrix();
}

BDTrue::~BDTrue() {}

void BDTrue::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    /* The original's compare tree needs the labels 1 and 2 beside 0 (other label sets compile
     * differently); states 1 and 2 do nothing here. */
    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 1:
    case 2:
        break;
    }

    UpdateModelMatrix();
}

void BDTrue::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
}

void BDTrue::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
}

void BDTrue::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Vector3 angles = fn_80109B80(fn_80009548(), 0);
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, angles.y);
        m_model->fn_801633DC(&matrix);
    }
}

} // namespace BDTrue

extern "C" void fn_80303CD0(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_80303CCC(void) {}

void BDTrue::BDTrue::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BDTrue(parent, owner);
}

GadgetTypeInfo lbl_8055AEC0 = {
    "BD True", fn_80303CD0, fn_80303CCC, BDTrue::BDTrue::Create, 0, 0, 0x1B58, 1, 0, 10, 0,
};
