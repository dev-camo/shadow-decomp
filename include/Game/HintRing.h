#ifndef GAME_HINTRING_H
#define GAME_HINTRING_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

struct RpClump;

namespace HintRing {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings;
 * "Brunch" is the original spelling). fn_800C9DBC copies them from the owner only when the owner's
 * parameter size is 0x14 (compare at 0x800C9DD4, USA (GUPE8P)); otherwise they keep their values,
 * which start as the defaults this constructor stores. Those equal the first value of each table
 * entry (word 0x8 of the integer entries, float 0x14 of the float ones; inferred to be the
 * defaults). */
struct SetHintRingParams {
    SetHintRingParams() : messageId(0), branchId(-1), range(10.0f), sec(0.0f), navi(7) {}

    /* 0x00 */ s32 messageId; /* "Message Id" */
    /* 0x04 */ s32 branchId;  /* "Brunch": a negative value takes the branch from fn_8006D6A8 */
    /* 0x08 */ float range;   /* "range": 10 times it goes to the Target's float at 0x20 */
    /* 0x0C */ float sec;     /* "sec": not read in this file */
    /* 0x10 */ s32 navi;      /* "navi": selects the Target kind, 0 to 7 (labels in the .cpp) */
};
CHECK_SIZEOF(SetHintRingParams, 0x14);

/* HintRing::SetHintRing (class name from RTTI; the gadget's descriptor is "HINTRING"): a ring of
 * the HINTRING.BIN model whose collision node "col" requests a message. Through the callback it
 * hands to a draw layer it also renders the clump of HINTRING01.DFF at its position, turned by the
 * angles that fn_80109B80 returns, while bit 2 of its flags is clear and its sphere of radius 10 is
 * in view (behaviour inferred). The constructor adds a reference to the clump under the name
 * "HintRing" in the registry of fn_8010A458; the destructor releases one reference to that name.
 *
 * Bits of m_flags, by their set and clear points (meanings inferred). ReadOwner clears all bits,
 * then sets bit 0. Bit 0: in state 1 UpdateBody clears it when the com id is positive and
 * fn_8016904C returns 1 for it, and sets it otherwise; in state 2 it sets it when fn_801FAAC8
 * returns true. OnCollision does nothing while bit 0 is clear. Bit 1: CreateTarget sets it, then
 * clears it for any "navi" value outside 0 to 6; it selects OnCollision's hit test. Bit 2: in state
 * 1 UpdateBody sets it when bits 0 and 2 are both clear, and clears it when both are set; only when
 * the model pointer is non-null does it first call the model's fn_80162790 (before setting) or
 * fn_8016271C (before clearing). OnDraw draws only while it is clear. Bit 3: OnCollision sets it on
 * a hit; at the start of each UpdateBody bit 4 takes its value and bit 3 is cleared. Bit 5: set
 * when OnCollision moves to state 2, cleared when UpdateBody leaves state 2. Member names, the
 * names of the non-virtual methods and the parameters' pointer and reference spellings are
 * inferred. */
class SetHintRing : public Gadget {
  public:
    SetHintRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetHintRing();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget, and its parameters into
     * m_params only when the owner's parameter size is 0x14 (fn_800C9DBC); then sets the state and
     * the message id to 0 and the flags to bit 0 alone (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null, places the model at the character info's position,
     * turned by its y, x and z rotations; then, when the Target pointer is non-null, moves the
     * Target to that position (name inferred). */
    void UpdateModelMatrix();
    /* Releases the Target and sets bit 1 of the flags; then, by the "navi" value, creates a Target
     * with the argument 0x200000 (values 0 and 4), 0x400000 (1 and 5) or 0x600000 (3 and 6). The
     * value 2 creates none and keeps bit 1; any other value creates none and clears bit 1. When a
     * Target exists afterwards, its float at 0x20 is set to 10 times "range" (name inferred). */
    void CreateTarget();
    /* The model's collision callback (name inferred). Only while bit 0 of the flags is set: a hit
     * is tested with fn_8015B1C4 when bit 1 is set and with fn_8015AC84 otherwise. A hit sets bit
     * 3; when bits 5 and 4 are both clear it also sets the message id to 0 and requests the
     * message "Message Id" through fn_801FAF00 with the branch "Brunch", or, when that is
     * negative, with the branch 0, 1 or 2 equal to the word at 0x20 of fn_8006D6A8's manager (for
     * any other value of that word no request is made and the message id stays 0). Then, unless
     * the message id is -1, it calls fn_801D560C for sound 0xE01B (which returns 0 and does
     * nothing more when its lookup returns null), sets bit 5 and moves to state 2; this also
     * happens when no request was made. */
    void OnCollision(ColliDetectArgs &args);
    /* The draw layer's callback (name inferred): see the class comment. */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_target;
    /* 0x48 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x50 */ SetHintRingParams m_params;
    /* 0 after ReadOwner, 1 idle (the flag and model updates run), 2 from a collision that passes
     * the bit 4 and 5 tests; entered also when no message was requested (message id 0). UpdateBody
     * moves state 2 to 1 when fn_801FAAC8 returns true for the message id; ReadOwner (also through
     * ResetFromOwner) sets 0 from any state. */
    /* 0x64 */ s32 m_state;
    /* 0 from ReadOwner and before each request, then fn_801FAF00's result when one is made. */
    /* 0x68 */ s32 m_messageId;
    /* 0x6C */ UnkBitFlags_80014E04 m_flags;
    /* 0x70 */ RpClump *m_clump; /* the clump of HINTRING01.DFF */
    /* 0x74 */ s32 m_comId;      /* the owner's com id (fn_800C9E50) */
};
CHECK_SIZEOF(SetHintRing, 0x78);

} // namespace HintRing

#endif
