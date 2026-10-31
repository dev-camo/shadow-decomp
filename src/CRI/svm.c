/* CRI SVM server manager (svm.c, SVM/GC Ver.1.58): the dispatch layer under every CRI
 * module. It holds up to 6 named server callbacks per server type, the lock/unlock hooks
 * and the error callback that every module's error message ends in.
 *
 * Names, types and layout follow the RE4 and Mortal Kombat: Deception GameCube decompilations.
 */
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "CRI/svm.h"
#include "layout_check.h"

#define SVM_MAX_SVTYPE 8
#define SVM_MAX_SVR_FUNC 6
#define SVM_ERR_MSG_LEN 128

typedef struct {
    Sint32 (*func)(void *obj); /* 0x00 */
    void *obj;                 /* 0x04 */
    const Char8 *name;         /* 0x08 */
} SvmSvrFunc;
CHECK_SIZEOF(SvmSvrFunc, 0xC);

typedef struct {
    void (*func)(void *obj); /* 0x00 */
    void *obj;               /* 0x04 */
} SvmVoidFunc;
CHECK_SIZEOF(SvmVoidFunc, 0x8);

typedef struct {
    void (*func)(void *obj, Char8 *msg); /* 0x00 */
    void *obj;                           /* 0x04 */
} SvmErrFunc;
CHECK_SIZEOF(SvmErrFunc, 0x8);

/* Version string, kept by its pointer in .rodata. */
const Char8 *const svm_build = "\nSVM/GC Ver.1.58 Build:Feb 28 2005 15:48:25\n";

/* The three counters are volatile: in the original each store is followed by a reload, which
 * MWCC emits only for a volatile object. Declared plain (measured per function against the
 * original): svm_init_level makes SVM_Finish 48 words instead of 50 (40 aligned words differ);
 * svm_lock_level makes svm_unlock, SVM_Unlock, SVM_TestAndSet, SVM_SetCbSvr, SVM_SetCbSvrId,
 * SVM_DelCbSvr and SVM_SetCbBdr 2 words shorter each (23 to 27 aligned words differ);
 * svm_locking_type makes the same seven functions 1 word shorter each (20 to 24 aligned words
 * differ). */
volatile Sint32 svm_init_level = 0;
volatile Sint32 svm_lock_level = 0;
volatile Sint32 svm_locking_type = 0;

SvmVoidFunc svm_post_waitv_func;
SvmVoidFunc svm_pre_waitv_func;
Sint32 (*svm_tas_fptr)(Sint32 *flag);
Char8 svmerr_msg[SVM_ERR_MSG_LEN];
SvmErrFunc svm_err_func;
SvmVoidFunc svm_unlock_func;
SvmVoidFunc svm_lock_func;
SvmVoidFunc svm_goto_border_func[SVM_MAX_SVTYPE];
SvmSvrFunc svm_svr_ftbl[SVM_MAX_SVTYPE][SVM_MAX_SVR_FUNC];
Sint32 svm_svr_exec_flag[SVM_MAX_SVTYPE];
Sint32 svm_exec_cnt[SVM_MAX_SVTYPE];

/* Matching workaround: stand-ins (svm_BssOrderHooks, svm_BssOrderTables, svm_RodataOrderExecId
 * and svm_RodataOrderSpace below). Several functions of this file are stripped from the game, so
 * their bodies are unknown. MWCC lays out .bss by first reference and .rodata literals by first
 * use, so these never-called functions refer to what the stripped ones left in the object, at
 * their positions: a 32-byte static buffer (invented storage, sized to fill the .bss gap before
 * the vsync-wait hooks), the hooks, and three literals. Revisit when the stripped functions' real
 * bodies, or another cause of this layout, are found.
 */

/* Matching workaround: stand-in for the stripped functions before svm_call_err1. The body is not
 * the original's. It refers to the static buffer at .bss 0x0C, then the vsync-wait hooks and the
 * test-and-set hook; storing the addresses keeps the references. Without it the .bss objects land
 * in the first-reference order of the surviving functions and the code that pools their addresses
 * changes (measured by compiling without it). Revisit when the stripped code is known. */
static void svm_BssOrderHooks(void **out) {
    static Char8 buf[32];

    out[0] = buf;
    out[1] = &svm_post_waitv_func;
    out[2] = &svm_pre_waitv_func;
    out[3] = &svm_tas_fptr;
}

/* Delivers a fixed message to the error callback. */
static void svm_call_err1(Char8 *msg) {
    strncpy(svmerr_msg, msg, SVM_ERR_MSG_LEN - 1);
    if (svm_err_func.func != 0) {
        svm_err_func.func(svm_err_func.obj, svmerr_msg);
    }
}

