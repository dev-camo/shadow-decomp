/* SuctionPillar::SuctionPillar (class name from RTTI; the gadget's descriptor is "SUCTION
 * PILLAR"). File name inferred from the class. Method names marked "(name inferred)" in the header
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/SuctionPillar.h"
#include "Game/CharCommandUnit.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

static const char *s_modelFile = "SUCTIONPILLAR.BIN";

/* The value that the constructor stores in the float at 0x20 of each object, and the height at
 * which the pillar stops rising (names inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original sets them with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_objectField20(40.0f);
static UnkStartupFloat_800E2570 s_riseLimit(80.0f);

namespace SuctionPillar {

const char *SuctionPillar::s_name = "SuctionPillar";

SuctionPillar::SuctionPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x90, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SuctionPillar::OnCommand, this, _1));
    ReadOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, m_charInfo);

    for (int i = 0; i < 2; i++) {
        m_objects[i] = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x80);
        m_objects[i]->SetField20(s_objectField20.value);
    }

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

SuctionPillar::~SuctionPillar() {
    for (int i = 0; i < 2; i++) {
        fn_801E7108(m_objects[i]);
    }
    fn_8034DB60(m_commandObject);
}

void SuctionPillar::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        if (m_flags.Test(0)) {
            m_rise += 25.0f * dt;
            if (m_rise > s_riseLimit.value) {
                m_rise = s_riseLimit.value;
                m_state = 2;
                fn_8034DB60(m_commandObject);
                fn_801D54F4(fn_80053164(), 0x793F, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D53C4(fn_80053164(), 0x793E, &m_charInfo->m_position, this, 3, 0, 2);
            }

            Fn8040Vector3 offset(0.0f, 0.0f, m_rise);
            Fn8040Matrix34 matrix;
            Fn8040Vector3 position;
            Fn8040Vector3 rotated;

            fn_8040B6F8(&matrix);
            fn_8040B848(&matrix, m_charInfo->m_rotation.y);
            fn_8040B884(&matrix, m_charInfo->m_rotation.x);
            fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
            fn_8040B67C(&rotated, &matrix, &offset);
            fn_8040CDD0(&position, &m_basePosition, &rotated);
            fn_800091D0(&m_charInfo->m_position, &position);
            UpdateModelMatrix();
        }
        break;

    /* The pillar has stopped. */
    case 2:
        break;
    }
}

void SuctionPillar::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        /* Draws a debug line between two points in the pillar's frame, the second at the rise
         * limit along z (behaviour inferred). */
        Fn8040Vector3 start(0.0f, 60.0f, 40.0f);
        Fn8040Vector3 end(0.0f, 60.0f, s_riseLimit.value);
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

        Fn8040Vector3 placedStart;

        fn_8040B67C(&placedStart, &matrix, &start);
        fn_800091D0(&start, &placedStart);

        Fn8040Vector3 placedEnd;

        fn_8040B67C(&placedEnd, &matrix, &end);
        fn_800091D0(&end, &placedEnd);

        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
        m_model->fn_8016108C();
    }
}

void SuctionPillar::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_switchId);
    m_state = 0;
    fn_800091D0(&m_basePosition, &m_charInfo->m_position);
    m_flags.Clear();
    m_rise = 0.0f;
}

void SuctionPillar::UpdateModelMatrix() {
    /* The original also compares the address of the m_objects array with null (always true). */
    if (m_model.get() != 0 && m_objects != 0) {
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);

        Fn8040Vector3 offset(0.0f, 40.0f, 40.0f);
        Fn8040Vector3 placed;

        fn_8040B67C(&placed, &matrix, &offset);
        fn_800091D0(&offset, &placed);
        fn_800CCE38(m_objects[0].get(), &offset);

        Fn8040Vector3 offset2(0.0f, 40.0f, -40.0f);
        Fn8040Vector3 placed2;

        fn_8040B67C(&placed2, &matrix, &offset2);
        fn_800091D0(&offset2, &placed2);
        fn_800CCE38(m_objects[1].get(), &offset2);
    }
}

void SuctionPillar::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14:
        if (!m_flags.Test(0)) {
            m_flags.Set(0);
            if (m_switchId > 0) {
                fn_801690D0(fn_800CB69C(), m_switchId, 1);
            }
            m_commandObject = fn_80169B1C(8, &m_charInfo->m_position);
        }
        break;
    }
}

} // namespace SuctionPillar

extern "C" void fn_80304120(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_8030411C(void) {}

void SuctionPillar::SuctionPillar::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SuctionPillar(parent, owner);
}

GadgetTypeInfo lbl_8055AF34 = {
    "SUCTION PILLAR",
    fn_80304120,
    fn_8030411C,
    SuctionPillar::SuctionPillar::Create,
    0,
    0,
    0x1902,
    1,
    0,
    10,
    0,
};
