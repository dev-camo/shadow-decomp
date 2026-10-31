/* CRI Sofdec MW player server-manager glue (mwsfdsvm.c): registers the player's servers with the
 * SVM (main, idle and vsync types), removes them again, and routes the player's error messages
 * (MWSFSVM_Error) to the SVM error callback.
 *
 * Names and structure follow the RE4 GameCube decompilation of the same file. This version has an
 * extra name argument on every registration and an unregistration function for each server type
 * and for the SVM itself.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "CRI/mwsfdsvm.h"
#include "CRI/svm.h"

/* ids of the registered servers */
Sint32 mwg_main_fid;
Sint32 mwg_idle_fid;
Sint32 mwg_vsync_fid;
Sint32 mwg_vbin_fid;

/* printf-style error of the MW player: formats into a 256-byte static and reports it through
 * SVM_CallErr1. */
void MWSFSVM_Error(const Char8 *fmt, ...) {
    static Char8 errstr[256];
    va_list ap;

    memset(errstr, 0, sizeof(errstr));
    va_start(ap, fmt);
    vsprintf(errstr, fmt, ap);
    va_end(ap);
    SVM_CallErr1(errstr);
}

/* Re-entry guard for the server flags. */
Sint32 MWSFSVM_TestAndSet(Sint32 *flag) {
    return SVM_TestAndSet(flag);
}

/* Unregisters the main-type server (inferred from the registration it undoes). */
void fn_804031D4(void) {
    SVM_DelCbSvr(SVM_SVTYPE_MAIN, mwg_main_fid);
}

/* Registers the player's server as an SVM main server. */
void MWSFSVM_EntryMainFunc(Sint32 (*func)(void *obj), void *obj, const Char8 *name) {
    mwg_main_fid = SVM_SetCbSvr(SVM_SVTYPE_MAIN, func, obj, name);
}

/* Unregisters the idle-type server (inferred from the registration it undoes). */
void fn_80403244(void) {
    SVM_DelCbSvr(SVM_SVTYPE_MWIDLE, mwg_idle_fid);
}

/* Registers the idle-thread server (SVM mwidle type). */
void MWSFSVM_EntryIdleFunc(Sint32 (*func)(void *obj), void *obj, const Char8 *name) {
    mwg_idle_fid = SVM_SetCbSvr(SVM_SVTYPE_MWIDLE, func, obj, name);
}

/* Unregisters the vsync-type server (inferred from the registration it undoes). */
void fn_804032B4(void) {
    SVM_DelCbSvr(SVM_SVTYPE_VSYNC, mwg_vsync_fid);
}

/* Registers the vsync server in the fixed slot `id`. */
void MWSFSVM_EntryIdVfunc(Sint32 id, Sint32 (*func)(void *obj), void *obj, const Char8 *name) {
    SVM_SetCbSvrId(SVM_SVTYPE_VSYNC, id, func, obj, name);
    mwg_vsync_fid = id;
}

/* Shuts the SVM down (inferred from SVM_Finish being the counterpart of SVM_Init). */
void fn_80403334(void) {
    SVM_Finish();
}

/* SVM init and cleared server ids. */
void MWSFSVM_Init(void) {
    SVM_Init();
    mwg_vbin_fid = 0;
    mwg_vsync_fid = 0;
    mwg_idle_fid = 0;
    mwg_main_fid = 0;
}
