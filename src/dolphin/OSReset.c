// Dolphin SDK OS library: reset and shutdown.
// The following functions are not present as separate functions in this game's DOL; their bodies
// are unverified unless they are inlined into a function that is present: OSSetBootDol,
// OSUnregisterResetFunction, __OSShutdownDevices.
#include <dolphin.h>
#include <dolphin/os.h>

#include "__os.h"

// Queue helpers for the reset function list. They follow the thread queue macros in OSThread.c,
// but the list is always linked through the prev/next members, so there is no link parameter.
#define ENQUEUE_INFO(info, queue)                                                                  \
    do {                                                                                           \
        OSResetFunctionInfo *__prev = (queue)->tail;                                               \
        if (__prev == 0) {                                                                         \
            (queue)->head = (info);                                                                \
        } else {                                                                                   \
            __prev->next = (info);                                                                 \
        }                                                                                          \
        (info)->prev = __prev;                                                                     \
        (info)->next = 0;                                                                          \
        (queue)->tail = (info);                                                                    \
    } while (0)

#define DEQUEUE_INFO(info, queue)                                                                  \
    do {                                                                                           \
        OSResetFunctionInfo *__next = (info)->next;                                                \
        OSResetFunctionInfo *__prev = (info)->prev;                                                \
        if (__next == 0) {                                                                         \
            (queue)->tail = __prev;                                                                \
        } else {                                                                                   \
            __next->prev = __prev;                                                                 \
        }                                                                                          \
        if (__prev == 0) {                                                                         \
            (queue)->head = __next;                                                                \
        } else {                                                                                   \
            __prev->next = __next;                                                                 \
        }                                                                                          \
    } while (0)

#define ENQUEUE_INFO_PRIO(info, queue)                                                             \
    do {                                                                                           \
        OSResetFunctionInfo *__prev;                                                               \
        OSResetFunctionInfo *__next;                                                               \
        for (__next = (queue)->head; __next && (__next->priority <= (info)->priority);             \
             __next = __next->next)                                                                \
            ;                                                                                      \
                                                                                                   \
        if (__next == 0) {                                                                         \
            ENQUEUE_INFO(info, queue);                                                             \
        } else {                                                                                   \
            (info)->next = __next;                                                                 \
            __prev = __next->prev;                                                                 \
            __next->prev = (info);                                                                 \
            (info)->prev = __prev;                                                                 \
            if (__prev == 0) {                                                                     \
                (queue)->head = (info);                                                            \
            } else {                                                                               \
                __prev->next = (info);                                                             \
            }                                                                                      \
        }                                                                                          \
    } while (0)

static OSResetFunctionQueue ResetFunctionQueue;
static u32 bootThisDol;

static int CallResetFunctions(int final);
static void Reset(u32 resetCode);

void OSRegisterResetFunction(OSResetFunctionInfo *info) {
    ASSERTLINE(208, info->func);

    ENQUEUE_INFO_PRIO(info, &ResetFunctionQueue);
}

void OSUnregisterResetFunction(OSResetFunctionInfo *info) {
    DEQUEUE_INFO(info, &ResetFunctionQueue);
}

int __OSCallResetFunctions(BOOL final) {
    OSResetFunctionInfo *info;
    int err;
    u32 priority;

    priority = 0;
    err = 0;

    for (info = ResetFunctionQueue.head; info != 0;) {
        if (err != 0 && priority != info->priority)
            break;
        err |= !info->func(final);
        priority = info->priority;
        info = info->next;
    }

    err |= !__OSSyncSram();
    if (err) {
        return 0;
    }
    return 1;
}

