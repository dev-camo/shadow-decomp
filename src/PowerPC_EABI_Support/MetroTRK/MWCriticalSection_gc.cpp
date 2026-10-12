#include <dolphin/os.h>

extern "C" {

void MWInitializeCriticalSection(unsigned int *section) {}

void MWEnterCriticalSection(unsigned int *section) {
    *section = OSDisableInterrupts();
}

void MWExitCriticalSection(unsigned int *section) {
    OSRestoreInterrupts(*section);
}
}
