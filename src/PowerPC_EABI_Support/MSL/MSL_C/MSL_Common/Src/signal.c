#include "signal.h"
#include "abort_exit.h"
#include "critical_regions.h"

#define SIGNAL_NUM 6

__signal_func_ptr signal_funcs[SIGNAL_NUM];

int raise(int sig) {
    __signal_func_ptr signal_func;

    if (sig < 1 || sig > SIGNAL_NUM) {
        return -1;
    }

    __begin_critical_region(stderr_access);
    signal_func = signal_funcs[sig - 1];

    if (signal_func != SIG_IGN) {
        signal_funcs[sig - 1] = SIG_DFL;
    }

    __end_critical_region(stderr_access);

    if (signal_func == SIG_IGN || (signal_func == SIG_DFL && sig == 1)) {
        return 0;
    }

    if (signal_func == SIG_DFL) {
        exit(0);
    }

    (*signal_func)(sig);
    return 0;
}
