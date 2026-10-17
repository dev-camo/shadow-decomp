/* csdBase.h (original name from the library's allocation calls, which pass __FILE__): CBase, the
 * base class of the library's heap objects, whose operator new passes __LINE__ 40 of the original
 * header. The other Chao::CSD base services are assumed to be declared here too. */
#ifndef GAME_CSD_CSDBASE_H
#define GAME_CSD_CSDBASE_H

#include <stddef.h>
#include "Game/CSD/CMemoryAlloc.h"
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* Drawing and message output for the target system (see CPlatform.h). */
class CPlatform;

/* The current allocator. SetMemoryAlloc ignores null, returns the previous one (names inferred). */
CMemoryAlloc *GetMemoryAlloc();
CMemoryAlloc *SetMemoryAlloc(CMemoryAlloc *alloc);

/* Buffer allocation through the current allocator; callers pass __FILE__ and __LINE__. */
void *Alloc(unsigned long size, const char *file, int line);
void Free(void *ptr);

/* The current platform (names inferred). */
CPlatform *GetPlatform();
void SetPlatform(CPlatform *platform);

/* Formats a printf-style message and passes it to the platform's Print (name inferred). */
void PrintError(const char *format, ...);

/* As PrintError, then never returns (name inferred). */
void PrintFatalError(const char *format, ...);

/* Heap objects of the library allocate through the current allocator (sizes are unsigned long,
 * MWCC's size type, as in CMemoryAlloc). */
class CBase {
  public:
    static void *operator new(unsigned long size) {
        return GetMemoryAlloc()->AllocObject(size, __FILE__, __LINE__);
    }
    static void operator delete(void *ptr) {
        GetMemoryAlloc()->FreeObject(ptr);
    }
};
CHECK_SIZEOF(Chao::CSD::CBase, 0x1);

} // namespace CSD
} // namespace Chao

#endif
