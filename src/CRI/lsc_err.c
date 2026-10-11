/* CRI LSC error reporting (lsc_err.c). */

#include "mwcc_varargs.h"

/* Inferred callback signature; its return value is unused. */
typedef void (*DiagnosticCallback)(void *context, const char *text);

/* Inferred storage view from this helper and LSC_EntryErrFunc. */
typedef struct {
    /* 0x00 */ DiagnosticCallback callback;
    /* 0x04 */ void *context;
    /* 0x08 */ char text[0x100];
} DiagnosticState;

/* Incomplete storage views preserve the target's absolute-address loads and stores. */
extern DiagnosticCallback lbl_805B6498[];
extern void *lbl_805B649C[];

/* Inferred variadic diagnostic signature; callers do not consume a result. */
void LSC_CallErrFunc(const char *format, ...) {
    MwccVaList args;
    DiagnosticCallback callback;
    DiagnosticState *state = (DiagnosticState *)lbl_805B6498;

    /* Use the compiler intrinsic to save registers and initialize the varargs cursor. */
    __builtin_va_info(args);
    vsprintf(state->text, format, args);
    callback = state->callback;
    if (callback != 0) {
        callback(state->context, state->text);
    }
}

/* Inferred setter signature; callers do not consume a result. */
void LSC_EntryErrFunc(DiagnosticCallback callback, void *context) {
    if (callback == 0) {
        lbl_805B6498[0] = 0;
        lbl_805B649C[0] = 0;
    } else {
        lbl_805B6498[0] = callback;
        lbl_805B649C[0] = context;
    }
}
