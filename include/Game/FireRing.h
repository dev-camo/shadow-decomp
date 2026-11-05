#ifndef GAME_FIRERING_H
#define GAME_FIRERING_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace FireRing {

/* FireRing::FireRing (class name from RTTI; the gadget's descriptor is "FIRE_RING"): a ring that
 * shows the FIRERING.BIN model and has two probes, offset by (0, 0, 14) and (0, 0, -14). A
 * character that touches one probe and then the other has passed through the ring: the ring
 * passes it to fn_8015F6EC with 0, 5 and 0, calls fn_80136B5C(1) and finishes once m_timer, set
 * to 0.5, has counted down. Some attacks finish it too (see OnAttack). Behaviour inferred; method
 * names marked "(name inferred)" are not original. */
class FireRing : public Gadget {
  public:
    FireRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FireRing();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and clears the state, the
     * timer and the pass flags (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned by its rotation (name inferred). */
    void UpdateModelMatrix();
    /* The attack callback of the model's "ring" attack object (name inferred). While the ring
     * waits, an attack whose attack object lacks bit 4 of the word at 0x40 gets the result 5;
     * any other attack gets 3 and finishes the ring. Outside that state the result is 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The collision callbacks of m_handleA and m_handleB (names inferred): with a character in the
     * hits, the first touch sets the pass flag of that side and remembers the character; a touch
     * after one on the other side sets the state "passed". */
    void OnHitA(ColliDetectArgs &args);
    void OnHitB(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The handle targets of the probes offset by (0, 0, 14) and (0, 0, -14). */
    /* 0x40 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handleA;
    /* 0x48 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handleB;
    /* 0x50 */ boost::weak_ptr<CharInfo> m_character; /* the character that touched a probe first */
    /* 0x58 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* The state (names inferred from the code that sets them): 0 after construction or a reset,
     * 1 waiting, 2 passed through, 3 finishing when m_timer runs out. */
    /* 0x60 */ s32 m_state;
    /* 0x64 */ float m_timer;
    /* Bit 0: the character touched m_handleA's probe first, bit 1: m_handleB's, bit 2: a
     * character is passing (inferred). */
    /* 0x68 */ UnkBitFlags_80014E04 m_passFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(FireRing, 0x70);

} // namespace FireRing

#endif
