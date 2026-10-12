/* CRI ADX critical section (adx_crs.c): the ADX layer's lock/unlock mapped onto the SVM lock
 * callbacks, plus the enter/leave hooks around the public ADXF/ADXT entries (empty here).
 *
 * Functions, order and variable names from The World Ends With You's CRI adx_crs.c; RE4's
 * Lock/Unlock bodies are the same. The file is compiled with deferred inlining, so the
 * functions are emitted in reverse order.
 */
#include "CRI/adx.h"
#include "CRI/svm.h"

/* volatile as in The World Ends With You's adx_crs.c (the counter is reloaded after each
 * update). */
volatile Sint32 adxcrs_msk = 0;
volatile Sint32 adxcrs_cnt = 0;

/* Counts an ADX user; the first one clears the interrupt mask. */
void ADXCRS_Init(void) {
    adxcrs_cnt++;
    if (adxcrs_cnt == 1) {
        adxcrs_msk = 0;
    }
}

/* Releases an ADX user; the last one clears the interrupt mask. */
void ADXCRS_Finish(void) {
    adxcrs_cnt--;
    if (adxcrs_cnt == 0) {
        adxcrs_msk = 0;
    }
}

/* Enters the ADX critical section (SVM lock). */
void ADXCRS_Lock(void) {
    SVM_Lock();
}

/* Leaves the ADX critical section (SVM unlock). */
void ADXCRS_Unlock(void) {
    SVM_Unlock();
}

/* Hook before a public entry; empty in this build. */
void ADXCRS_Enter(void) {}

/* Hook after a public entry; empty in this build. */
void ADXCRS_Leave(void) {}
