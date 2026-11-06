#ifndef GAME_LINETARGET_H
#define GAME_LINETARGET_H

#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

/* LineTarget (class name from the RTTI record at 0x805E8CF8, with Target as its base): the
 * 0x50-byte object that fn_801E29D8 creates. Its constructor (0x801E2D18) runs Target's
 * constructor at 0x801E7490, sets the vtable at 0x8053D86C and default-constructs the segment at
 * 0x38. The vtable also overrides Target's first slot (0x801E2C4C), which is not declared here
 * because nothing in the reconstructed code calls it. */
class LineTarget : public Target {
  public:
    /* Defined elsewhere (0x801E2CC0). */
    virtual ~LineTarget();

    /* Set by fn_801E2C9C (the TurnOver gadget passes the segment between two frames of its
     * model). */
    /* 0x38 */ UnkSegment_80020A20 m_segment;
};
CHECK_SIZEOF(LineTarget, 0x50);

extern "C" {
/* Allocates a LineTarget for the character info (constructor at 0x801E2D18, with info and arg1
 * passed on; the number is not understood: the TurnOver gadget passes 0x40), owns it in a shared
 * pointer with the deleter at 0x801E2C0C, passes it as a shared pointer to Target to fn_801E7AF4
 * together with the object that fn_80060058 returns, and returns it (behaviour inferred from its
 * body). When the allocation fails, the constructor is skipped and the null pointer goes on to
 * fn_801E7AF4, which reads the target's word at 0x30 (0x801E7B0C) without a null check, so the
 * null target is dereferenced there before this function could return it. Passing info by value
 * is inferred, as for fn_801E7128. */
boost::shared_ptr<LineTarget> fn_801E29D8(boost::weak_ptr<CharInfo> info, int arg1);
/* Assigns segment to the object's m_segment (behaviour from its body). Passing the segment by
 * reference is inferred: the bytes pass its address. */
void fn_801E2C9C(LineTarget *object, const UnkSegment_80020A20 &segment);
}

#endif
