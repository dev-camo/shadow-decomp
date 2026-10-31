#include <dolphin/dsp.h>

#include "__dsp.h"

// The SDK ships this as an empty function; the calls in dsp.c and dsp_task.c remain.
void __DSP_debug_printf(const char *fmt, ...) {}

DSPTaskInfo *__DSPGetCurrentTask(void) {
    return __DSP_curr_task;
}