#ifdef __GEKKO__
// The sequence enters through the hop_1..hop_4 branches, then sets a HID0 bit, waits on the
// timebase, writes the reset code to the reset register and spins.
// clang-format off
static asm void Reset(u32 resetCode) {
    nofralloc
    b hop_1
set_hid0:
    mfspr r8, HID0
    ori r8, r8, 0x8
    mtspr HID0, r8
    isync
    sync
    nop
    b delay_start
hop_1:
    b hop_2
delay_start:
    mftb r5, 268
delay_loop:
    mftb r6, 268
    subf r7, r5, r6
    cmplwi r7, 0x1124
    blt delay_loop
    nop
    b write_reset
hop_2:
    b hop_3
write_reset:
    lis r8, 0xcc00
    ori r8, r8, 0x3000
    li r4, 0x3
    stw r4, 0x24(r8)
    stw r3, 0x24(r8)
    nop
    b halt
hop_3:
    b hop_4
halt:
    nop
    b halt
hop_4:
    b set_hid0
}
// clang-format on
#endif

static void KillThreads(void) {
    OSThread *thread;
    OSThread *next;

    for (thread = __OSActiveThreadQueue.head; thread; thread = next) {
        next = thread->linkActive.next;
        switch (thread->state) {
        case OS_THREAD_STATE_READY:
        case OS_THREAD_STATE_WAITING:
            OSCancelThread(thread);
            continue;
        default:
            continue;
        }
    }
}

void __OSDoHotReset(u32 resetCode) {
    OSDisableInterrupts();
    __VIRegs[1] = 0;
    ICFlashInvalidate();
    Reset(resetCode * 8);
}

void __OSShutdownDevices(BOOL doRecal) {
    int rc;
    BOOL disableRecalibration;

    __OSStopAudioSystem();

    if (!doRecal) {
        disableRecalibration = __PADDisableRecalibration(TRUE);
    }

    do {
    } while (!__OSCallResetFunctions(FALSE));
    do {
    } while (!__OSSyncSram());

    OSDisableInterrupts();

    rc = __OSCallResetFunctions(TRUE);
    ASSERTLINE(408, rc);

    LCDisable();
    if (!doRecal) {
        __PADDisableRecalibration(disableRecalibration);
    }

    KillThreads();
}

void OSResetSystem(BOOL reset, u32 resetCode, BOOL forceMenu) {
    OSSram *sram;

    OSDisableScheduler();

    if (reset == TRUE && forceMenu) {
        sram = __OSLockSram();
        sram->flags |= 0x40;
        __OSUnlockSram(TRUE);

        resetCode = 0;
    }

    if (reset == OS_RESET_SHUTDOWN ||
        (reset == OS_RESET_RESTART && (bootThisDol || resetCode == 0xC0010000))) {
        __OSShutdownDevices(FALSE);
    } else {
        __OSShutdownDevices(TRUE);
    }

    if (reset == OS_RESET_HOTRESET) {
        __OSDoHotReset(resetCode);
    } else if (reset == OS_RESET_RESTART) {
        if (forceMenu == TRUE) {
            OSReport(
                "OSResetSystem(): You can't specify TRUE to forceMenu if you restart. Ignored\n");
        }
        OSEnableScheduler();
        __OSReboot(resetCode, bootThisDol);
    }

    memset(OSPhysicalToCached(0x40), 0, 0xcc - 0x40);
    memset(OSPhysicalToCached(0xd4), 0, 0xe8 - 0xd4);
    memset(OSPhysicalToCached(0xf4), 0, 0xf8 - 0xf4);
    memset(OSPhysicalToCached(0x3000), 0, 0xc0);
    memset(OSPhysicalToCached(0x30c8), 0, 0xd4 - 0xc8);
    // The byte at 0x800030E2 is __gUnknown800030E2, which OSExec sets to 1.
    memset(OSPhysicalToCached(0x30e2), 0, 1);
}

u32 OSGetResetCode(void) {
    u32 resetCode;
    if (__OSRebootParams.valid)
        resetCode = 0x80000000 | __OSRebootParams.restartCode;
    else
        resetCode = (__PIRegs[9] & 0xFFFFFFF8) / 8;

    return resetCode;
}

u32 OSSetBootDol(u32 dolOffset) {
    u32 oldDol;

    oldDol = bootThisDol;
    bootThisDol = dolOffset;
    return oldDol;
}
