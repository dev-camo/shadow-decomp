/* TestNakazawa (class name from RTTI): a test task (by its class name), derived from Task directly,
 * that registers with an owner and probes the world below its character info. File name inferred
 * from the class. Method names marked "(name inferred)" are not original. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "layout_check.h"

class TestNakazawa : public Task {
  public:
    TestNakazawa(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~TestNakazawa();
    virtual void Update(float dt);

    /* Creates the task under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x28 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x2C */ CharInfo *m_charInfo;
    /* 0x30 */ u32 field_0x30;
    /* 0x34 */ u32 field_0x34;
    /* 0x38 */ u32 field_0x38;
    /* 0x3C */ u32 field_0x3C;
};
CHECK_SIZEOF(TestNakazawa, 0x40);

const char *TestNakazawa::s_name = "TestNakazawa";

TestNakazawa::TestNakazawa(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Task(parent) {
    m_name = s_name;
    m_owner = owner;
    fn_800C9CD0(m_owner, this);
    m_charInfo = new CharInfo(5);
    m_charInfo->field_0x5C = 4;
}

TestNakazawa::~TestNakazawa() {
    if (m_charInfo != 0) {
        delete m_charInfo;
        m_charInfo = 0;
    }
    fn_800C9BE8(m_owner);
}

void TestNakazawa::Update(float dt) {
    /* Copies the owner's position and rotation when both queries are true (fn_800C9D0C always
     * returns false in this build, so this never runs). */
    if (fn_800C9D0C(m_owner) && fn_800C9D14(m_owner)) {
        fn_800C9F14(m_owner, &m_charInfo->m_position);
        fn_800C9ED4(m_owner, &m_charInfo->m_rotation);
        fn_800C9CF8(m_owner);
    }

    /* Flag 0x01 marks the task finished (inferred), as Gadget::IsFinished does for these two owner
     * queries. */
    if (fn_800C9A64(m_owner)) {
        m_flags |= 1;
    } else if (fn_800C996C(m_owner)) {
        m_flags |= 1;
    } else {
        /* Probes 1000 units straight down from the character info for objects whose weak pointer
         * is still alive. The hit point copied out of the first such object is not used. */
        Fn8040Vector3 start;
        Fn8040Vector3 end;
        Fn8040Vector3 lineEnd;
        Fn8040Vector3 hitPoint;

        fn_800091EC(&start, &m_charInfo->m_position);
        fn_800091EC(&end, &start);
        end.y -= 1000.0f;
        fn_800091EC(&lineEnd, &start);

        UnkSegment_80020A20 probe(start, end);
        UnkProbeList_80021FE4 *hits = fn_80021FE4(fn_80020D40(), &probe, 1, 0, 0, 0);
        if (hits != 0) {
            UnkProbeNode_80021FE4 *iterator = hits->fn_8001EE04();

            while (iterator != hits->fn_8001EDEC()) {
                UnkProbeNode_80021FE4 *node = iterator;

                if (node != 0) {
                    boost::shared_ptr<UnkProbeTarget_800587A0> target =
                        fn_800587A0(&node->target).lock();

                    if (target.get() != 0) {
                        fn_800091EC(&hitPoint, fn_8001E9BC(node->hit));
                        break;
                    }
                }
                fn_80027398(&iterator);
            }
        }
        delete hits;

        /* The debug line runs from the position to a copy of it, so it has no length. */
        fn_802E0B58(UnkSegment_80020A20(start, lineEnd), &lbl_805F06D8);
    }
}

extern "C" void fn_802DF708(void) {}

extern "C" void fn_802DF704(void) {}

void TestNakazawa::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new TestNakazawa(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80559210 = {
    "TESTNAKAZAWA", fn_802DF708, fn_802DF704, TestNakazawa::Create, 0, 0, 0x2587, 1, 0, 10, 0,
};
