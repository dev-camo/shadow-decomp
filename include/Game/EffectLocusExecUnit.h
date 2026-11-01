#ifndef GAME_EFFECTLOCUSEXECUNIT_H
#define GAME_EFFECTLOCUSEXECUNIT_H

#include "Game/fn_8040_vector_types.h"
#include "Game/fn_8042A444.h"
#include "layout_check.h"

namespace Effect {
namespace System {

/* Class name comes from RTTI; member names are inferred from matrix and list operations. */
class EffectLocusExecUnit {
  public:
    EffectLocusExecUnit();
    virtual ~EffectLocusExecUnit();

    static void *operator new(unsigned long) throw() {
        return fn_8042A50C(fn_8042A5D0());
    }

    static void operator delete(void *block) {
        fn_8042A444(fn_8042A5D0(), block);
    }

    /* 0x04 */ Fn8040Matrix34 m_transform;
    /* 0x34 */ float m_age;
    /* 0x38 */ EffectLocusExecUnit *m_prev;
    /* 0x3C */ EffectLocusExecUnit *m_next;
};
CHECK_SIZEOF(EffectLocusExecUnit, 0x40);

} // namespace System
} // namespace Effect

#endif
