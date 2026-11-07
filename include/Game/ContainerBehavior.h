#ifndef GAME_CONTAINERBEHAVIOR_H
#define GAME_CONTAINERBEHAVIOR_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetModelInfo;
class UnkHandleBTarget_8015BE64;
class UnkIdList_8003A154;

/* The 0x20-byte block at 0x1C of ContainerBehavior (placeholder name; the original type name is
 * unknown). The ContainerBehavior constructor initialises the block with one call on this+0x1C
 * (USA (GUPE8P) 0x800F9398) to an out-of-line function (0x800F9570) that stores 0.0f to all eight
 * words, which sits among the constructor's out-of-line template and inline copies. That shows a
 * contiguous block initialised together, not its original C++ type: grouping it as one member
 * type is inferred, and so are the member names and that the copy is a default constructor.
 * Under this unit's -inline noauto,level=1,deferred setting, this default constructor stays out
 * of line because its body makes inline calls through the vector members' component constructors
 * (assigning the components in its body, which goes through their empty default constructors,
 * gives the same copy), whereas eight plain float members, or these members declared directly in
 * ContainerBehavior, are stored inline in the constructor. */
struct UnkState_800F9570 {
    UnkState_800F9570()
        : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), field_0x18(0.0f),
          field_0x1C(0.0f) {}

    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ UnkRotation3 m_rotation;
    /* Added to m_position.y on each update that does not return early; the ContainerBehavior
     * constructor stores -1.2. */
    /* 0x18 */ float field_0x18;
    /* Zeroed by the constructor; ProbeGround stores the y of the hit it finds here (it leaves the
     * value unchanged when it returns 0). ContainerBehavior::Update raises m_position.y to it in
     * the cases its comment lists. */
    /* 0x1C */ float field_0x1C;
};
CHECK_SIZEOF(UnkState_800F9570, 0x20);

/* ContainerBehavior (class name from RTTI, with no base class): a 0x44-byte object with a position
 * and a rotation. Unless Update returns early, it steps the position's y, may raise y to a stored
 * hit height and, when it has a probe, moves the probe to the position (conditions at Update). The
 * BOMB and CONTAINER gadgets each keep one in a shared pointer; copies of that shared pointer's
 * functions and its control block's constructor are in the BOMB gadget's file (the first at USA
 * (GUPE8P) 0x800EED84), and the CONTAINER gadget's file has its own local copies of the two
 * constructors that take the raw pointer (0x800F7BE0). Its out-of-line functions
 * (0x800F8D2C-0x800F95C0), vtable (0x80525560) and RTTI follow the CONTAINER gadget's file in a
 * file of their own (Game/ContainerBehavior.cpp).
 *
 * The layout comes from the constructor (0x800F9354) and the virtual destructor (0x800F9290).
 * Method names are inferred from what the bodies do, and so are the parameters' pointer and const
 * spellings. */
class ContainerBehavior {
  public:
    /* Copies *position to the state's position, keeps model in m_model, builds field_0xC for the
     * position and wraps a new box probe at the state's position and rotation in field_0x14
     * (0x800F9354; the parameter types are inferred: the BOMB and CONTAINER gadgets pass their
     * character info's position and a copy of their model pointer that they destroy after the
     * call). */
    ContainerBehavior(const Fn8040Vector3 *position, boost::shared_ptr<GadgetModelInfo> model);
    /* 0x800F9290. */
    virtual ~ContainerBehavior();

    /* Copies *position to the state's position and *rotation to its rotation (0x800F8FC4 calls
     * it; the original's out-of-line copy is at 0x800EF474). */
    void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
        m_state.m_position = *position;
        m_state.m_rotation = *rotation;
    }
    /* Copies the state's position to *position. The original calls its out-of-line copy
     * (0x800EF448) from all six callers (0x800EF1E8, 0x800F79E0, 0x800FAA64, 0x800FC0FC,
     * 0x800FDCF8 and 0x80278CF0). */
    void GetPosition(Fn8040Vector3 *position) {
        *position = m_state.m_position;
    }
    /* SetTransform with position and rotation, then sets field_0x40 and clears m_flags
     * (0x800F8FC4, behaviour from its body). */
    void Reset(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Does nothing when field_0xC is set and fn_80039EDC is true for it. Otherwise adds
     * field_0x18 to the state's position.y. While field_0x40 is set, it then calls ProbeGround for
     * the segment from 5 above to 100 below the new position (ProbeGround makes the query only
     * while m_model locks, and returns 0 otherwise); for a result of 1 or 2 it raises y to
     * field_0x1C when y is below it, and for 2 it first sets bit 0 of m_flags when
     * y - field_0x1C < 5 (which also holds when y is below field_0x1C, before that raise) and
     * afterwards clears field_0x40. While field_0x40 is clear, it raises y to field_0x1C when y is
     * below it and, if bit 0 of m_flags is clear, sets it and requests sound 0x1003 at the
     * position. Then, when field_0x14 is set, it moves that probe to the position and rotation,
     * passes it half the height above field_0x1C as its y size when that is greater than 0, and
     * passes (0, -half that height, 0) to its UnkSlot17. Each of these comparisons is false for a
     * NaN (0x800F9000; it takes no dt). */
    void Update();
    /* Asks fn_80021E24 (with 3 and 8) for the hit along the segment from start to end, skipping
     * the collision object that the model's fn_80161614(0) returns (name inferred; the int return
     * and the parameter spellings are inferred too). Returns 0 when the model has expired, when
     * there is no hit node, or when the node has no hit (that node is not deleted). Otherwise it
     * stores the hit point's y in *groundY, deletes the node and returns 2 when fn_800F8E88 is
     * true for the node, else 1. */
    int ProbeGround(const Fn8040Vector3 &start, const Fn8040Vector3 &end, float *groundY);

    /* 0x04 */ boost::weak_ptr<GadgetModelInfo> m_model;
    /* Built for the position by the constructor (fn_8003A154 may leave it empty); Update does
     * nothing while it is set and fn_80039EDC is true for it. */
    /* 0x0C */ boost::shared_ptr<UnkIdList_8003A154> field_0xC;
    /* 0x14 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> field_0x14;
    /* 0x1C */ UnkState_800F9570 m_state;
    /* 0x3C */ u32 m_flags; /* cleared by the constructor and Reset; Update sets bit 0 */
    /* While set (by the constructor and Reset), each Update that does not return early calls
     * ProbeGround for the segment from 5 above to 100 below the stepped position (no query is made
     * while m_model does not lock); Update clears it when ProbeGround returns 2. */
    /* 0x40 */ u8 field_0x40;
};
CHECK_SIZEOF(ContainerBehavior, 0x44);

#endif
