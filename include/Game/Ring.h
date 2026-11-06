#ifndef GAME_RING_H
#define GAME_RING_H

#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "rw/rpworld.h"
#include "layout_check.h"

class CharCommand;
class ColliDetectArgs;

namespace RegularRing {

/* The values of the "type" parameter, named after the descriptor's labels "type : normal",
 * "type : liner", "type : circle" and "type : parabola". */
enum { TYPE_NORMAL = 0, TYPE_LINER = 1, TYPE_CIRCLE = 2, TYPE_PARABOLA = 3 };

/* The gadget parameters in the order of the RING descriptor's table, whose entry names give the
 * member names. fn_800C9DBC reads them from the owner. The labels that the descriptor shows for
 * param0 and param1 depend on the type: "length" and nothing for a line, "radius" and nothing
 * for a circle, "length" and "height" for a parabola. */
struct RingParams {
    RingParams() : type(0), num(1), param0(0.0f), param1(0.0f), ghost(0) {}

    /* 0x00 */ s32 type;     /* TYPE_ values */
    /* 0x04 */ s32 num;      /* the number of rings */
    /* 0x08 */ float param0; /* the length of a line or parabola, the radius of a circle */
    /* 0x0C */ float param1; /* the height of a parabola */
    /* 0x10 */ s32 ghost;    /* 1 "ghost on", 0 "ghost off" */
};
CHECK_SIZEOF(RingParams, 0x14);

/* One ring of the gadget (placeholder name; the binary does not name the class). Ring keeps
 * them in an array made with new[] (constructor 0x8011464C, destructor 0x8011323C).
 *
 * Bit 1 of m_flags is set while the ring is there (collectable or shown as a ghost). Bit 2 draws
 * the ring with the second clump; the constructor starts every ring with bit 1 set and bit 2
 * clear, and nothing in the file clears bit 2 again. It is set when the ring is collected with
 * m_params.ghost == 1, and by RingState::Apply with ghost true for every ring whose saved bit is
 * clear, including rings past the count the state was made for, so it does not prove that the
 * ring was collected before. Bit 3 is set while Suspend's time runs (names of the bits inferred
 * from their uses). */
struct UnkRingElement_8011464C {
    UnkRingElement_8011464C();

    /* Sets bit 3 of m_flags and the time, clears bit 0 of the target's flag word when there is a
     * target and calls the probe's UnkSlot2 with -1 when there is a probe (name inferred). */
    void Suspend(float time);
    /* While bit 3 of m_flags is set, subtracts dt from the time; when the time is then <= 0 (a NaN
     * time is not), sets bit 0 of the target's flag word when there is a target, calls the
     * probe's UnkSlot1 with -1 when there is a probe and clears bit 3 (name inferred). */
    void UpdateSuspension(float dt);

    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ Fn8040Vector3 field_0xC; /* set to the clear value by CreateRings; not read here */
    /* 0x18 */ UnkRotation3 m_rotation; /* only y is used: the angle the ring is drawn at */
    /* 0x24 */ float m_phase;           /* the angle of the sine that turns the ring */
    /* 0x28 */ float field_0x28;        /* cleared by the constructor; not read here */
    /* 0x2C */ float field_0x2C;        /* cleared by the constructor; not read here */
    /* 0x30 */ RpClump *m_clump;        /* drawn while bit 2 of m_flags is clear */
    /* 0x34 */ RpClump *m_ghostClump;   /* drawn while bit 2 of m_flags is set */
    /* 0x38 */ boost::shared_ptr<Target> m_target;
    /* 0x40 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x48 */ float m_suspendTime; /* the time left of the suspension */
    /* 0x4C */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(UnkRingElement_8011464C, 0x50);

/* RegularRing::RingState (class name from RTTI, derived from SetAdapterState): the state that the
 * owner remembers between the creations of a Ring. Bit i of the 128-bit set is set for each ring
 * i whose bit 1 was set and bit 2 clear when the state was made; the bits of the rings past the
 * count stay clear. */
class RingState : public SetAdapterState {
  public:
    /* Clears the bits, then sets the bit of each ring whose bit 1 is set and bit 2 clear. */
    RingState(const UnkRingElement_8011464C *rings, int count);
    virtual ~RingState() {}

    /* For each ring: when its bit is set, sets bit 1 of the ring's flags; otherwise sets bit 2
     * when ghost is true, and clears bit 1 when it is false. A ring past the count the state was
     * made for has a clear bit, so it is treated the same way (name inferred). */
    void Apply(UnkRingElement_8011464C *rings, int count, bool ghost);

