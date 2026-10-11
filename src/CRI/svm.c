/* CRI SVM server manager (svm.c, SVM/GC Ver.1.58): the dispatch layer under every CRI
 * module. It holds up to 6 named server callbacks per server type, the lock/unlock hooks
 * and the error callback that every module's error message ends in.
 *
 * Names, types and layout follow SVM/GC Ver.1.54 in the RE4 and Mortal Kombat: Deception
 * GameCube decompilations (same error codes and function order). Ver.1.58 adds a name to
 * each server callback and validates the server type in the registration functions.
 */
#include "msl_string.h"
#include "mwcc_varargs.h"

extern void *memset(void *destination, int value, unsigned int size);
extern unsigned int strlen(const char *text);
extern char *strncat(char *destination, const char *source, unsigned int count);

/* CRI basic types (cri_xpt.h); 32-bit integers are long. */
typedef signed long Sint32;
typedef unsigned long Uint32;
typedef char Char8;

#define SVM_MAX_SVTYPE 8
#define SVM_MAX_SVR_FUNC 6
#define SVM_ERR_MSG_LEN 128

#define SVM_SVTYPE_VSYNC 2
#define SVM_SVTYPE_FS 4
#define SVM_SVTYPE_MAIN 5
#define SVM_SVTYPE_MWIDLE 6

typedef struct {
    Sint32 (*func)(void *obj);
    void *obj;
    const Char8 *name;
} SvmSvrFunc;

typedef struct {
    void (*func)(void *obj);
    void *obj;
} SvmVoidFunc;

typedef struct {
    void (*func)(void *obj, Char8 *msg);
    void *obj;
} SvmErrFunc;

void SVM_CallErr(Char8 *fmt, ...);

/* Version string, kept by its pointer in .rodata. */
const Char8 *const svm_build = "\nSVM/GC Ver.1.58 Build:Feb 28 2005 15:48:25\n";

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

/* The unreferenced functions below are stripped by the linker. MWCC places .bss objects in
 * order of first reference, and these are the first to touch the itoa buffer, the hooks
 * and the server tables, which gives the target's .bss order.
 */

/* Decimal digits of val into str (at most len - 1 characters). */
static void svm_itoa(Sint32 val, Char8 *str, Sint32 len) {
    static Char8 buf[32];
    Sint32 i;
    Sint32 n;
    Sint32 l;

    for (i = 0; i < 32; i++) {
        buf[i] = val % 10;
        val /= 10;
        if (val == 0) {
            buf[i] = '\0';
            break;
        }
    }
    l = strlen(buf);
    n = len - 1;
    if (l < n) {
        n = l;
    }
    for (i = 0; i < n; i++) {
        str[i] = buf[n - 1 - i];
    }
    str[i] = '\0';
}

/* Pre/post vsync-wait hooks of the threaded server. */
void SVM_SetCbWaitVsync(void (*pre)(void *obj), void *pre_obj, void (*post)(void *obj),
                        void *post_obj) {
    svm_post_waitv_func.func = post;
    svm_post_waitv_func.obj = post_obj;
    svm_pre_waitv_func.func = pre;
    svm_pre_waitv_func.obj = pre_obj;
}

/* Installs a platform test-and-set for SVM_TestAndSet. */
void SVM_SetCbTestAndSet(Sint32 (*func)(Sint32 *flag)) {
    svm_tas_fptr = func;
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
                    svm_locking_type, 1);
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

/* Installs the per-server-type "go to border" hook. */
void SVM_SetCbGotoSvrBorder(Sint32 svtype, void (*func)(void *obj), void *obj) {
    svm_goto_border_func[svtype].func = func;
    svm_goto_border_func[svtype].obj = obj;
}

/* Registered callbacks of one server type. */
Sint32 SVM_GetNumCbSvr(Sint32 svtype) {
    Sint32 i;
    Sint32 n = 0;

    for (i = 0; i < SVM_MAX_SVR_FUNC; i++) {
        if (svm_svr_ftbl[svtype][i].func != 0) {
            n++;
        }
    }
    return n;
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

/* Installs the error callback (unreferenced in this game, so stripped). */
void SVM_SetCbErr(void (*func)(void *obj, Char8 *msg), void *obj) {
    svm_lock();
    svm_err_func.func = func;
    svm_err_func.obj = obj;
    svm_unlock();
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

/* Runs one callback of a server type by slot id. */
Sint32 SVM_ExecSvrFuncId(Sint32 svtype, Sint32 id) {
    Sint32 ret = 0;
    SvmSvrFunc *p;

    if (id < 0 || id >= SVM_MAX_SVR_FUNC) {
        svm_call_err1("1071301:SVM_ExecSvrFuncId:illegal id");
        return 0;
    }
    if (svtype < 0 || svtype >= SVM_MAX_SVTYPE) {
        svm_call_err1("1071302:SVM_ExecSvrFuncId:illegal svtype");
        return 0;
    }
    p = &svm_svr_ftbl[svtype][id];
    if (p->func != 0) {
        svm_svr_exec_flag[svtype] = 1;
        ret = p->func(p->obj);
        svm_svr_exec_flag[svtype] = 0;
    }
    svm_exec_cnt[svtype]++;
    return ret;
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
    MwccVaList ap;

    memset(svmerr_msg, 0, sizeof(svmerr_msg));
    __builtin_va_info(ap);
    vsprintf(svmerr_msg, fmt, ap);
    if (svm_err_func.func != 0) {
        svm_err_func.func(svm_err_func.obj, svmerr_msg);
    }
}

/* "a b" decimal formatter. */
void SVM_ItoA2(Sint32 a, Sint32 b, Char8 *str, Sint32 len) {
    svm_itoa(a, str, len);
    strncat(str, " ", len - strlen(str) - 1);
    svm_itoa(b, str + strlen(str), len - strlen(str));
}

void SVM_Unlock(void) {
    svm_unlock();
}

void SVM_Lock(void) {
    svm_lock();
}
