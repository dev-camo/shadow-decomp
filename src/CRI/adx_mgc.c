/* CRI ADX manager for GameCube (adx_mgc.c, ADXGC Ver.1.30): the ADXM_* front end. It runs the SVM
 * server groups on OS threads: a vsync thread and a file-system thread driven by the retrace, a
 * mwidle thread that runs the SVM mwidle servers and is resumed on every vsync, and a "safe"
 * thread that only spins, counting. ADXM_ExecMain runs the SVM main server from the game's
 * own thread.
 *
 * Structure and the ADXM_* and adxm_* names follow the RE4 GameCube decompilation of the same file
 * (ADXGC Ver.1.21), which creates the same four threads. The thread entry points and the SVM
 * callbacks are static here; their names are inferred from the work they do.
 *
 * Not yet matching: ADXM_SetupThrd differs in 2 words, the order of two constant loads
 * (li 0 and li 1) before the flag stores. */
#include <dolphin/os.h>
#include <dolphin/vi.h>

#include "CRI/adx.h"
#include "CRI/adx_t.h"
#include "CRI/svm.h"
#include "layout_check.h"

#define ADXM_STACK_SIZE 0x2000
#define ADXM_SAFE_STACK_SIZE 0x1000

/* Number of OSResumeThread attempts before the mwidle border is declared unreachable. */
#define ADXM_BORDER_RETRY 200000000

/* Thread priorities: the priority a locked caller (and a mwidle thread at its border) runs at,
 * then the priorities of the safe, vsync, fs and mwidle threads. The field at 0x10 is only stored,
 * never read, by the code in this executable. */
typedef struct {
    Sint32 lock;   /* 0x00 */
    Sint32 safe;   /* 0x04 */
    Sint32 vsync;  /* 0x08 */
    Sint32 fs;     /* 0x0C */
    Sint32 x10;    /* 0x10 */
    Sint32 mwidle; /* 0x14 */
} ADXM_TPRM;
CHECK_SIZEOF(ADXM_TPRM, 0x18);

typedef struct {
    void (*func)(void *obj);
    void *obj;
} ADXM_CB;
CHECK_SIZEOF(ADXM_CB, 8);

/* The build string is read without being used. */
const Char8 *const volatile adxgc_build = "\nADXGC Ver.1.30 Build:Feb 28 2005 15:49:04\n";

void (*adxgc_exec_svr)(void);
static Sint32 adxm_init_level;
/* The lock level and the safe thread's counter and run flag are shared with the other threads.
 * Plain variables compile differently: the spin loop in adxm_exec_safe then keeps the counter
 * in a register and never re-reads the flag, and the unlock callback reuses the decremented
 * value instead of reading it back. */
static volatile Sint32 adxm_lock_level;
static Sint32 adxm_goto_border_flag;
static volatile Sint32 adxm_safe_cnt;
static Sint32 adxm_vsync_cnt;
static Sint32 adxm_fs_cnt;
static Sint32 adxm_mwidle_cnt;
static Sint32 adxm_mwidle_exec_flag;
static Sint32 adxm_x24;
static OSThread *adxm_main_thread;
OSThread adxm_mwidle_thread;
OSThread adxm_vsync_thread;
static OSThread adxm_fs_thread;
OSThread adxm_safe_thread;
static Sint32 adxm_cur_prio;
static Sint32 adxm_set_prio;
Sint32 adxm_fs_end;
static Sint32 adxm_fs_act;
static Sint32 adxm_vsync_end;
static Sint32 adxm_vsync_act;
static Sint32 adxm_mwidle_end;
Sint32 adxm_mwidle_act;
Sint32 adxm_safe_end;
volatile Sint32 adxm_safe_act;
static Uint8 adxm_stack_mwidle[ADXM_STACK_SIZE];
Uint8 adxm_stack_fs[ADXM_STACK_SIZE];
Uint8 adxm_stack_vsync[ADXM_STACK_SIZE];
static Uint8 adxm_stack_safe[ADXM_SAFE_STACK_SIZE];

ADXM_TPRM adxm_save_tprm = {0};
/* Callback run when the mwidle thread suspends itself and after each vsync tick while the mwidle
 * thread is alive (nothing in this executable sets it). */
static ADXM_CB adxm_exec_cb = {0};

static void adxm_exec_mwidle(void);
static void adxm_exec_fs(void);
static void adxm_exec_vsync(void);
static void adxm_exec_safe(void);
static void adxm_border_cb(void *obj);
static void adxm_lock_cb(void *obj);
static void adxm_unlock_cb(void *obj);

