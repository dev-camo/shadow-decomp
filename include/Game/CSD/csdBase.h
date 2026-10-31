/* csdBase.h (original name from the library's allocation calls, which pass __FILE__): CBase, the
 * base class of the library's heap objects. Its operator new must stay on line 40 (see the
 * Matching workaround below the class). Other Chao::CSD base services are assumed declared here. */
#ifndef GAME_CSD_CSDBASE_H
#define GAME_CSD_CSDBASE_H

#include <cstddef>
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
void *Alloc(size_t size, const char *file, int line);
void Free(void *ptr);

/* The current platform (names inferred). */
CPlatform *GetPlatform();
void SetPlatform(CPlatform *platform);

/* Formats a printf-style message and passes it to the platform's Print (name inferred). */
void PrintError(const char *format, ...);

/* As PrintError, then never returns (name inferred). */
void PrintFatalError(const char *format, ...);

/* Heap objects of the library allocate through the current allocator (CMemoryAlloc's object
 * slots). */
class CBase {
  public:
    static void *operator new(size_t size) {
        return GetMemoryAlloc()->AllocObject(size, __FILE__, __LINE__);
    }
    static void operator delete(void *ptr) {
        GetMemoryAlloc()->FreeObject(ptr);
    }

    /* An empty constructor: with it the constructors of derived classes are emitted out of line,
     * as in the original. */
    CBase() {}
};
CHECK_SIZEOF(Chao::CSD::CBase, 0x1);

/* Matching workaround: InitLibrary and TermLibrary are declared after CBase so that the
 * allocation in CBase::operator new is on line 40, the __LINE__ the original's CBase allocations
 * pass (declared above the class, they put it on line 45). Keep that line when editing above it;
 * revisit if the original header layout is recovered. */

/* Set up and tear down the matrix stack and the registry of texture-list types (names
 * inferred). */
void InitLibrary();
void TermLibrary();

} // namespace CSD
} // namespace Chao

#endif