    /* 0x04 */ UnkBitFlags_80014E04 m_bits[4];
};
CHECK_SIZEOF(RingState, 0x14);

/* RegularRing::Ring (class name from RTTI; the gadget's descriptor is "RING"): a group of rings
 * (the "num" parameter) placed at the character info, along a line, around a circle or, for the
 * descriptor's "parabola" type, along a line with a sine-shaped offset along x (the "type"
 * parameter). The rings are drawn by a draw layer named "SetRing", and each has a Target and a
 * probe sphere. A ring is collected when its probe has hits and fn_8015B004 passes the hit
 * callback a character info whose fn_80078B2C returns true and for which the callback's
 * fn_80078B34 test also returns true, or when a command (ids 0x14 and 0xD) names its target. With
 * "ghost" equal to 1, a collected ring stays, is drawn with a second clump and is suspended for a
 * while; with any other value it is removed. The owner keeps a RingState with the rings that are
 * left (behaviour inferred). Method and member names marked "(name inferred)" are not original. */
class Ring : public Gadget {
  public:
    Ring(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    /* Unless fn_800C9870 returns true for the owner, replaces the owner's state with a new
     * RingState of the rings (the old one is deleted by fn_800C9894); then destroys the rings. */
    virtual ~Ring();
    /* State 0: when m_comId > 0, enters state 1, sets bit 0 of m_flags and clears the targets'
     * bit 0; otherwise enters state 2 and clears bit 0 of m_flags. State 1: once the com id's
     * entry in the manager of fn_800CB69C is 1, enters state 2, clears bit 0 of m_flags and sets
     * the targets' bit 0. State 2, only while there are rings: each ring that is there runs
     * UpdateSuspension(dt), adds 0.01 to its phase (once per update, not scaled by dt) and adds
     * 0.05 times the sine of the new phase to its y rotation. When no ring is there, it passes
     * itself to fn_8015A7C0 if m_params.ghost == 1 and to fn_8015A794 otherwise and sets Task flag
     * 0x01; otherwise it runs CollectTouchedRings. The state changes take effect in the next
     * update. */
    virtual void UpdateBody(float dt);
    /* Reads the owner again. When m_params.num differs from m_count and is > 0, makes the rings
     * anew and places them; when it equals the count, places the existing rings again (if there
     * are any); a num <= 0 that differs keeps the old rings. Then runs CollectTouchedRings,
     * UpdatePositions and fn_80112A04. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, its parameter block into
     * m_params only when the owner's parameter size is 0x14 (fn_800C9DBC compares it at
     * 0x800C9DD4) and its com id into m_comId, and clears the state and the flags (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When params.num > 0: makes params.num rings, applies the owner's RingState when the owner
     * holds one (with ghost true when m_params.ghost is not 0), gives each ring that is there a
     * target (fn_801E7128 with 8 when the ring's bit 2 is set after that, 0x188 otherwise) and a
     * probe sphere of radius 5, and creates the draw layer when there is none (name inferred). */
    void CreateRings(const RingParams &params);
    /* Releases the targets and the probes, deletes the rings and sets the count to 0 (name
     * inferred). */
    void DestroyRings();
    /* Places the rings that are there by params.type and turns each to the character info's y
     * rotation; does nothing while there are no rings (name inferred). */
    void PlaceRings(const RingParams &params);
    /* Collects each ring with a probe whose hits include a character info for which both
     * fn_80078B2C (tested by fn_8015B004) and fn_80078B34 (tested by the hit callback) return
     * true: in ghost mode (m_params.ghost == 1) the ring gets bit 2, Suspend(1.0f) and the
     * target flag word 8; otherwise its probe and target are released, bit 1 is cleared and
     * fn_8015D794 gets its position and 0x8A (name inferred). */
    void CollectTouchedRings();
    /* Moves each ring's target and probe to the ring's position (name inferred). */
    void UpdatePositions();
    /* Calls UnkSlot26 on the probe of each ring that has one (purpose unknown). */
    void fn_80112A04();
    /* Clears (ClearTargetFlags) or sets (SetTargetFlags) bit 0 of the flag word of the target of
     * each ring that is there and has a target (names inferred). */
    void ClearTargetFlags();
    void SetTargetFlags();
    /* Passes args to fn_8015B004 with the hit callback; true when fn_8015B004 called the
     * callback (only for a character info whose fn_80078B2C returns true) and the callback's
     * fn_80078B34 test passed (name inferred). */
    bool IsTouched(ColliDetectArgs &args, bool ghost);
    /* The character info's command handler (name inferred). */
    void OnCommand(CharCommand &command);
    /* The draw layer's callback: draws the rings that are there and in the camera's view
     * (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x30 */ UnkRingElement_8011464C *m_rings;
    /* 0x34 */ s32 m_count;
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ RingParams m_params;
    /* 0x54 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x5C */ s32 m_comId; /* the owner's com id (fn_800C9E50) */
    /* 0 at the start, 1 while waiting for the com id's switch, 2 once the rings are active
     * (inferred from UpdateBody). */
    /* 0x60 */ s32 m_state;
    /* Bit 0 is set while the state is 1, and OnDraw draws nothing while it is set. Hides
     * Task::m_flags. */
    /* 0x64 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(Ring, 0x68);

} // namespace RegularRing

#endif
