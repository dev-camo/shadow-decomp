/* MSL runtime abort (abort_exit.c). */

typedef void (*ExitCallback)(void);

extern int __aborting;
extern int __atexit_curr_func;
extern ExitCallback __atexit_funcs[];
extern ExitCallback __console_exit;

extern int raise(int signal);
extern void __begin_critical_region(int region);
extern void __end_critical_region(int region);
extern void __kill_critical_regions(void);
extern void _ExitProcess(void);

void abort(void) {
    raise(1);
    __aborting = 1;
    __begin_critical_region(0);

    while (__atexit_curr_func > 0) {
        __atexit_funcs[--__atexit_curr_func]();
    }

    __end_critical_region(0);
    __kill_critical_regions();
    if (__console_exit != 0) {
        __console_exit();
        __console_exit = 0;
    }
    _ExitProcess();
}
