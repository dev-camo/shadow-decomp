/* Sonicteam::System::IHeap and Sonicteam::System::Heap (class names from RTTI): a first-fit
 * allocator over one buffer, and a small tick-count source that precedes it in the binary. The
 * original file name is unknown.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include <algorithm>
#include <cstring>

#include <dolphin/os.h>
#include <dolphin/types.h>
#include "layout_check.h"

/* Rounds value up to a multiple of the power-of-two alignment (name inferred). The behaviour comes
 * from its code; the parameter types are unverified, since its own unit is not decompiled. */
u32 AlignUp(u32 value, u32 alignment);

/* A 64-bit tick count (placeholder type: only its size and use are known). */
struct UnkTick {
    UnkTick(u32 count) : ticks(count) {}

    u64 ticks;
};
CHECK_SIZEOF(UnkTick, 0x8);

/* An object without data that the game allocates into a global pointer and deletes again, and that
 * reads the current tick (placeholder class: its name is unknown). */
class UnkTickSource_8040ABC8 {
  public:
    UnkTickSource_8040ABC8();
    ~UnkTickSource_8040ABC8();

    /* Returns the current value of the processor tick counter (name inferred). */
    UnkTick GetTick();
};
CHECK_SIZEOF(UnkTickSource_8040ABC8, 0x1);

namespace Sonicteam {
namespace System {

/* Interface of the allocators (class name from RTTI). */
class IHeap {
  public:
    /* Allocates size bytes and returns them, or 0 (name inferred). */
    virtual void *Alloc(u32 size) = 0;
    /* Allocates size bytes at an address that is a multiple of alignment (name inferred). */
    virtual void *AllocAligned(u32 alignment, u32 size) = 0;
    /* Allocates size bytes and clears them (name inferred). */
    virtual void *AllocZeroed(u32 size) = 0;
    /* Unknown no-op in the interface (behaviour inferred from the empty default). */
    virtual void Unknown3() {}
    /* Returns a block from Alloc to the heap (name inferred). */
    virtual void Free(void *block) = 0;
    /* Returns the size of the largest free block (name inferred). */
    virtual u32 GetLargestFreeSize() = 0;
    /* Resizes a block, moving it when necessary (name inferred). */
    virtual void *Realloc(void *block, u32 size) = 0;
    /* Returns the total size of all free blocks (name inferred). */
    virtual u32 GetFreeSize() = 0;
};
CHECK_SIZEOF(IHeap, 0x4);

/* Every block starts with this header. A free block heads a list sorted by address; the header of
 * an allocated block keeps its size (header included) at the same place. The block's data follows
 * the header. */
struct HeapBlock {
    /* Returns the address that lies the given number of bytes after this block. */
    inline HeapBlock *Offset(u32 bytes) {
        return reinterpret_cast<HeapBlock *>(reinterpret_cast<char *>(this) + bytes);
    }

    /* Returns the number of bytes from this block to a later one. */
    inline u32 BytesTo(HeapBlock *later) {
        return reinterpret_cast<char *>(later) - reinterpret_cast<char *>(this);
    }

    /* 0x00 */ HeapBlock *next;
    /* 0x04 */ u32 size;
};
CHECK_SIZEOF(HeapBlock, 0x8);

/* First-fit allocator over the buffer it is given (class name from RTTI). */
class Heap : public IHeap {
  public:
    Heap(void *buffer, u32 size);

    virtual void *Alloc(u32 size);
    virtual void *AllocAligned(u32 alignment, u32 size);
    virtual void *AllocZeroed(u32 size);
    virtual void Unknown3();
    virtual void Free(void *block);
    virtual u32 GetLargestFreeSize();
    virtual void *Realloc(void *block, u32 size);
    virtual u32 GetFreeSize();

