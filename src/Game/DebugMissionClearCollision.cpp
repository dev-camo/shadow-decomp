/* DebugMissionClearCollision (class name from RTTI): the debug counterpart of
 * MissionClearCollision. Once the character info that fn_800A8C54 returns for 0 comes within 10
 * units of it, it finishes: for the parameter "0:DK 1:NM 2:HR 3:BOSS" set to 3 it passes 6 to
 * fn_80175EEC; otherwise, when fn_800740E4 returns a mission for the type whose word at 0xC is not
 * 1, it stores 1 in the mission's byte at 0x14 and calls fn_8016B2A4 on it. Until then it turns
 * about its y axis by a quarter turn per unit of dt and queues its debug shapes (behaviour
 * inferred). File name inferred from the class. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first). */
#include "Game/DebugMissionClearCollision.h"
#include "Game/AngleConversion.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MissionClearCollision.h"
/* The unit's .data, .sdata and .rodata start with the TNumEdit RTTI data that TNumEdit.h
 * reproduces. */
#include "Game/TNumEdit.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8040_vector_types.h"

const char *DebugMissionClearCollision::s_name = "DebugMissionClearCollision";

/* The trigger radius, the half size of the M and the float passed with its lines.
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original stores them with the unit's start-up function in .ctors (a constant
 * float initialiser would be emitted as initialised .sdata data instead). Revisit when the original
 * wrapper type is found. */
static UnkStartupFloat_800E2570 s_radius(10.0f);
static UnkStartupFloat_800E2570 s_markHalfSize(5.0f);
static UnkStartupFloat_800E2570 s_lineArg(1.0f);

/* The four lines of an M in the gadget's x-y plane. */
static UnkSegment_80020A20
    s_markLine0(Fn8040Vector3(-s_markHalfSize.value, -s_markHalfSize.value, 0.0f),
                Fn8040Vector3(-s_markHalfSize.value, s_markHalfSize.value, 0.0f));
static UnkSegment_80020A20 s_markLine1(Fn8040Vector3(-s_markHalfSize.value, s_markHalfSize.value,
                                                     0.0f),
                                       Fn8040Vector3(0.0f, 0.0f, 0.0f));
static UnkSegment_80020A20 s_markLine2(Fn8040Vector3(0.0f, 0.0f, 0.0f),
                                       Fn8040Vector3(s_markHalfSize.value, s_markHalfSize.value,
                                                     0.0f));
static UnkSegment_80020A20
    s_markLine3(Fn8040Vector3(s_markHalfSize.value, s_markHalfSize.value, 0.0f),
                Fn8040Vector3(s_markHalfSize.value, -s_markHalfSize.value, 0.0f));

/* The debug colours of the types 0 (DK), 1 (NM), 2 (HR) and 3 (BOSS), for the lines and for the
 * sphere. */
static UnkDebugColor_802E0E84 s_lineColorDK(0xC0, 0xE0, 0x10, 0x00);
static UnkDebugColor_802E0E84 s_sphereColorDK(0x80, 0xE0, 0x10, 0x00);
static UnkDebugColor_802E0E84 s_lineColorNM(0xC0, 0xFF, 0xFF, 0x10);
static UnkDebugColor_802E0E84 s_sphereColorNM(0x80, 0xFF, 0xFF, 0x10);
static UnkDebugColor_802E0E84 s_lineColorHR(0xC0, 0x35, 0xAD, 0xFF);
static UnkDebugColor_802E0E84 s_sphereColorHR(0x80, 0x35, 0xAD, 0xFF);
static UnkDebugColor_802E0E84 s_lineColorBOSS(0xC0, 0xFF, 0xFF, 0xFF);
static UnkDebugColor_802E0E84 s_sphereColorBOSS(0x80, 0xFF, 0xFF, 0xFF);

static UnkSegment_80020A20 *s_markLines[] = {&s_markLine0, &s_markLine1, &s_markLine2,
                                             &s_markLine3};
static UnkDebugColor_802E0E84 *s_sphereColors[] = {&s_sphereColorDK, &s_sphereColorNM,
                                                   &s_sphereColorHR, &s_sphereColorBOSS};
static UnkDebugColor_802E0E84 *s_lineColors[] = {&s_lineColorDK, &s_lineColorNM, &s_lineColorHR,
                                                 &s_lineColorBOSS};

/* Radians per unit of dt. */
static float s_turnSpeed = DegreesToRadians(90.0f);

DebugMissionClearCollision::DebugMissionClearCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x84, 4);
    CopyFromOwner(owner);
}

DebugMissionClearCollision::~DebugMissionClearCollision() {}

extern "C" void fn_802C8820(void) {}

extern "C" void fn_802C881C(void) {}

void DebugMissionClearCollision::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DebugMissionClearCollision(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "0:DK 1:NM 2:HR 3:BOSS", 0, 0, 3, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80557C54 = {
    "DebugDebugMissionClearCollision",
    fn_802C8820,
    fn_802C881C,
    DebugMissionClearCollision::Create,
    0,
    0,
    0x2592,
    1,
    0,
    10,
    s_paramInfo,
};

void DebugMissionClearCollision::UpdateBody(float dt) {
    boost::shared_ptr<CharInfo> character = fn_800A8C54(fn_80074194(), 0).lock();

    if (character.get() != 0) {
        Fn8040Vector3 distance = character->m_position - m_charInfo->m_position;

        if (fn_8040CF50(&distance) < s_radius.value) {
            u32 type = m_params.type;

            if (type == 3) {
                fn_80175EEC(fn_80074470(), 6, false);
                fn_800C9B34(m_owner);
                m_flags |= 1;
                return;
            }

            boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), type).lock();

            if (mission.get() != 0 && !mission->Field0xCEquals(1)) {
                mission->SetField0x14(1);
                fn_8016B2A4(mission.get());
                fn_800C9B34(m_owner);
                m_flags |= 1;
                return;
            }
        }
    }

    m_charInfo->m_rotation.y += s_turnSpeed * dt;
    DrawDebugMarker();
}

void DebugMissionClearCollision::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    DrawDebugMarker();
}

void DebugMissionClearCollision::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void DebugMissionClearCollision::DrawDebugMarker() {
    fn_802E0E84(&m_charInfo->m_position, *s_sphereColors[m_params.type], s_radius.value);

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);

    for (u32 i = 0; i < 4; i++) {
        UnkSegment_80020A20 line = *s_markLines[i];

        line.start = matrix * line.start;
        line.end = matrix * line.end;
        fn_802E0AA0(line, *s_lineColors[m_params.type], s_lineArg.value);
    }
}
