/* BDHologram::BDHologram (class name from RTTI): a gadget, in the namespace BDHologram, that shows
 * the BDHOLOGRAM.BIN model at the owner's position. While the point that fn_80010244 returns is
 * farther from the gadget than the parameter "distance" (compared on the ground plane), it
 * requests a sound at the gadget each frame (fn_801D53C4 reuses the sound it finds for this
 * gadget, if any); once the point is within that distance, it drops its target object (m_target),
 * plays the voice that the parameter "voice id" selects and another sound, and goes quiet
 * (behaviour inferred). File name inferred from the class. Method names marked "(name inferred)"
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "layout_check.h"

static const char *s_modelFile = "BDHOLOGRAM.BIN";

/* The gadget parameters in the order of the descriptor's table below. */
struct BDHologramParams {
    BDHologramParams(float distance_, s32 voiceId_) : distance(distance_), voiceId(voiceId_) {}

    /* 0x0 */ float distance; /* how close the point must come to trigger the gadget */
    /* 0x4 */ s32 voiceId;
};
CHECK_SIZEOF(BDHologramParams, 0x8);

/* The default value of the parameter "distance".
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it (and the table entry that shows it) with a start-up function in .ctors (a
 * constant float initialiser would be emitted as initialised .sdata data instead), and the shared
 * stand-in reproduces that function. Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_defaultDistance(500.0f);

namespace BDHologram {

class BDHologram : public Gadget {
  public:
    BDHologram(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BDHologram();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by the character
     * info's rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ s32 m_state; /* 0 until the first update after construction or a reset (inferred) */
    /* 0x4C */ BDHologramParams m_params;
    /* 0x54 */ u32 field_0x54; /* in the allocation size (0x58); no code here touches it */
};
CHECK_SIZEOF(BDHologram, 0x58);

const char *BDHologram::s_name = "BDHologram";

BDHologram::BDHologram(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_params(s_defaultDistance.value, 0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8B, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
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

BDHologram::~BDHologram() {}

void BDHologram::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    /* States: 0 after construction or a reset, 1 while waiting for the point to come within the
     * distance, 2 once triggered (inferred). */
    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 1: {
        float triggerDistanceSquared = m_params.distance * m_params.distance;
        Fn8040Vector3 point;
        Fn8040Vector3 self;
        Fn8040Vector3 difference;

        Fn8040Vector3 *pointPosition = fn_80010244(fn_80009548(), 0);
        fn_800091EC(&point, pointPosition);
        fn_800091EC(&self, &m_charInfo->m_position);
        self.y = 0.0f;
        point.y = 0.0f;
        fn_8040CD74(&difference, &point, &self);

        if (fn_8040CF2C(&difference) < triggerDistanceSquared) {
            if (m_target.get() != 0) {
                m_target.reset();
            }
            m_state = 2;

            int arg2 = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (arg2 != -1) {
                fn_801FAF00(fn_80109D64(), m_params.voiceId, arg2, 0, 0);
            }
            fn_801D54F4(fn_80053164(), 0x7941, &m_charInfo->m_position, 0, 1);
        } else {
            fn_801D53C4(fn_80053164(), 0x7940, &m_charInfo->m_position, this, 3, 0, 2);
        }
        break;
    }
    case 2:
        break;
    }
}

void BDHologram::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
}

void BDHologram::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
}

void BDHologram::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

} // namespace BDHologram

extern "C" void fn_803037AC(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_803037A8(void) {}

void BDHologram::BDHologram::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BDHologram(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "distance", 0, 0, 0, s_defaultDistance.value, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "voice id", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055AE58 = {
    "BD Hologram", fn_803037AC, fn_803037A8, BDHologram::BDHologram::Create, 0, 0, 0x1903, 1, 0, 10,
    s_paramInfo,
};