/* Matching workaround: order-only stand-in for stripped ADXM code: MWCC lays out .bss in order
 * of first use, and the original layout has these statics (adxgc_exec_svr,
 * adxm_mwidle_exec_flag and adxm_x24 are used nowhere else) first used ahead of ADXM_SetupThrd.
 * Never called; the linker discards it. Revisit when the stripped body is found. */
static void adxm_BssOrder(void) {
    adxgc_exec_svr = NULL;
    adxm_init_level = 0;
    adxm_lock_level = 0;
    adxm_goto_border_flag = 0;
    adxm_safe_cnt = 0;
    adxm_vsync_cnt = 0;
    adxm_fs_cnt = 0;
    adxm_mwidle_cnt = 0;
    adxm_mwidle_exec_flag = 0;
    adxm_x24 = 0;
    adxm_main_thread = NULL;
    adxm_mwidle_thread.state = 0;
    adxm_vsync_thread.state = 0;
    adxm_fs_thread.state = 0;
    adxm_safe_thread.state = 0;
    adxm_cur_prio = 0;
    adxm_set_prio = 0;
    adxm_fs_end = 0;
    adxm_fs_act = 0;
    adxm_vsync_end = 0;
    adxm_vsync_act = 0;
    adxm_mwidle_end = 0;
    adxm_mwidle_act = 0;
    adxm_safe_end = 0;
    adxm_safe_act = 0;
    adxm_stack_mwidle[0] = 0;
    adxm_stack_fs[0] = 0;
    adxm_stack_vsync[0] = 0;
    adxm_stack_safe[0] = 0;
}

/* Starts the server threads once (every call counts a level): the saved priorities come from tprm
 * (six entries) or the defaults, the vsync, fs and mwidle threads are resumed and
 * the SVM lock, unlock and border callbacks are installed. */
void ADXM_SetupThrd(ADXM_TPRM *tprm) {
    adxgc_build;
    if (adxm_init_level == 0) {
        SVM_Init();
        SVM_SetCbLock(adxm_lock_cb, NULL);
        SVM_SetCbUnlock(adxm_unlock_cb, NULL);
        if (tprm == NULL) {
            adxm_save_tprm.x10 = 16;
            adxm_save_tprm.lock = 1;
            adxm_save_tprm.safe = 8;
            adxm_save_tprm.vsync = 12;
            adxm_save_tprm.fs = 14;
            adxm_save_tprm.mwidle = 24;
        } else {
            adxm_save_tprm = *tprm;
        }
        OSCreateThread(&adxm_safe_thread, (void *(*)(void *))adxm_exec_safe, NULL,
                       adxm_stack_safe + ADXM_SAFE_STACK_SIZE, ADXM_SAFE_STACK_SIZE,
                       adxm_save_tprm.safe, OS_THREAD_ATTR_DETACH);
        OSCreateThread(&adxm_vsync_thread, (void *(*)(void *))adxm_exec_vsync, NULL,
                       adxm_stack_vsync + ADXM_STACK_SIZE, ADXM_STACK_SIZE, adxm_save_tprm.vsync,
                       OS_THREAD_ATTR_DETACH);
        OSCreateThread(&adxm_fs_thread, (void *(*)(void *))adxm_exec_fs, NULL,
                       adxm_stack_fs + ADXM_STACK_SIZE, ADXM_STACK_SIZE, adxm_save_tprm.fs,
                       OS_THREAD_ATTR_DETACH);
        OSCreateThread(&adxm_mwidle_thread, (void *(*)(void *))adxm_exec_mwidle, NULL,
                       adxm_stack_mwidle + ADXM_STACK_SIZE, ADXM_STACK_SIZE, adxm_save_tprm.mwidle,
                       OS_THREAD_ATTR_DETACH);
        adxm_main_thread = OSGetCurrentThread();
        adxm_fs_act = 1;
        adxm_mwidle_act = 1;
        adxm_vsync_act = 1;
        adxm_safe_act = 1;
        adxm_fs_end = 0;
        adxm_mwidle_end = 0;
        adxm_vsync_end = 0;
        adxm_safe_end = 0;
        adxm_set_prio = 0;
        OSResumeThread(&adxm_vsync_thread);
        OSResumeThread(&adxm_fs_thread);
        OSResumeThread(&adxm_mwidle_thread);
        SVM_SetCbBdr(SVM_SVTYPE_MWIDLE, adxm_border_cb, NULL);
    }
    adxm_init_level++;
}

