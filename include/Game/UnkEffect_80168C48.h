#ifndef GAME_UNKEFFECT_80168C48_H
#define GAME_UNKEFFECT_80168C48_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The effect that fn_80168C48 creates (placeholder name). Only the members used through this
 * class are declared, and the size check covers only them: fn_80425E4C stores a float at 0xB0,
 * so the object is at least 0xB4 bytes. */
class UnkEffect_80168C48 {
  public:
    /* Clears the bits of mask in the flag word (name inferred). Called as a statement, it leaves
     * the smart pointer's operator-> out of line, as in the original; a plain field access expands
     * it in place. */
    void ClearFlags(u32 mask) {
        m_flags &= ~mask;
    }

    /* 0x00 */ u8 field_0x0[0x18];
    /* Bit 0 is cleared when the candle stick stops its effect (behaviour inferred). */
    /* 0x18 */ u32 m_flags;
};
CHECK_SIZEOF(UnkEffect_80168C48, 0x1C);

extern "C" {
/* Creates an effect at position (the other arguments are not understood). */
boost::weak_ptr<UnkEffect_80168C48> fn_80168C48(const Fn8040Vector3 *position,
                                                const Fn8040Vector3 *arg1, int arg2);
/* Stores position's x, y and z at 0x90, 0xA0 and 0xB0 of the effect (behaviour inferred). */
void fn_80425E4C(UnkEffect_80168C48 *effect, const Fn8040Vector3 *position);
}

#endif
