/* GravityChangeCollision (class name from RTTI): a gadget with a box-shaped probe (parameters
 * SIZE X m, SIZE Y m and SIZE Z m, and GRAVITY) that sets the stage's gravity direction to the
 * index in GRAVITY when a character touches the box (behaviour inferred), and then finishes. File
 * name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGravity_80293928.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table below. CopyFromOwner scales the
 * three sizes by 10 and halves them after reading them, so from then on they hold the half
 * extents of the box. */
struct GravityChangeCollisionParams {
    GravityChangeCollisionParams() : sizeX(10.0f), sizeY(10.0f), sizeZ(10.0f), gravity(0) {}

    /* 0x0 */ float sizeX; /* SIZE X m */
    /* 0x4 */ float sizeY; /* SIZE Y m */
    /* 0x8 */ float sizeZ; /* SIZE Z m */
    /* 0xC */ s32 gravity; /* GRAVITY: the index of the gravity direction to set */
};
CHECK_SIZEOF(GravityChangeCollisionParams, 0x10);

class GravityChangeCollision : public Gadget {
  public:
    GravityChangeCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GravityChangeCollision();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ GravityChangeCollisionParams m_params;
    /* 0x40 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x48 */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x50 */ UnkBitFlags_80014E04 m_hitFlags; /* bit 0: a character touched the box (inferred) */
    /* 0x54 */ u32 field_0x54;
};
CHECK_SIZEOF(GravityChangeCollision, 0x58);

const char *GravityChangeCollision::s_name = "GravityChangeCollision";

GravityChangeCollision::GravityChangeCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_hitFlags(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x68, 4);
    CopyFromOwner(owner);

    Fn8040Vector3 halfExtents(m_params.sizeX, m_params.sizeY, m_params.sizeZ);

    m_probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), &halfExtents,
                          0x140100, 7, 0, &lbl_805E07A0, &lbl_805E07B8, 0);
}

GravityChangeCollision::~GravityChangeCollision() {}

extern "C" void fn_80247914(void) {
    fn_80293B1C();
}

extern "C" void fn_802478F4(void) {
    fn_80293AE4();
}

void GravityChangeCollision::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GravityChangeCollision(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void GravityChangeCollision::UpdateBody(float dt) {
    if (fn_8015D2EC(m_probe->GetHits())) {
        m_hitFlags.Set(0);
    }
    if (m_hitFlags.Test(0)) {
        if (m_params.gravity != fn_80293AC8()) {
            fn_80293928(m_params.gravity);
        }
        m_hitFlags.Reset(0);
        fn_800C9AD4(m_owner);
        m_flags |= 1;
    }
}

void GravityChangeCollision::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    if (m_probe.get() != 0) {
        m_probe->SetSize(m_params.sizeX, -2, m_params.sizeY, m_params.sizeZ);
        m_probe->UnkSlot26();
    }

    /* Draws a debug line from the character info's position to the point 50 times the gravity
     * table's vector for the GRAVITY index away in the opposite direction. */
    Fn8040Vector3 direction;
    Fn8040Vector3 end;

    fn_800091EC(&direction, fn_80293B60(m_params.gravity));
    fn_8040CE38(&direction, -50.0f);
    fn_8040CDD0(&end, &m_charInfo->m_position, &direction);

    UnkSegment_80020A20 segment(m_charInfo->m_position, end);

    fn_802E0AA0(segment, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 1.0f);
}

void GravityChangeCollision::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.sizeX = 0.5f * (10.0f * m_params.sizeX);
    m_params.sizeY = 0.5f * (10.0f * m_params.sizeY);
    m_params.sizeZ = 0.5f * (10.0f * m_params.sizeZ);
}

static GadgetParamInfo lbl_80548968[] = {
    {4, "SIZE X m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SIZE Y m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SIZE Z m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "GRAVITY", 0, 0, 5, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80548A30 = {
    "GravityChangeCollision",
    fn_80247914,
    fn_802478F4,
    GravityChangeCollision::Create,
    0,
    0,
    0x14B6,
    1,
    0,
    10,
    lbl_80548968,
};
