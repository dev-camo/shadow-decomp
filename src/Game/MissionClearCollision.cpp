/* MissionClearCollision (class name from RTTI): a gadget that, once the character info that
 * fn_800A8C54 returns for 0 comes within the parameter "COLLI_RADIUS point" of it, calls
 * fn_8016B2A4 on the mission that fn_800740E4 returns for the parameter "0:DK 1:NM 2:HR" and
 * finishes, provided there is such a mission and its word at 0xC is neither 1 nor 2. Except for
 * type 1 it creates an UnkObject_802B4254 for the type, placed above the gadget (behaviour
 * inferred). File name inferred from the class. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first). */
#include "Game/MissionClearCollision.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8040_vector_types.h"

const char *MissionClearCollision::s_name = "MissionClearCollision";

/* The debug sphere colours of the types 0 (DK), 1 (NM) and 2 (HR). */
static UnkDebugColor_802E0E84 s_colorDK(0x80, 0xE0, 0x10, 0x00);
static UnkDebugColor_802E0E84 s_colorNM(0x80, 0xFF, 0xFF, 0x10);
static UnkDebugColor_802E0E84 s_colorHR(0x80, 0x35, 0xAD, 0xFF);
static UnkDebugColor_802E0E84 *s_sphereColors[] = {&s_colorDK, &s_colorNM, &s_colorHR};

/* How far above the gadget the object is placed.
 * Matching workaround: the original sets this float with the unit's start-up function in .ctors;
 * a constant initialiser gives no store. UnkStartupFloat_800E2570 stands in for the original
 * initialiser expression, which is unknown, and gives the start-up function. The const qualifier
 * (inferred) is what lets the start-up function reload the saved link register (0x80231AF4)
 * between the load of the value and its store, as the original does; with a non-const float the
 * store comes before that reload. Revisit when the original initialiser is found. */
static const float s_objectHeight = UnkStartupFloat_800E2570(40.0f).value;

MissionClearCollision::MissionClearCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x85, 4);
    CopyFromOwner(owner);

    if (m_params.type != 1) {
        m_object = fn_802B4254(m_params.type, 0);
        fn_802B4510(m_object.get(), 1);

        Fn8040Vector3 offset(0.0f, s_objectHeight, 0.0f);

        const Fn8040Vector3 &position = m_charInfo->m_position + offset;
        fn_802B44C8(m_object.get(), &position);
    }
}

MissionClearCollision::~MissionClearCollision() {}

extern "C" void fn_8023192C(void) {}

extern "C" void fn_80231928(void) {}

void MissionClearCollision::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new MissionClearCollision(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "0:DK 1:NM 2:HR", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "COLLI_RADIUS point", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805470EC = {
    "MissionClearCollision",
    fn_8023192C,
    fn_80231928,
    MissionClearCollision::Create,
    0,
    0,
    0x2595,
    1,
    0,
    10,
    s_paramInfo,
};

void MissionClearCollision::UpdateBody(float dt) {
    boost::shared_ptr<CharInfo> character = fn_800A8C54(fn_80074194(), 0).lock();

    if (character.get() != 0) {
        Fn8040Vector3 distance = character->m_position - m_charInfo->m_position;

        if (fn_8040CF50(distance) < m_params.radius) {
            boost::shared_ptr<Mission::Mission> mission =
                fn_800740E4(fn_8006D6A8(), m_params.type).lock();

            if (mission.get() != 0 && !mission->Field0xCEquals(2) && !mission->Field0xCEquals(1)) {
                fn_8016B2A4(mission.get());
                fn_800C9B34(m_owner);
                m_flags |= 1;
                /* The original returns here: falling through instead shares the destructor calls
                 * with the other path (0x150 bytes instead of 0x16C). */
                return;
            }
        }
    }
}

void MissionClearCollision::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    DrawDebugRange();
}

void MissionClearCollision::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void MissionClearCollision::DrawDebugRange() {
    fn_802E0E84(&m_charInfo->m_position, *s_sphereColors[m_params.type], m_params.radius);
}
