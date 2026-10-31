/* TornadoCollision (class name from RTTI): a task, derived from Task directly, that keeps a probe
 * sphere (gadget parameter "Radius") at the owner's position and offers every character it
 * touches to the tornado character infos. File name inferred from the class. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). NonMatching: ProcessHits is one branch short. */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/TornadoCharInfo.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkCharInfo_800999C0.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "layout_check.h"

class TornadoCollision : public Task {
  public:
    TornadoCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~TornadoCollision();
    virtual void Update(float dt);

    /* Creates the task under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Offers charInfo to each tornado character info (word at 0x50, the constructor argument,
     * equal to 0x2A) until one accepts it for this task's tag byte (name inferred). */
    bool AddToTornado(boost::shared_ptr<CharInfo> charInfo);
    /* Offers the characters that the probe touches to the tornado (name inferred). The float is
     * the time step that Update passes; the function does not use it. */
    void ProcessHits(float dt);

    static const char *s_name;

    /* 0x28 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x2C */ boost::shared_ptr<UnkProbeTarget_800587A0> m_target;
    /* 0x34 */ float m_radius; /* the gadget parameter "Radius" */
    /* 0x38 */ Fn8040Vector3 m_position;
    /* 0x44 */ u8 m_tag; /* copied from the owner; compared with the tornado's tag (inferred) */
};
CHECK_SIZEOF(TornadoCollision, 0x48);

const char *TornadoCollision::s_name = "TornadoCollision";

TornadoCollision::TornadoCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Task(parent) {
    m_name = s_name;
    if (owner != 0) {
        m_owner = owner;
        fn_800C9CD0(m_owner, this);
        fn_800C9F14(m_owner, &m_position);
        fn_800C9DBC(m_owner, &m_radius, 4);
        fn_800C9E60(m_owner, &m_tag);
        m_target = fn_800C7EA8(fn_80021B08(), &m_position, 0x40100, 10, 0, lbl_805EE698,
                               &lbl_805E07A0, m_radius);
    }
}

TornadoCollision::~TornadoCollision() {
    if (m_owner != 0) {
        fn_800C9BE8(m_owner);
        m_owner = 0;
    }
}

extern "C" void fn_800DF91C(void) {}

extern "C" void fn_800DF918(void) {}

void TornadoCollision::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new TornadoCollision(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void TornadoCollision::Update(float dt) {
    if (m_owner != 0) {
        /* Refreshes the settings from the owner when both queries are true (as in
         * TestNakazawa::Update; fn_800C9D0C always returns false in this build). */
        if (fn_800C9D0C(m_owner) && fn_800C9D14(m_owner)) {
            fn_800C9F14(m_owner, &m_position);
            fn_800C9DBC(m_owner, &m_radius, 4);
            fn_800C9E60(m_owner, &m_tag);
            if (m_target.get() != 0) {
                m_target->SetPosition(&m_position);
                m_target->SetRadius(m_radius, -2, 0.0f, 0.0f);
            }
            fn_800C9CF8(m_owner);
        }

        /* Flag 0x01 marks the task finished (inferred), as Gadget::IsFinished does for these two
         * owner queries. */
        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
            return;
        }
        if (fn_800C996C(m_owner)) {
            m_flags |= 1;
            return;
        }
    }
    ProcessHits(dt);
}

void TornadoCollision::ProcessHits(float) {
    if (m_target.get() != 0) {
        UnkProbeList_80021FE4 *hits = m_target->GetHits();

        if (hits != 0) {
            for (UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
                 iterator.node != hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
                boost::shared_ptr<CharInfo> charInfo =
                    fn_800C4568(fn_80021B08(), fn_800587A0(&iterator.node->target)).lock();

                if (charInfo.get() != 0) {
                    /* Only a character info whose word at 0x5C is 1 or 2 and whose word at 0x50
                     * (the constructor argument) is 1 is offered, and not while bit 0x14 of the
                     * bit set at 0xA8 is set. */
                    if (charInfo->field_0x5C == 1 || charInfo->field_0x5C == 2) {
                        if (charInfo->field_0x50 == 1) {
                            /* Where this scope begins, the original has a second branch on the
                             * same comparison of the word at 0x50, which skips the cast check and
                             * goes to AddToTornado; that branch is this unit's only difference.
                             * The cast pointer is released before AddToTornado. */
                            {
                                boost::shared_ptr<UnkCharInfo_800999C0> derived =
                                    boost::static_pointer_cast<UnkCharInfo_800999C0>(charInfo);

                                if (fn_8006CD78(&derived->field_0xA8, 0x14)) {
                                    continue;
                                }
                            }
                            AddToTornado(charInfo);
                        }
                    }
                }
            }
        }
    }
}

bool TornadoCollision::AddToTornado(boost::shared_ptr<CharInfo> charInfo) {
    UnkCharInfoNode_801DCB14 *node = fn_801DCB14(fn_800DDF20(), 0x2A);

    while (node != 0) {
        boost::shared_ptr<TornadoCharInfo> tornadoInfo =
            boost::static_pointer_cast<TornadoCharInfo>(node->charInfo.lock());

        if (tornadoInfo.get() != 0) {
            if (tornadoInfo->AttachTarget(m_tag, charInfo)) {
                return true;
            }
        }
        node = node->next;
    }
    return false;
}

static GadgetParamInfo lbl_80523F00[] = {
    {4, "Radius", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80523F50 = {
    "TORNADO_COLLI", fn_800DF91C, fn_800DF918, TornadoCollision::Create, 0, 0, 0x835, 1, 0, 0x14,
    lbl_80523F00,
};
