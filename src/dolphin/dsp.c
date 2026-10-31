// Dolphin SDK DSP library: DSP control and mailboxes.
// The following functions are not present as separate functions in this game's DOL; their bodies
// are unverified unless they are inlined into a function that is present: DSPAssertInt,
// DSPGetDMAStatus, DSPHalt, DSPReadCPUToDSPMbox, DSPReset, DSPUnhalt.
#include <stddef.h>
#include <dolphin.h>
#include <dolphin/hw_regs.h>

#include "__dsp.h"

#define BUILD_DATE "Apr  5 2004"
#if DEBUG
#define BUILD_TIME "03:56:49"
#else
#define BUILD_TIME "04:15:32"
#endif

#if DEBUG
const char *__DSPVersion = "<< Dolphin SDK - DSP\tdebug build: Apr  5 2004 03:56:49 (0x2301) >>";
#else
const char *__DSPVersion = "<< Dolphin SDK - DSP\trelease build: Apr  5 2004 04:15:32 (0x2301) >>";
#endif

static BOOL __DSP_init_flag;

u32 DSPCheckMailToDSP(void) {
    return (__DSPRegs[DSP_MAILBOX_IN_HI] & (1 << 15)) >> 15;
}

u32 DSPCheckMailFromDSP(void) {
    return (__DSPRegs[DSP_MAILBOX_OUT_HI] & (1 << 15)) >> 15;
}

u32 DSPReadCPUToDSPMbox(void) {
    return (__DSPRegs[DSP_MAILBOX_IN_HI] << 16) | __DSPRegs[DSP_MAILBOX_IN_LO];
}

u32 DSPReadMailFromDSP(void) {
    return (__DSPRegs[DSP_MAILBOX_OUT_HI] << 16) | __DSPRegs[DSP_MAILBOX_OUT_LO];
}

void DSPSendMailToDSP(u32 mail) {
    __DSPRegs[DSP_MAILBOX_IN_HI] = mail >> 16;
    __DSPRegs[DSP_MAILBOX_IN_LO] = mail & 0xFFFF;
}

void DSPAssertInt(void) {
    BOOL old;
    u16 tmp;

    old = OSDisableInterrupts();
    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    tmp = (tmp & ~0xA8) | 2;
    __DSPRegs[DSP_CONTROL_STATUS] = tmp;
    OSRestoreInterrupts(old);
}

void DSPInit(void) {
    BOOL old;
    u16 tmp;

    __DSP_debug_printf("DSPInit(): Build Date: %s %s\n", BUILD_DATE, BUILD_TIME);

    if (__DSP_init_flag == 1)
        return;

    OSRegisterVersion(__DSPVersion);

    old = OSDisableInterrupts();
    __OSSetInterruptHandler(__OS_INTERRUPT_DSP_DSP, __DSPHandler);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_DSP_DSP);

    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    tmp = (tmp & ~0xA8) | 0x800;
    __DSPRegs[DSP_CONTROL_STATUS] = tmp;

    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    __DSPRegs[DSP_CONTROL_STATUS] = tmp = tmp & ~0xAC;

    __DSP_first_task = __DSP_last_task = __DSP_curr_task = __DSP_tmp_task = NULL;
    __DSP_init_flag = 1;

    OSRestoreInterrupts(old);
}

BOOL DSPCheckInit(void) {
    return __DSP_init_flag;
}

void DSPReset(void) {
    BOOL old;
    u16 tmp;

    old = OSDisableInterrupts();
    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    tmp = (tmp & ~0xA8) | 0x800 | 1;
    __DSPRegs[DSP_CONTROL_STATUS] = tmp;
    __DSP_init_flag = 0;
    OSRestoreInterrupts(old);
}

void DSPHalt(void) {
    BOOL old;
    u16 tmp;

    old = OSDisableInterrupts();
    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    tmp = (tmp & ~0xA8) | 4;
    __DSPRegs[DSP_CONTROL_STATUS] = tmp;
    OSRestoreInterrupts(old);
}

void DSPUnhalt(void) {
    BOOL old;
    u16 tmp;

    old = OSDisableInterrupts();
    tmp = __DSPRegs[DSP_CONTROL_STATUS];
    tmp = (tmp & ~0xAC);
    __DSPRegs[DSP_CONTROL_STATUS] = tmp;
    OSRestoreInterrupts(old);
}

u32 DSPGetDMAStatus(void) {
    return (__DSPRegs[DSP_CONTROL_STATUS] & (1 << 9));
}

DSPTaskInfo *DSPAddTask(DSPTaskInfo *task) {
    BOOL old;

    ASSERTMSGLINE(556, __DSP_init_flag == 1, "DSPAddTask(): DSP driver not initialized!\n");

    old = OSDisableInterrupts();

    __DSP_insert_task(task);
    task->state = 0;
    task->flags = 1;

    OSRestoreInterrupts(old);
    if (task == __DSP_first_task)
        __DSP_boot_task(task);
    return task;
}

DSPTaskInfo *DSPCancelTask(DSPTaskInfo *task) {
    BOOL old;

    ASSERTMSGLINE(592, __DSP_init_flag == 1, "DSPCancelTask(): DSP driver not initialized!\n");

    old = OSDisableInterrupts();

    task->flags |= 2;

    OSRestoreInterrupts(old);
    return task;
}

DSPTaskInfo *DSPAssertTask(DSPTaskInfo *task) {
    BOOL old;

    ASSERTMSGLINE(623, __DSP_init_flag == 1, "DSPAssertTask(): DSP driver not initialized!\n");
    ASSERTMSGLINE(624, task->flags & 1,
                  "DSPAssertTask(): Specified task not in active task list!\n");

    old = OSDisableInterrupts();

    if (__DSP_curr_task == task) {
        __DSP_rude_task = task;
        __DSP_rude_task_pending = 1;
        OSRestoreInterrupts(old);
        return task;
    }

    if (task->priority < __DSP_curr_task->priority) {
        __DSP_rude_task = task;
        __DSP_rude_task_pending = 1;
        if (__DSP_curr_task->state == 1) {
            DSPAssertInt();
        }
        OSRestoreInterrupts(old);
        return task;
    }

    OSRestoreInterrupts(old);
    return NULL;
}
