#include <dolphin/base/PPCArch.h>
#include <dolphin/os.h>
#include <stdlib.h>

#include "__os.h"

__declspec(section ".ctors") extern void (*_ctors[])();
__declspec(section ".dtors") extern void (*_dtors[])();

static void __init_cpp(void);
static void __fini_cpp(void);

#ifdef __GEKKO__
// clang-format off
__declspec(section ".init") asm void __init_hardware(void) {
    nofralloc
    mfmsr r0
    ori r0, r0, MSR_FP
    mtmsr r0
    mflr r31
    bl __OSPSInit
    bl __OSFPRInit
    bl __OSCacheInit
    mtlr r31
    blr
}

// Writes back the data cache and invalidates the instruction cache over the given range
__declspec(section ".init") asm void __flush_cache(void *address, unsigned int size) {
    nofralloc
    lis r5, 0xffff
    ori r5, r5, 0xfff1
    and r5, r5, r3
    subf r3, r5, r3
    add r4, r4, r3
rept:
    dcbst 0, r5
    sync
    icbi 0, r5
    addic r5, r5, 0x8
    subic. r4, r4, 0x8
    bge rept
    isync
    blr
}
// clang-format on
#endif

void __init_user(void) {
    __init_cpp();
}

static void __init_cpp(void) {
    void (**constructor)();

    for (constructor = _ctors; *constructor; constructor++) {
        (*constructor)();
    }
}

static void __fini_cpp(void) {
    void (**destructor)();

    for (destructor = _dtors; *destructor; destructor++) {
        (*destructor)();
    }
}

__declspec(weak) void abort(void) {
    _ExitProcess();
}

__declspec(weak) void exit(int status) {
    __fini_cpp();
    _ExitProcess();
}

void _ExitProcess(void) {
    PPCHalt();
}
