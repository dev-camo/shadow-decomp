/* RenderWare errors: the pending error kept in the engine globals, setting and taking it, and
 * forming error codes. The original file name is unknown. */
#include "rw/rwcore.h"
#include <stdarg.h>
#include "layout_check.h"

/* The engine globals of the error module */
typedef struct rwErrorGlobals {
    /* 0x0 */ RwError errEr; /* the pending error */
} rwErrorGlobals;
CHECK_SIZEOF(rwErrorGlobals, 0x8);

#define RWERRORGLOBAL(var)                                                                         \
    (RWPLUGINOFFSET(rwErrorGlobals, lbl_805F15B4, lbl_805F1558.globalsOffset)->var)

RwModuleInfo lbl_805F1558; /* the error module */
RwModuleInfo lbl_805F1550; /* a module that only counts its instances (inferred) */

void *fn_8047DB10(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1550.numInstances++;
    return instance;
}

void *fn_8047DB24(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1550.numInstances--;
    return instance;
}

/* Opens the error module with its globals at offset, with no error pending */
void *fn_8047DB38(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1558.globalsOffset = offset;
    lbl_805F1558.numInstances++;
    RWERRORGLOBAL(errEr.pluginID) = 0;
    RWERRORGLOBAL(errEr.errorCode) = E_RW_NOERROR;
    return instance;
}

void *fn_8047DB70(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1558.numInstances--;
    return instance;
}

/* Sets the pending error, unless one is pending already; core errors (with the top bit of their
 * code set) are kept without a plugin */
RwError *fn_8047DB84(const RwError *error) {
    if (RWERRORGLOBAL(errEr.pluginID) == 0 && RWERRORGLOBAL(errEr.errorCode) == E_RW_NOERROR) {
        if (error->errorCode & 0x80000000) {
            RWERRORGLOBAL(errEr.pluginID) = 0;
        } else {
            RWERRORGLOBAL(errEr.pluginID) = error->pluginID;
        }
        RWERRORGLOBAL(errEr.errorCode) = error->errorCode;
    }
    return (RwError *)error;
}

/* Takes the pending error, leaving none pending */
RwError *fn_8047DBE4(RwError *error) {
    *error = RWERRORGLOBAL(errEr);
    RWERRORGLOBAL(errEr.pluginID) = 0;
    RWERRORGLOBAL(errEr.errorCode) = E_RW_NOERROR;
    return error;
}

/* Forms an error code; the arguments after it describe the error and are not kept */
RwInt32 fn_8047DC28(RwInt32 code, ...) {
    va_list args;

    va_start(args, code);
    va_end(args);
    return code;
}