/* Leaves the lock through the unlock callback, checking the nesting type. */
static void svm_unlock(void) {
    if (svm_unlock_func.func != 0) {
        svm_lock_level--;
        if (svm_lock_level == 0) {
            if (svm_locking_type != 1) {
                SVM_CallErr(
                    "2103102:SVM:svm_unlock:lock type miss match.(type org=%d, type now=%d)",
                    (int)svm_locking_type, 1);
            }
            svm_locking_type = 0;
        }
        svm_unlock_func.func(svm_unlock_func.obj);
    }
}

/* Enters the lock through the lock callback (nesting counted). */
static void svm_lock(void) {
    if (svm_lock_func.func != 0) {
        svm_lock_func.func(svm_lock_func.obj);
        if (svm_lock_level == 0) {
            svm_locking_type = 1;
        }
        svm_lock_level++;
    }
}

/* Matching workaround: stand-in for the stripped functions before svm_exec_svr. The body is not
 * the original's. It refers to the border hooks before the code that follows does; storing the
 * addresses keeps the references. Without it the .bss objects land in the first-reference order of
 * the surviving functions and the code that pools their addresses changes (measured by compiling
 * without it). Revisit when the stripped code is known. */
static void svm_BssOrderTables(void **out) {
    out[0] = svm_goto_border_func;
}

/* Runs the callbacks of one server type in order and counts the pass. */
static Sint32 svm_exec_svr(Sint32 svtype) {
    SvmSvrFunc *p;
    Sint32 (*func)(void *obj);
    void *obj;
    Sint32 i;
    Sint32 ret;

    ret = 0;
    p = svm_svr_ftbl[svtype];
    for (i = 0; i < SVM_MAX_SVR_FUNC; i++) {
        func = p->func;
        obj = p->obj;
        if (func != 0) {
            svm_svr_exec_flag[svtype] = 1;
            ret |= func(obj);
            svm_svr_exec_flag[svtype] = 0;
        }
        p++;
    }
    svm_exec_cnt[svtype]++;
    return ret;
}

/* Atomically sets *flag to 1; nonzero if it was not already 1. */
Sint32 SVM_TestAndSet(Sint32 *flag) {
    Sint32 ret;
    Sint32 old;

    if (svm_tas_fptr != 0) {
        ret = svm_tas_fptr(flag);
    } else {
        svm_lock();
        old = *flag;
        *flag = 1;
        ret = (old != 1);
        svm_unlock();
    }
    return ret;
}

/* Resets the tables, hooks and pass counters. */
static void svm_clear(void) {
    Sint32 i;
    Sint32 *p;

    memset(svm_svr_exec_flag, 0, sizeof(svm_svr_exec_flag));
    memset(&svm_lock_func, 0, sizeof(svm_lock_func));
    memset(&svm_unlock_func, 0, sizeof(svm_unlock_func));
    memset(&svm_pre_waitv_func, 0, sizeof(svm_pre_waitv_func));
    memset(&svm_post_waitv_func, 0, sizeof(svm_post_waitv_func));
    /* The original clears only the first 6 (SVM_MAX_SVR_FUNC, the per-type callback count) of the
     * SVM_MAX_SVTYPE (8) execution counters; the stores in SVM_Init show exactly six. */
    p = svm_exec_cnt;
    for (i = 0; i < SVM_MAX_SVR_FUNC; i++) {
        *p++ = 0;
    }
    svm_tas_fptr = 0;
}

/* Reference-counted shutdown; clears the error callback on the last release. */
void SVM_Finish(void) {
    svm_init_level--;
    if (svm_init_level == 0) {
        svm_clear();
        memset(&svm_err_func, 0, sizeof(svm_err_func));
    }
}

/* Reference-counted init. */
void SVM_Init(void) {
    if (svm_init_level == 0) {
        svm_clear();
    }
    svm_init_level++;
}

Sint32 SVM_ExecSvrMwIdle(void) {
    return svm_exec_svr(SVM_SVTYPE_MWIDLE);
}

Sint32 SVM_ExecSvrMain(void) {
    return svm_exec_svr(SVM_SVTYPE_MAIN);
}

Sint32 SVM_ExecSvrFs(void) {
    return svm_exec_svr(SVM_SVTYPE_FS);
}

Sint32 SVM_ExecSvrVsync(void) {
    return svm_exec_svr(SVM_SVTYPE_VSYNC);
}

void SVM_SetCbUnlock(void (*func)(void *obj), void *obj) {
    svm_unlock_func.func = func;
    svm_unlock_func.obj = obj;
}

void SVM_SetCbLock(void (*func)(void *obj), void *obj) {
    svm_lock_func.func = func;
    svm_lock_func.obj = obj;
}

/* Calls the registered border hook of a server type. */
void SVM_GotoSvrBorder(Sint32 svtype) {
    if (svm_goto_border_func[svtype].func != 0) {
        svm_goto_border_func[svtype].func(svm_goto_border_func[svtype].obj);
    }
}

