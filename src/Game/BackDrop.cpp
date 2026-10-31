/* BackDrop (class name from RTTI): a gadget that shows the BACKDROP.BIN model rising at a fixed
 * speed from the owner's position and jumping back to it once it is a fixed distance above. File
 * name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

class BackDrop : public Gadget {
  public:
    BackDrop(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BackDrop();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation and rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
};
CHECK_SIZEOF(BackDrop, 0x40);

const char *BackDrop::s_name = "BackDrop";

/* The original sets these two floats in the static initialiser instead of placing them in .sdata;
 * their declared type is unknown. The shared startup float class reproduces this; a plain float
 * initialiser would place them in .sdata. */
static UnkStartupFloat_800E2570 s_riseSpeed(100.0f);
static UnkStartupFloat_800E2570 s_riseLimit(400.0f);

BackDrop::BackDrop(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x6A, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, "BACKDROP.BIN");
    m_model->fn_80162A68(0, 0);
}

BackDrop::~BackDrop() {}

extern "C" void fn_8026B59C(void) {
    fn_8006B474("BACKDROP.BIN", 1);
}

extern "C" void fn_8026B598(void) {}

void BackDrop::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BackDrop(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8054C790 = {
    "BACKDROP", fn_8026B59C, fn_8026B598, BackDrop::Create, 0, 0, 0x189C, 1, 0, 10, 0,
};

void BackDrop::UpdateBody(float dt) {
    Fn8040Vector3 ownerPosition;

    m_charInfo->m_position.y += s_riseSpeed.value * dt;
    fn_800C9F14(m_owner, &ownerPosition);
    if (m_charInfo->m_position.y - ownerPosition.y > s_riseLimit.value) {
        m_charInfo->m_position.y = ownerPosition.y;
    }
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void BackDrop::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
}

void BackDrop::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void BackDrop::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
}
