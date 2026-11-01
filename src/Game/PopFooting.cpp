/* PopFooting::SetPopFooting (class name from RTTI; the gadget's descriptor is "POP FOOTING"): a
 * footing that moves from -s_hiddenDepth along its own z axis up to its position, waits, moves
 * back and waits, over and over (behaviour inferred). It shows the model POPFOOTING.BIN twice,
 * each copy with a character info of its own, and plays a sound while it is in motion. The
 * parameters are the model number and the waiting times. File name inferred from the class.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first).
 *
 * NonMatching: 11 of the 12 functions have identical instructions. UpdateBody differs in 13
 * words (counted as raw words of the 125): the original loads the two startup floats before the
 * stores to the gadget's members, so a load is scheduled earlier and the registers of the floats
 * differ in the case that hides the footing and in the final case. */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

static const char *s_modelFile = "POPFOOTING.BIN";

/* Two floats that the original builds at startup (names inferred from their uses): the distance
 * back along its own z axis at which the footing waits hidden, and the speed at which it moves. */
static UnkStartupFloat_800E2570 s_hiddenDepth(80.0f);
static UnkStartupFloat_800E2570 s_speed(10.0f);

namespace PopFooting {

class SetPopFooting : public Gadget {
  public:
    SetPopFooting(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetPopFooting();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the gadget, puts both character
     * infos at the position with the same rotation and restarts the cycle (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the first model at the first character info and the second at the second (name
     * inferred). */
    void UpdateModelMatrices();
    /* Moves the first character info to the position, shifted by the current height along the
     * z axis of its own rotation (name inferred). */
    void UpdateCharInfoPosition();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo2;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_model2;
    /* The parameter block (the "model", "sec_begin" and "sec_wait" entries of the descriptor's
     * table), which fn_800C9DBC reads from the owner. */
    /* 0x50 */ s32 m_modelIndex;
    /* 0x54 */ float m_secBegin; /* seconds hidden before moving out (inferred) */
    /* 0x58 */ float m_secWait;  /* seconds waiting out before moving back (inferred) */
    /* The state (inferred from the update): 0 start, 1 hidden, 2 moving out, 3 out, 4 moving
     * back. */
    /* 0x5C */ s32 m_state;
    /* 0x60 */ s32 m_loadedModelIndex; /* the model number the models were loaded with */
    /* 0x64 */ float m_timer;
    /* 0x68 */ Fn8040Vector3 m_position;
    /* 0x74 */ float m_height; /* offset from m_position: 0 out, -s_hiddenDepth hidden */
    /* Bit 0 is set while the footing is in motion and plays the sound. */
    /* 0x78 */ UnkBitFlags_80014E04 m_flags;
    /* 0x7C */ u32 field_0x7C; /* never accessed; Create allocates 0x80 bytes */
};
CHECK_SIZEOF(SetPopFooting, 0x80);

const char *SetPopFooting::s_name = "SetPopFooting";

SetPopFooting::SetPopFooting(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_modelIndex(0), m_secBegin(0.0f), m_secWait(0.0f) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x7A, 4);
    m_charInfo2 = fn_8015CD90(0x7A, 4);
    ReadOwner(owner);

    m_loadedModelIndex = m_modelIndex;
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(m_loadedModelIndex, 0);
    m_model2 = fn_8015F800(1, s_modelFile);
    m_model2->fn_80162A68(m_loadedModelIndex + 1000, 0);

    m_model->fn_80161FC8(m_loadedModelIndex, boost::weak_ptr<CharInfo>(m_charInfo));
    m_model2->fn_80161FC8(m_loadedModelIndex + 1000, boost::weak_ptr<CharInfo>(m_charInfo2));

    UpdateModelMatrices();
}

SetPopFooting::~SetPopFooting() {}

void SetPopFooting::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_model2.get() != 0) {
        m_model2->fn_80162FB0(dt);
    }

