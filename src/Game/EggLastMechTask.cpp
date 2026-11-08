/* EggLastMech::EggLastMechTask (class name from RTTI; the gadget's descriptor is "EGG_LASTMECH"):
 * the task that owns the EggLastMech boss (Game/EggLastMech.cpp). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/EggLastMechTask.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkResource_80012700.h"

/* Defined before the hooks, s_dataFile and s_name: the unit's .rodata starts with "EGG_LASTMECH",
 * followed by the data file name and s_name's string, the order of their definitions. Defined
 * last, as in other gadget files, "EGG_LASTMECH" follows s_name's string; .data is the same
 * either way. */
GadgetTypeInfo lbl_8055A7F8 = {
    "EGG_LASTMECH",
    fn_802FDF30,
    fn_802FDF04,
    EggLastMech::EggLastMechTask::Create,
    0,
    0x800,
    0xBD,
    2,
    0,
    10,
    0,
};

/* An array, not a pointer: the load hook addresses the string itself. A const char * variable
 * adds a 4-byte .sdata word that the hook loads (one instruction shorter than the original's
 * hook), and a string literal in the hook is pooled after the type name of EggLastMechTask's RTTI
 * instead of between the descriptor's name and s_name's string. */
static const char s_dataFile[] = "enemy/boss/EggLastMechData.one";

namespace EggLastMech {

const char *EggLastMechTask::s_name = "EggLastMech";

/* Without an owner the task has no boss. Otherwise it registers itself with the owner and creates
 * the boss, whose Init runs when the allocation succeeded. */
EggLastMechTask::EggLastMechTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Task(parent), m_owner(0) {
    m_name = s_name;
    if (owner != 0) {
        m_owner = owner;
        fn_800C9CD0(m_owner, this);
        m_mech = EggLastMech::Create(this);
        if (m_mech) {
            m_mech->Init(m_owner);
        }
    }
}

EggLastMechTask::~EggLastMechTask() {
    if (m_owner != 0) {
        fn_800C9B34(m_owner);
        m_owner = 0;
    }
}

/* Ends the boss (Finish) when there is an owner and fn_800C9A64 reports it, and updates the boss
 * otherwise. Nothing happens without a boss. */
void EggLastMechTask::Update(float dt) {
    if (m_mech) {
        if (m_owner != 0 && fn_800C9A64(m_owner)) {
            Finish();
        } else {
            m_mech->Update(dt);
        }
    }
}

/* Releases the boss and sets Task flag 0x01. */
void EggLastMechTask::Finish() {
    if (m_mech) {
        m_mech.reset();
    }
    m_flags |= 1;
}

} // namespace EggLastMech

/* The gadget's load hook: registers the boss's data file under kResourceKey and calls
 * fn_8032D9B8. */
extern "C" void fn_802FDF30(void) {
    fn_8001299C(fn_800122D4(), EggLastMech::kResourceKey, new ResourceOneFile(s_dataFile, false));
    fn_8032D9B8();
}

/* The gadget's unload hook. */
extern "C" void fn_802FDF04(void) {
    fn_8032D970();
    fn_800127F8(fn_800122D4(), EggLastMech::kResourceKey);
}

namespace EggLastMech {

/* Allocates the task under task layer 14 and constructs it when the allocation succeeds. */
void EggLastMechTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new EggLastMechTask(fn_801EB304(fn_8001C4F8(), 14), owner);
}

} // namespace EggLastMech