    /* 0x04 */ HeapBlock *m_freeList; /* sentinel of size 0 whose next is the first free block */
};
CHECK_SIZEOF(Heap, 0x8);

Heap::Heap(void *buffer, u32 size) {
    m_freeList = static_cast<HeapBlock *>(buffer);

    HeapBlock *firstFree = m_freeList + 1;
    m_freeList->next = firstFree;
    m_freeList->size = 0;
    firstFree->next = 0;
    firstFree->size = size - sizeof(HeapBlock);
}

void *Heap::Alloc(u32 size) {
    u32 blockSize = AlignUp(size + sizeof(HeapBlock), 8);

    HeapBlock *previous;
    HeapBlock *block;
    for (previous = m_freeList; (block = previous->next) != 0; previous = block) {
        if (block->size >= blockSize) {
            if (blockSize == block->size) {
                previous->next = block->next;
            } else {
                // Matching workaround: not block->Offset(). That form gives the registers a
                // different allocation here; revisit if the registers are matched another way.
                HeapBlock *rest =
                    reinterpret_cast<HeapBlock *>(reinterpret_cast<char *>(block) + blockSize);
                rest->next = block->next;
                rest->size = block->size - blockSize;
                block->size = blockSize;
                previous->next = rest;
            }
            return block + 1;
        }
    }
    return 0;
}

void *Heap::AllocAligned(u32 alignment, u32 size) {
    u32 blockSize = AlignUp(size + sizeof(HeapBlock), 8);

    HeapBlock *previous = m_freeList;
    HeapBlock *block;
    // Room for the block plus the padding that aligning its data may need.
    u32 searchSize = alignment + blockSize;
    for (; (block = previous->next) != 0; previous = block) {
        if (block->size >= searchSize) {
            u32 alignedData = AlignUp(reinterpret_cast<u32>(block + 1), alignment);
            HeapBlock *allocated = reinterpret_cast<HeapBlock *>(alignedData) - 1;
            if (searchSize == block->size) {
                if (allocated == block) {
                    previous->next = block->next;
                } else {
                    block->size = block->BytesTo(allocated);
                    allocated->size = searchSize - block->size;
                }
            } else {
                HeapBlock *rest = block->Offset(searchSize);
                rest->next = block->next;
                rest->size = block->size - searchSize;
                if (allocated == block) {
                    block->size = searchSize;
                    previous->next = rest;
                } else {
                    block->size = block->BytesTo(allocated);
                    block->next = rest;
                    allocated->size = searchSize - block->size;
                }
            }
            return allocated + 1;
        }
    }
    return 0;
}

void *Heap::AllocZeroed(u32 size) {
    void *block = Alloc(size);
    if (block != 0) {
        memset(block, 0, size);
    }
    return block;
}

void Heap::Unknown3() {}

void Heap::Free(void *block) {
    if (block == 0) {
        return;
    }

    HeapBlock *freed = static_cast<HeapBlock *>(block) - 1;

    // Find the free blocks on either side of the returned one.
    HeapBlock *previous;
    HeapBlock *next;
    for (previous = m_freeList; (next = previous->next) < freed && next != 0; previous = next) {
    }

    // Merge with the following free block when they touch.
    if (next != 0) {
        if (freed->Offset(freed->size) == next) {
            freed->next = next->next;
            freed->size += next->size;
        } else {
            freed->next = next;
        }
    } else {
        freed->next = 0;
    }

    // Merge with the preceding free block when they touch.
    if (previous->Offset(previous->size) == freed) {
        previous->next = freed->next;
        previous->size += freed->size;
    } else {
        previous->next = freed;
    }
}

u32 Heap::GetLargestFreeSize() {
    u32 largest = 0;
    for (HeapBlock *block = m_freeList->next; block != 0; block = block->next) {
        if (largest < block->size) {
            largest = block->size;
        }
    }
    return largest;
}

void *Heap::Realloc(void *block, u32 size) {
    if (block == 0) {
        return Alloc(size);
    }

    HeapBlock *header = static_cast<HeapBlock *>(block) - 1;
    if (header->size == size) {
        return block;
    }

    void *moved = Alloc(size);
    if (moved == 0) {
        return 0;
    }

    u32 oldSize = header->size - sizeof(HeapBlock);
    memcpy(moved, block, std::min(oldSize, size));
    Free(block);
    return moved;
}

u32 Heap::GetFreeSize() {
    u32 total = 0;
    for (HeapBlock *block = m_freeList->next; block != 0; block = block->next) {
        total += block->size;
    }
    return total;
}

} // namespace System
} // namespace Sonicteam

UnkTickSource_8040ABC8::UnkTickSource_8040ABC8() {}

UnkTickSource_8040ABC8::~UnkTickSource_8040ABC8() {}

UnkTick UnkTickSource_8040ABC8::GetTick() {
    return UnkTick(OSGetTick());
}
