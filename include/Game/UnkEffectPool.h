#ifndef GAME_UNKEFFECTPOOL_H
#define GAME_UNKEFFECTPOOL_H

#include "Game/EffectSystem.h"
#include "Game/fn_8042A444.h"
#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectPoolEntry;

/* Placeholder type name; layout and lifecycle are inferred from fn_8042A344/fn_8042A2D8. */
struct UnkEffectPool {
    UnkEffectPool(unsigned int payloadSize, int count);
    ~UnkEffectPool();

    static void *operator new(unsigned long size) throw() {
        return fn_8042A5D0()->Allocate(size);
    }
    static void operator delete(void *block);

    /* 0x00 */ UnkEffectPoolEntry *field_0x0;
    /* 0x04 */ UnkEffectPoolEntry *field_0x4;
    /* 0x08 */ UnkEffectPoolEntry *field_0x8;
    /* 0x0C */ UnkEffectPoolEntry *field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ void *field_0x18;
    /* 0x1C */ unsigned int field_0x1C;
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
