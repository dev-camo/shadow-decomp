/* CRI LSC error reporting (lsc_err.c): printf-style messages into a 256-byte buffer, passed
 * to the installed error callback. Names follow the RE4 GameCube decompilation of the same
 * file.
 */

#include "CRI/lsc.h"
#include <stdarg.h>
#include <stdio.h>

static void (*lsc_err_func)(void *obj, Char8 *msg) = 0;
static void *lsc_err_obj = 0;
static Char8 lsc_err_msg[256];

/* Formats and reports a scheduler error. */
void LSC_CallErrFunc(Char8 *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vsprintf(lsc_err_msg, fmt, ap);
    va_end(ap);
    if (lsc_err_func != 0) {
        lsc_err_func(lsc_err_obj, lsc_err_msg);
    }
}

/* Installs (or clears with a null callback) the error callback. */
void LSC_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj) {
    if (func == 0) {
        lsc_err_func = 0;
        lsc_err_obj = 0;
    } else {
        lsc_err_func = func;
        lsc_err_obj = obj;
    }
}
