#ifndef GAME_CSD_CMEMORYALLOC_H
#define GAME_CSD_CMEMORYALLOC_H

#include <cstddef>
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* The allocator behind every allocation of the library (class name from RTTI). The game derives
 * its own allocator from it; the library supplies one for RenderWare. Slot names and the parameter
 * lists of Alloc and Free are inferred (the implementations read only the size or pointer).
 * The destructor and the default slots are inline; the copies kept in the game are the game's
 * own. */
class CMemoryAlloc {
  public:
    virtual ~CMemoryAlloc() {}
    virtual void *Alloc(size_t size) = 0;
    virtual void Free(void *ptr) = 0;
    /* Object allocations (CBase's operator new and others) go through it. */
    virtual void *AllocObject(size_t size, const char *file, int line) {
        return Alloc(size);
    }
    virtual void FreeObject(void *ptr) {
        Free(ptr);
    }
    /* Used by the library's other allocations (Chao::CSD::Alloc and Free). */
    virtual void *AllocBuffer(size_t size, const char *file, int line) {
        return Alloc(size);
    }
    virtual void FreeBuffer(void *ptr) {
        Free(ptr);
    }
};
CHECK_SIZEOF(Chao::CSD::CMemoryAlloc, 0x4);

} // namespace CSD
} // namespace Chao

#endif
