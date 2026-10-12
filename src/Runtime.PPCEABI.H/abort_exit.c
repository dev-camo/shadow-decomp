/* MSL runtime exit and abort (abort_exit.c). The two functions are in exit, abort order in the
 * executable. The exit-handler state they share belongs to this file in MSL, but its storage is
 * not split out yet, so it is declared here. */
#include "abort_exit.h"
#include "critical_regions.h"
#include "NMWException.h"
#include "signal.h"

typedef void (*ExitCallback)(void);

extern int __aborting;
extern int __atexit_curr_func;
extern ExitCallback __atexit_funcs[];
extern ExitCallback __console_exit;
extern ExitCallback _dtors[];

/* Defined by the Dolphin SDK's __ppc_eabi_init.c, which has no public header. */
extern void _ExitProcess(void);

void exit(int status) {
    ExitCallback *dtor;

    if (__aborting == 0) {
        __begin_critical_region(atexit_funcs_access);
        __end_critical_region(atexit_funcs_access);
        __destroy_global_chain();

        for (dtor = _dtors; *dtor != 0; dtor++) {
            (*dtor)();
        }

        if (__stdio_exit != 0) {
            __stdio_exit();
            __stdio_exit = 0;
        }
    }

    __begin_critical_region(atexit_funcs_access);
    while (__atexit_curr_func > 0) {
        __atexit_funcs[--__atexit_curr_func]();
    }
    __end_critical_region(atexit_funcs_access);
    __kill_critical_regions();

    if (__console_exit != 0) {
        __console_exit();
        __console_exit = 0;
    }

    _ExitProcess();
}

void abort(void) {
    raise(1);
    __aborting = 1;
    __begin_critical_region(atexit_funcs_access);

    while (__atexit_curr_func > 0) {
        __atexit_funcs[--__atexit_curr_func]();
    }

    __end_critical_region(atexit_funcs_access);
    __kill_critical_regions();
    if (__console_exit != 0) {
        __console_exit();
        __console_exit = 0;
    }
    _ExitProcess();
}
