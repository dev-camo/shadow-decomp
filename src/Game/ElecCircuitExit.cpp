/* ElecCircuitExit (class name from RTTI): the exit gadget of the electric circuit puzzle.
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order: game C++ is compiled with deferred inlining, which
 * emits a file's functions last-defined first. */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <string.h>

class UnkHandleBTarget_8015BE64;

struct RwFrame;
struct RwMatrix;

extern "C" {
RwMatrix *RwFrameGetLTM(RwFrame *frame);
void fn_800635A4(void *dst, RwMatrix *matrix);
void fn_8015BE64(UnkHandleBTarget_8015BE64 *target);
}

/* fn_802C7FC0 to fn_802C8068 precede the gadget in this unit. Their owning class and real (C++)
 * link names are unknown, so they keep their placeholder names through C linkage. */

/* Five fields of unknown purpose, kept as a current and a saved copy. */
struct UnkStateFields_802C7FC0 {
    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ u8 field_0x8;
    /* 0x0C */ u32 field_0xC;
    /* 0x10 */ u8 field_0x10;
};

struct UnkState_802C7FC0 {
    /* 0x00 */ UnkStateFields_802C7FC0 cur;
    /* 0x14 */ UnkStateFields_802C7FC0 saved;
    /* 0x28 */ u8 dirty;
    /* 0x2C */ s32 index;
};

class ElecCircuitExit : public Gadget {
  public:
    ElecCircuitExit(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCircuitExit();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C();
    /* Re-reads the owner's settings (name inferred). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the exit gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and byte setting into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    void fn_802C8158();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ u8 field_0x38; /* copied from the owner */
    /* 0x3C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> field_0x3C;
    /* 0x44 */ boost::weak_ptr<GadgetCharInfo> field_0x44; /* pointee type inferred */
    /* 0x4C */ u32 field_0x4C;
};

const char *ElecCircuitExit::s_name = "ElecCircuitExit";

/* Three angles (in radians) built at startup; nothing in this unit reads them (purpose unknown). */
struct UnkAngles_80584590 {
    UnkAngles_80584590(float x, float y, float z) : x(x), y(y), z(z) {}

    float x;
    float y;
    float z;
};

static UnkAngles_80584590 lbl_80584590(0.5f * lbl_805FAD38, 0.0f, 0.0f);

ElecCircuitExit::ElecCircuitExit(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x59, 4, "ElecCircuitExit");
    CopyFromOwner(m_owner);
}

ElecCircuitExit::~ElecCircuitExit() {}

void ElecCircuitExit::UpdateBody(float dt) {}

void ElecCircuitExit::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecCircuitExit::fn_800CBC1C() {
    fn_802C8158();
}

void ElecCircuitExit::fn_802C8158() {
    if (field_0x3C.px != 0) {
        fn_8015BE64(field_0x3C.px);
    }
}

void ElecCircuitExit::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo.px->m_position);
    fn_800C9ED4(owner, &m_charInfo.px->m_rotation);
    fn_800C9E60(owner, &field_0x38);
}

extern "C" void fn_802C8100(void) {}

extern "C" void fn_802C80FC(void) {}

void ElecCircuitExit::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCircuitExit(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80557B00 = {
    "ELEC_EXIT", fn_802C8100, fn_802C80FC, ElecCircuitExit::Create, 0, 0, 0x7EA, 1, 0, 10, 0,
};

extern "C" void fn_802C8068(void *dst, RwFrame *frame) {
    RwMatrix *ltm = RwFrameGetLTM(frame);
    fn_800635A4(dst, ltm);
}

extern "C" void fn_802C8034(UnkState_802C7FC0 *state) {
    state->saved.field_0x0 = state->cur.field_0x0;
    state->saved.field_0x4 = state->cur.field_0x4;
    state->saved.field_0x8 = state->cur.field_0x8;
    state->saved.field_0xC = state->cur.field_0xC;
    state->saved.field_0x10 = state->cur.field_0x10;
    state->dirty = 0;
}

extern "C" void fn_802C7FF0(UnkState_802C7FC0 *state) {
    memset(&state->cur, 0, sizeof(state->cur));
    fn_802C8034(state);
    state->index = -1;
}

extern "C" UnkState_802C7FC0 *fn_802C7FC0(UnkState_802C7FC0 *state) {
    fn_802C7FF0(state);
    return state;
}
