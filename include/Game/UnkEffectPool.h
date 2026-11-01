#ifndef GAME_UNKEFFECTPOOL_H
#define GAME_UNKEFFECTPOOL_H

#include "Game/EffectSystem.h"
#include "Game/fn_8042A444.h"
#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectPoolEntry;

/* Placeholder type name; layout and lifecycle are inferred from fn_8042A344/fn_8042A2D8.
 * Member names are inferred from the pool operations. */
struct UnkEffectPool {
    UnkEffectPool(unsigned int payloadSize, int count);
    ~UnkEffectPool();

    static void *operator new(unsigned long size) throw() {
        return fn_8042A5D0()->Allocate(size);
    }
    static void operator delete(void *block);

    /* 0x00 */ UnkEffectPoolEntry *m_freeHead;
    /* 0x04 */ UnkEffectPoolEntry *m_freeTail;
    /* 0x08 */ UnkEffectPoolEntry *m_usedHead;
    /* 0x0C */ UnkEffectPoolEntry *m_usedTail;
    /* 0x10 */ int m_freeCount;
    /* 0x14 */ int m_usedCount;
    /* 0x18 */ void *m_blockStorage;
    /* 0x1C */ unsigned int m_blockStride;
};
CHECK_SIZEOF(UnkEffectPool, 0x20);

} // namespace System
} // namespace Effect

extern "C" {
/* Changes the allocator's current pool and returns its previous pool (inferred). */
Effect::System::UnkEffectPool *fn_8042A5C0(Effect::System::EffectSystem *memory,
                                           Effect::System::UnkEffectPool *pool);
}

#endif
