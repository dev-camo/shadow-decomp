/* The Chao::CSD platform hooks: the current CPlatform, its Begin/End nesting and the message
 * functions built on its Print. The original file name is unknown. Definitions are in reverse
 * address order (deferred inlining emits the last definition first). */
#include "Game/CSD/csdBase.h"
#include "Game/CSD/CPlatform.h"
#include <cstdarg>
#include <cstdio>

namespace Chao {
namespace CSD {

static CPlatform *s_pPlatform;

void SetPlatform(CPlatform *platform) {
    s_pPlatform = platform;
}

CPlatform *GetPlatform() {
    return s_pPlatform;
}

void CPlatform::Begin() {
    if (m_beginCount == 0) {
        OnBegin();
    }
    m_beginCount++;
}

void CPlatform::End() {
    m_beginCount--;
    if (m_beginCount == 0) {
        OnEnd();
    }
}

void PrintError(const char *format, ...) {
    char message[256];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    s_pPlatform->Print(message);
}

void PrintFatalError(const char *format, ...) {
    char message[256];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    s_pPlatform->Print(message);
    for (;;) {
    }
}

} // namespace CSD
} // namespace Chao

/* Called by the game when it shuts the library down (behaviour inferred). */
extern "C" void fn_80361CF8(void) {
    Chao::CSD::TermLibrary();
}
