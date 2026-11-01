/* Effect pool routines; original type and source-file names are unknown. */
#include "Game/UnkEffectPool.h"

namespace Effect {
namespace System {

/* Link names are inferred. The payload begins 0x10 bytes after this header;
 * the final eight header bytes are not accessed by the recovered functions. */
struct UnkEffectPoolEntry {
    /* 0x00 */ UnkEffectPoolEntry *next;
    /* 0x04 */ UnkEffectPoolEntry *previous;
    /* 0x08 */ unsigned char field_0x8[8];
};
CHECK_SIZEOF(UnkEffectPoolEntry, 0x10);

} // namespace System
} // namespace Effect

Effect::System::EffectSystem *lbl_805E1508;

extern "C" void fn_8042A5E0(Effect::System::EffectSystem *memory) {
    lbl_805E1508 = memory;
}

extern "C" Effect::System::EffectSystem *fn_8042A5D0() throw() {
    return lbl_805E1508;
}

extern "C" Effect::System::UnkEffectPool *fn_8042A5C0(Effect::System::EffectSystem *memory,
                                                      Effect::System::UnkEffectPool *pool) {
    Effect::System::UnkEffectPool *previous = memory->m_pool;
    memory->m_pool = pool;
    return previous;
}

extern "C" void *fn_8042A50C(Effect::System::EffectSystem *memory) throw() {
    using Effect::System::UnkEffectPoolEntry;

    if (memory->m_pool == 0 || memory->m_pool->field_0x0 == 0) {
        return 0;
    }
    UnkEffectPoolEntry *entry = memory->m_pool->field_0x0;
    memory->m_pool->field_0x0 = entry->next;
    if (memory->m_pool->field_0x0 == 0) {
        memory->m_pool->field_0x4 = 0;
    } else {
        memory->m_pool->field_0x0->previous = 0;
    }
    --memory->m_pool->field_0x10;
    if (memory->m_pool->field_0xC != 0) {
        memory->m_pool->field_0xC->next = entry;
        entry->previous = memory->m_pool->field_0xC;
    } else {
        memory->m_pool->field_0x8 = entry;
        entry->previous = 0;
    }
    memory->m_pool->field_0xC = entry;
    entry->next = 0;
    ++memory->m_pool->field_0x14;
    return entry + 1;
}

extern "C" void fn_8042A444(Effect::System::EffectSystem *memory, void *block) {
    using Effect::System::UnkEffectPoolEntry;

    if (block == 0) {
        return;
    }
    UnkEffectPoolEntry *entry = static_cast<UnkEffectPoolEntry *>(block) - 1;
    if (entry->previous != 0) {
        entry->previous->next = entry->next;
    }
    if (entry->next != 0) {
        entry->next->previous = entry->previous;
    }
    if (entry == memory->m_pool->field_0x8) {
        memory->m_pool->field_0x8 = entry->next;
    }
    if (entry == memory->m_pool->field_0xC) {
        memory->m_pool->field_0xC = entry->previous;
    }
    --memory->m_pool->field_0x14;
    if (memory->m_pool->field_0x4 != 0) {
        memory->m_pool->field_0x4->next = entry;
        entry->previous = memory->m_pool->field_0x4;
    } else {
        memory->m_pool->field_0x0 = entry;
        entry->previous = 0;
    }
    memory->m_pool->field_0x4 = entry;
    entry->next = 0;
    ++memory->m_pool->field_0x10;
}

namespace Effect {
namespace System {

UnkEffectPool::UnkEffectPool(unsigned int payloadSize, int count) {
    field_0x1C = payloadSize + sizeof(UnkEffectPoolEntry);
    unsigned int remainder = field_0x1C & 0xF;
    if (remainder != 0) {
        field_0x1C += 0x10 - remainder;
    }

    if (count != 0) {
        field_0x18 = lbl_805E1508->Allocate(field_0x1C * count);
    } else {
        field_0x18 = 0;
    }
    if (field_0x18 == 0) {
        count = 0;
    }

    field_0x0 = static_cast<UnkEffectPoolEntry *>(field_0x18);
    field_0x4 = 0;
    field_0xC = 0;
    field_0x8 = 0;
    field_0x14 = 0;
    field_0x10 = count;
    if (field_0x0 != 0) {
        UnkEffectPoolEntry *entry = field_0x0;
        UnkEffectPoolEntry *previous = 0;
        for (int i = 0; i < count; ++i) {
            entry->previous = previous;
            previous = entry;
            entry->next = reinterpret_cast<UnkEffectPoolEntry *>(
                reinterpret_cast<unsigned char *>(entry) + field_0x1C);
            entry = entry->next;
        }
        // The original does not reject negative counts before this final link write.
        previous->next = 0;
        field_0x4 = previous;
    }
}

UnkEffectPool::~UnkEffectPool() {
    if (field_0x18 != 0) {
        lbl_805E1508->Free(field_0x18);
    }
}

} // namespace System
} // namespace Effect
