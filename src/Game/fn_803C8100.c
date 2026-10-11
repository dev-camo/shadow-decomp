#include "CRI/lsc.h"

/* Status callback installed by LSC_EntryStatFunc (unreferenced in this game). */
void (*lsc_stat_func)(void *obj1, void *obj2);
void *lsc_stat_obj1;
void *lsc_stat_obj2;

/* Calls the status callback when a scheduler's queue runs empty. */
void LSC_CallStatFunc(void) {
    if (lsc_stat_func != 0) {
        lsc_stat_func(lsc_stat_obj1, lsc_stat_obj2);
    }
}