/* Installs the border hook of a server type under the lock. */
void SVM_SetCbBdr(Sint32 svtype, void (*func)(void *obj), void *obj) {
    svm_lock();
    svm_goto_border_func[svtype].func = func;
    svm_goto_border_func[svtype].obj = obj;
    svm_unlock();
}

/* Matching workaround: stand-in for the stripped functions before SVM_SetCbSvrId. The body is not
 * the original's. The stripped functions' parameter messages name SVM_ExecSvrFuncId, which has no
 * code in this unit, and no surviving function uses them; storing the addresses keeps the
 * references. Without it these literals are missing from .rodata (measured by compiling without
 * it). Revisit when the stripped code is known. */
static void svm_RodataOrderExecId(const Char8 **out) {
    out[0] = "1071301:SVM_ExecSvrFuncId:illegal id";
    out[1] = "1071302:SVM_ExecSvrFuncId:illegal svtype";
}

/* Registers a named callback in a fixed slot of a server type. */
void SVM_SetCbSvrId(Sint32 svtype, Sint32 id, Sint32 (*func)(void *obj), void *obj,
                    const Char8 *name) {
    SvmSvrFunc *p;

    svm_lock();
    if (id < 0 || id >= SVM_MAX_SVR_FUNC) {
        svm_call_err1("1071201:SVM_SetCbSvrId:illegal id");
    } else if (svtype < 0 || svtype >= SVM_MAX_SVTYPE) {
        svm_call_err1("1071202:SVM_SetCbSvrId:illegal svtype");
    } else {
        p = &svm_svr_ftbl[svtype][id];
        if (p->func != 0) {
            svm_call_err1("2100801:SVM_SetCbSvrId:over write callback function.");
        }
        p->func = func;
        p->obj = obj;
        if (name != 0) {
            p->name = name;
        } else {
            p->name = "Unknown";
        }
    }
    svm_unlock();
}

/* Clears one callback slot. */
void SVM_DelCbSvr(Sint32 svtype, Sint32 id) {
    svm_lock();
    if (id < 0 || id >= SVM_MAX_SVR_FUNC) {
        svm_call_err1("1051002:SVM_DelCbSvr:illegal id");
    } else if (svtype < 0 || svtype >= SVM_MAX_SVTYPE) {
        /* the message text names SVM_SetCbSvrId, as in the original string */
        svm_call_err1("1071206:SVM_SetCbSvrId:illegal svtype");
    } else {
        svm_svr_ftbl[svtype][id].func = 0;
        svm_svr_ftbl[svtype][id].obj = 0;
        svm_svr_ftbl[svtype][id].name = 0;
    }
    svm_unlock();
}

/* Registers a named callback in the first free slot; returns the slot id or -1. */
Sint32 SVM_SetCbSvr(Sint32 svtype, Sint32 (*func)(void *obj), void *obj, const Char8 *name) {
    Sint32 i;
    SvmSvrFunc *p;

    svm_lock();
    if (svtype < 0 || svtype >= SVM_MAX_SVTYPE) {
        /* the message text names SVM_SetCbSvrId, as in the original string */
        svm_call_err1("1071205:SVM_SetCbSvrId:illegal svtype");
        i = -1;
    } else {
        p = svm_svr_ftbl[svtype];
        for (i = 0; i < SVM_MAX_SVR_FUNC; i++) {
            if (p->func == 0) {
                p->func = func;
                p->obj = obj;
                if (name != 0) {
                    p->name = name;
                } else {
                    p->name = "Unknown";
                }
                break;
            }
            p++;
        }
        if (i == SVM_MAX_SVR_FUNC) {
            svm_call_err1("1051001:SVM_SetCbSvr:too many server function");
            i = -1;
        }
    }
    svm_unlock();
    return i;
}

/* Public wrapper of svm_call_err1. */
void SVM_CallErr1(Char8 *msg) {
    svm_call_err1(msg);
}

/* printf-style error: formats into svmerr_msg and calls the error callback. */
void SVM_CallErr(Char8 *fmt, ...) {
    va_list ap;

    memset(svmerr_msg, 0, sizeof(svmerr_msg));
    va_start(ap, fmt);
    vsprintf(svmerr_msg, fmt, ap);
    va_end(ap);
    if (svm_err_func.func != 0) {
        svm_err_func.func(svm_err_func.obj, svmerr_msg);
    }
}

/* Matching workaround: stand-in for the stripped function before SVM_Unlock. The body is not the
 * original's. No surviving function uses its " " literal, which stays last in .rodata; storing the
 * address keeps the reference. Without it the literal is missing from .rodata (measured by
 * compiling without it). Revisit when the stripped code is known. */
static void svm_RodataOrderSpace(const Char8 **out) {
    out[0] = " ";
}

void SVM_Unlock(void) {
    svm_unlock();
}

void SVM_Lock(void) {
    svm_lock();
}
