/* GrindGadget (class name from RTTI): a gadget that shows the GRINDGADGET.BIN model and, from the
 * first update, sets up a PathCtrl for its character info with one of two PTB files that the
 * gadget type's hookA (fn_80228348) selects by the word at 0xAC of lbl_8057D6A0, probably the
 * stage (behaviour inferred). File name inferred from the class. Method names marked "(name
 * inferred)" are not original. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * NonMatching: hookA (fn_80228348) differs in 7 of its 23 words (the registers of the file pointer
 * and of the word at 0xAC, and one li placed earlier). */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PathCtrl.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class GrindGadget : public Gadget {
  public:
    GrindGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GrindGadget();
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
    /* 0x40 */ s32 field_0x40;
    /* 0x44 */ UnkBitFlags_80014E04 m_pathFlags; /* bit 0: the path needs setting up (inferred) */
    /* 0x48 */ SonicteamUSA::System::PathCtrl m_path;
};
CHECK_SIZEOF(GrindGadget, 0xA0);

/* The PTB files that hookA chooses between by the word at 0xAC of lbl_8057D6A0 (names of
 * the pointers inferred). */
static const char *const s_fileA = "A10000GRINDOBJ_GR.PTB";
static const char *const s_fileB = "A20401GRINDOBJ_GR.PTB";
static const char *const *s_file = &s_fileA;

const char *GrindGadget::s_name = "GrindGadget";

GrindGadget::GrindGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), field_0x40(1), m_pathFlags(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x63, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, "GRINDGADGET.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));
    UpdateModelMatrix();
    m_pathFlags.Set(0);
}

GrindGadget::~GrindGadget() {
    if (fn_800ABDCC(&m_path)) {
        fn_8001E2DC(fn_8001B494(), &m_path);
    }
}

extern "C" void fn_80228348(void) {
    const char *const *file = &s_fileA;

    if (lbl_8057D6A0.field_0xAC == 13) {
        file = &s_fileB;
    }
    s_file = file;
    fn_8006B474("GRINDGADGET.BIN", 1);
    fn_8006B474(*s_file, 0);
}

extern "C" void fn_80228344(void) {}

void GrindGadget::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GrindGadget(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void GrindGadget::UpdateBody(float dt) {
    if (m_pathFlags.Test(0)) {
        if (fn_800ABDCC(&m_path)) {
            fn_8001E2DC(fn_8001B494(), &m_path);
        }

        UnkPathData_8001DB08 *resource =
            static_cast<UnkPathData_8001DB08 *>(fn_80012700(fn_800122D4(), 1, *s_file));
        fn_8001DB08(fn_8001B494(), resource, &m_charInfo->m_position, m_charInfo->m_rotation,
                    m_charInfo->field_0x30, &m_path, &lbl_80570B50);
        m_pathFlags.Reset(0);
    }
    m_model->fn_80162FB0(dt);
}

void GrindGadget::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
    m_pathFlags.Set(0);
}

void GrindGadget::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void GrindGadget::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
}

GadgetTypeInfo lbl_80546700 = {
    "GRINDGADGET", fn_80228348, fn_80228344, GrindGadget::Create, 0, 0, 0x138D, 1, 0, 10, 0,
};