    if (m_flags.Test(0)) {
        fn_801D53C4(fn_80053164(), 0x783A, &m_charInfo->m_position, this, 3, 0, 2);
    }

    switch (m_state) {
    case 0:
        m_timer = m_secBegin;
        m_height = -s_hiddenDepth.value;
        m_state = 1;
        break;

    case 1:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = 2;
            m_flags.Set(0);
        }
        break;

    case 2:
        m_height += s_speed.value * dt;
        if (m_height > 0.0f) {
            m_height = 0.0f;
            m_state = 3;
            m_timer = m_secWait;
            m_flags.Reset(0);
        }
        break;

    case 3:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = 4;
            m_flags.Set(0);
        }
        break;

    case 4:
        m_height -= s_speed.value * dt;
        if (m_height < -s_hiddenDepth.value) {
            m_height = -s_hiddenDepth.value;
            m_state = 1;
            m_timer = m_secBegin;
            m_flags.Reset(0);
        }
        break;

    case 5:
        break;
    }

    UpdateCharInfoPosition();
    UpdateModelMatrices();
}

void SetPopFooting::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrices();

    if (m_modelIndex != m_loadedModelIndex) {
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model2->fn_80162804();

        m_loadedModelIndex = m_modelIndex;
        m_model = fn_8015F800(1, s_modelFile);
        m_model->fn_80162A68(m_loadedModelIndex, 0);
        m_model2 = fn_8015F800(1, s_modelFile);
        m_model2->fn_80162A68(m_loadedModelIndex + 1000, 0);

        m_model->fn_80161FC8(m_loadedModelIndex, boost::weak_ptr<CharInfo>(m_charInfo));
        m_model2->fn_80161FC8(m_loadedModelIndex + 1000, boost::weak_ptr<CharInfo>(m_charInfo2));
    }
}

void SetPopFooting::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_modelIndex, 0xC);

    fn_800091D0(&m_charInfo->m_position, &m_position);
    fn_800091D0(&m_charInfo2->m_position, &m_charInfo->m_position);
    m_charInfo2->m_rotation = m_charInfo->m_rotation;

    m_state = 0;
    m_timer = 0.0f;
    m_height = 0.0f;
    m_flags.Clear();
}

void SetPopFooting::UpdateModelMatrices() {
    Fn8040Matrix34 matrix;
    Fn8040Matrix34 matrix2;

    if (m_model.get() != 0) {
        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
    if (m_model2.get() != 0) {
        fn_8015CEA0(m_charInfo2.get(), &matrix2);
        m_model2->fn_801633DC(&matrix2);
    }
}

void SetPopFooting::UpdateCharInfoPosition() {
    Fn8040Vector3 offset(0.0f, 0.0f, m_height);
    Fn8040Matrix34 rotation;
    Fn8040Vector3 rotated;
    Fn8040Vector3 shifted;

    fn_8040B6F8(&rotation);
    fn_8040B848(&rotation, m_charInfo->m_rotation.y);
    fn_8040B884(&rotation, m_charInfo->m_rotation.x);
    fn_8040B80C(&rotation, m_charInfo->m_rotation.z);
    fn_8040B67C(&rotated, &rotation, &offset);
    fn_800091D0(&offset, &rotated);

    fn_8040CDD0(&shifted, &m_position, &offset);
    fn_800091D0(&m_charInfo->m_position, &shifted);
}

} // namespace PopFooting

extern "C" void fn_8022BC84(void) {
    fn_8006B474("POPFOOTING.BIN", 1);
}

extern "C" void fn_8022BC80(void) {}

void PopFooting::SetPopFooting::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetPopFooting(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "sec_begin", 0, 0, 0, 1.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "sec_wait", 0, 0, 0, 1.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80546BA0 = {
    "POP FOOTING",
    fn_8022BC84,
    fn_8022BC80,
    PopFooting::SetPopFooting::Create,
    0,
    0,
    0x13F0,
    1,
    0,
    10,
    s_paramInfo,
};
