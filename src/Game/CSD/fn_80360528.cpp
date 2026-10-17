/* The Chao::CSD allocator hooks: the current CMemoryAlloc and the library's buffer allocation
 * through it. The original file name is unknown. Definitions are in reverse address order (the
 * library is compiled with deferred inlining, which emits the last definition first). */
#include "Game/CSD/csdBase.h"

namespace Chao {
namespace CSD {

static CMemoryAlloc *s_pMemoryAlloc;

CMemoryAlloc *SetMemoryAlloc(CMemoryAlloc *alloc) {
    if (alloc != NULL) {
        CMemoryAlloc *previous = s_pMemoryAlloc;
        s_pMemoryAlloc = alloc;
        return previous;
    }
    return s_pMemoryAlloc;
}

CMemoryAlloc *GetMemoryAlloc() {
    return s_pMemoryAlloc;
}

void *Alloc(size_t size, const char *file, int line) {
    return s_pMemoryAlloc->AllocBuffer(size, file, line);
}

void Free(void *ptr) {
    s_pMemoryAlloc->FreeBuffer(ptr);
}

} // namespace CSD
} // namespace Chao
