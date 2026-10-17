/* ElecCristalWallSwitch (class name from RTTI; "Cristal" is the original spelling): a switch of the
 * electric circuit puzzle that sets its circuit's state to 1 once the nearest character comes
 * within its range. File name inferred from the class. Method names marked "(name inferred)" are
 * not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class ElecCristalWallSwitch : public Gadget {
  public:
    ElecCristalWallSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCristalWallSwitch();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C();
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the switch under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, circuit id and range into the switch (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Queues a debug sphere showing the range (name inferred). */
    void DrawDebugRange();
    /* True when the nearest character info is within the range (name inferred). */
    bool IsCharInfoInRange();

    static const char *s_name;

    /* 0x30 */ float m_range; /* the "range" parameter */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ u8 field_0x3C; /* copied from the owner; the circuit id it sets (inferred) */
};
CHECK_SIZEOF(ElecCristalWallSwitch, 0x40);

const char *ElecCristalWallSwitch::s_name = "ElecCristalWallSwitch";

ElecCristalWallSwitch::ElecCristalWallSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x59, 4, "ElecCristalWallSwitch");
    CopyFromOwner(m_owner);
}

ElecCristalWallSwitch::~ElecCristalWallSwitch() {}

void ElecCristalWallSwitch::UpdateBody(float dt) {
    if (IsCharInfoInRange()) {
        if (fn_8016904C(fn_800CB69C(), field_0x3C) != 1) {
            fn_801690D0(fn_800CB69C(), field_0x3C, 1);
        }
    }
}

void ElecCristalWallSwitch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecCristalWallSwitch::fn_800CBC1C() {
    DrawDebugRange();
}

void ElecCristalWallSwitch::DrawDebugRange() {
    fn_802E0E84(&m_charInfo->m_position, &lbl_805F06D8, m_range);
}

void ElecCristalWallSwitch::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9E60(owner, &field_0x3C);
    fn_800C9DBC(owner, &m_range, sizeof(m_range));
}

bool ElecCristalWallSwitch::IsCharInfoInRange() {
    boost::shared_ptr<CharInfo> nearest =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (nearest.get() != 0) {
        float rangeSquared = m_range * m_range;
        Fn8040Vector3 offset;

        fn_8040CD74(&offset, &m_charInfo->m_position, &nearest->m_position);
        return fn_8040CF2C(&offset) <= rangeSquared;
    }

    return false;
}

extern "C" void fn_801F9FF4(void) {}

extern "C" void fn_801F9FF0(void) {}

void ElecCristalWallSwitch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCristalWallSwitch(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo lbl_80542528[] = {
    {4, "range", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80542578 = {
    "ELEC_WALLSWITCH",
    fn_801F9FF4,
    fn_801F9FF0,
    ElecCristalWallSwitch::Create,
    0,
    0,
    0x7E1,
    1,
    0,
    10,
    lbl_80542528,
};
