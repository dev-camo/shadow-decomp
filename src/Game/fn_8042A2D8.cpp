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

    if (memory->m_pool == 0 || memory->m_pool->m_freeHead == 0) {
        return 0;
    }
    UnkEffectPoolEntry *entry = memory->m_pool->m_freeHead;
    memory->m_pool->m_freeHead = entry->next;
    if (memory->m_pool->m_freeHead == 0) {
        memory->m_pool->m_freeTail = 0;
    } else {
        memory->m_pool->m_freeHead->previous = 0;
    }
    --memory->m_pool->m_freeCount;
    if (memory->m_pool->m_usedTail != 0) {
        memory->m_pool->m_usedTail->next = entry;
        entry->previous = memory->m_pool->m_usedTail;
    } else {
        memory->m_pool->m_usedHead = entry;
        entry->previous = 0;
    }
    memory->m_pool->m_usedTail = entry;
    entry->next = 0;
    ++memory->m_pool->m_usedCount;
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
    if (entry == memory->m_pool->m_usedHead) {
        memory->m_pool->m_usedHead = entry->next;
    }
    if (entry == memory->m_pool->m_usedTail) {
        memory->m_pool->m_usedTail = entry->previous;
    }
    --memory->m_pool->m_usedCount;
    if (memory->m_pool->m_freeTail != 0) {
        memory->m_pool->m_freeTail->next = entry;
        entry->previous = memory->m_pool->m_freeTail;
    } else {
        memory->m_pool->m_freeHead = entry;
        entry->previous = 0;
    }
    memory->m_pool->m_freeTail = entry;
    entry->next = 0;
    ++memory->m_pool->m_freeCount;
}

namespace Effect {
namespace System {

UnkEffectPool::UnkEffectPool(unsigned int payloadSize, int count) {
    m_blockStride = payloadSize + sizeof(UnkEffectPoolEntry);
    unsigned int remainder = m_blockStride & 0xF;
    if (remainder != 0) {
        m_blockStride += 0x10 - remainder;
    }

    if (count != 0) {
        m_blockStorage = lbl_805E1508->Allocate(m_blockStride * count);
    } else {
        m_blockStorage = 0;
    }
    if (m_blockStorage == 0) {
        count = 0;
    }

    m_freeHead = static_cast<UnkEffectPoolEntry *>(m_blockStorage);
    m_freeTail = 0;
    m_usedTail = 0;
    m_usedHead = 0;
    m_usedCount = 0;
    m_freeCount = count;
    if (m_freeHead != 0) {
        UnkEffectPoolEntry *entry = m_freeHead;
        UnkEffectPoolEntry *previous = 0;
        for (int i = 0; i < count; ++i) {
            entry->previous = previous;
            previous = entry;
            entry->next = reinterpret_cast<UnkEffectPoolEntry *>(
                reinterpret_cast<unsigned char *>(entry) + m_blockStride);
            entry = entry->next;
        }
        // The original does not reject negative counts before this final link write.
        previous->next = 0;
        m_freeTail = previous;
    }
}

UnkEffectPool::~UnkEffectPool() {
    if (m_blockStorage != 0) {
        lbl_805E1508->Free(m_blockStorage);
    }
}

} // namespace System
} // namespace Effect