/* mwidle thread: runs the SVM mwidle servers and, once they have nothing left to do (or a border
 * was requested), calls the executed-servers callback and suspends itself until the next vsync. */
static void adxm_exec_mwidle(void) {
    while (adxm_mwidle_act == 1) {
        adxm_mwidle_cnt++;
        if (SVM_ExecSvrMwIdle() == 0 || adxm_goto_border_flag == 1) {
            if (adxm_goto_border_flag == 1) {
                adxm_goto_border_flag = 0;
                OSSetThreadPriority(&adxm_mwidle_thread, adxm_save_tprm.mwidle);
            }
            if (adxm_exec_cb.func != NULL) {
                adxm_exec_cb.func(adxm_exec_cb.obj);
            }
            OSSuspendThread(&adxm_mwidle_thread);
        }
    }
    adxm_mwidle_end = 1;
}

/* fs thread: one SVM file-system server pass per retrace. */
static void adxm_exec_fs(void) {
    while (adxm_fs_act == 1) {
        VIWaitForRetrace();
        adxm_fs_cnt++;
        SVM_ExecSvrFs();
    }
    adxm_fs_end = 1;
}

/* vsync thread: counts the retrace for ADXT and the SVM vsync servers, then wakes the mwidle thread
 * and calls the executed-servers callback. */
static void adxm_exec_vsync(void) {
    while (adxm_vsync_act == 1) {
        VIWaitForRetrace();
        adxm_vsync_cnt++;
        adxt_vsync_cnt++;
        SVM_ExecSvrVsync();
        if (adxm_mwidle_end == 0) {
            OSResumeThread(&adxm_mwidle_thread);
            if (adxm_exec_cb.func != NULL) {
                adxm_exec_cb.func(adxm_exec_cb.obj);
            }
        }
    }
    adxm_vsync_end = 1;
}

/* safe thread: spins, counting, at the safe priority (8 by default). */
static void adxm_exec_safe(void) {
    while (adxm_safe_act == 1) {
        adxm_safe_cnt++;
    }
    adxm_safe_end = 1;
}

/* SVM border callback: raises the mwidle thread to the lock priority and resumes it until it has
 * passed its border (it clears the flag), then restores its priority. */
static void adxm_border_cb(void *obj) {
    Sint32 i;

    if (adxm_mwidle_end == 1) {
        return;
    }
    adxm_goto_border_flag = 1;
    OSSetThreadPriority(&adxm_mwidle_thread, adxm_save_tprm.lock);
    for (i = 0; i < ADXM_BORDER_RETRY; i++) {
        OSResumeThread(&adxm_mwidle_thread);
        if (adxm_goto_border_flag == 0) {
            break;
        }
    }
    if (i == ADXM_BORDER_RETRY) {
        SVM_CallErr1("1060102: Internal Error: adxm_goto_mwidle_border");
    }
    OSSetThreadPriority(&adxm_mwidle_thread, adxm_save_tprm.mwidle);
}

/* SVM unlock callback: the last unlock suspends the safe thread and restores the caller's
 * priority. */
static void adxm_unlock_cb(void *obj) {
    OSThread *thread;

    adxm_lock_level--;
    if (adxm_lock_level == 0) {
        thread = OSGetCurrentThread();
        OSSuspendThread(&adxm_safe_thread);
        OSSetThreadPriority(thread, adxm_cur_prio);
    }
}

/* SVM lock callback: the first lock raises the caller to the lock priority (with interrupts and
 * the scheduler off, so the priority is saved atomically) and resumes the safe thread. */
static void adxm_lock_cb(void *obj) {
    BOOL enabled;
    OSThread *thread;
    Sint32 prio;

    if (adxm_lock_level == 0) {
        enabled = OSDisableInterrupts();
        OSDisableScheduler();
        adxm_set_prio = 1;
        thread = OSGetCurrentThread();
        prio = OSGetThreadPriority(thread);
        OSSetThreadPriority(thread, adxm_save_tprm.lock);
        adxm_cur_prio = prio;
        adxm_set_prio = 0;
        OSEnableScheduler();
        OSRestoreInterrupts(enabled);
        OSResumeThread(&adxm_safe_thread);
    }
    adxm_lock_level++;
}

/* Runs the SVM main server group: the ADX talk, file and scheduler servers and the Sofdec
 * player. */
void ADXM_ExecMain(void) {
    SVM_ExecSvrMain();
}

/* Waits for the next retrace. */
void ADXM_WaitVsync(void) {
    VIWaitForRetrace();
}
