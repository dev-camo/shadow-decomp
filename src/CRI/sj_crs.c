/* CRI stream-joint critical section (sj_crs.c): a nesting interrupt lock and the SJ library
 * init count. Names follow the Xenoblade CRI sj_crs.c, whose bodies match this file.
 */

/* CRI basic types (cri_xpt.h); 32-bit integers are long. */
typedef signed long Sint32;

extern Sint32 OSDisableInterrupts(void);
extern Sint32 OSRestoreInterrupts(Sint32 level);

/* volatile: the nesting level is reloaded after each update. */
volatile Sint32 sjcrs_lvl;
Sint32 sjcrs_msk;
static volatile Sint32 sj_init_count;

void SJCRS_Unlock(void) {
    sjcrs_lvl--;
    if (sjcrs_lvl == 0) {
        OSRestoreInterrupts(sjcrs_msk);
    }
}

void SJCRS_Lock(void) {
    if (sjcrs_lvl == 0) {
        sjcrs_msk = OSDisableInterrupts();
    }
    sjcrs_lvl++;
}

/* Releases one SJ library user; the last one resets the lock nesting. */
void SJCRS_Finish(void) {
    sj_init_count--;
    if (sj_init_count == 0) {
        sjcrs_lvl = 0;
    }
}

/* Registers one SJ library user; the first one resets the lock nesting. */
void SJCRS_Init(void) {
    sj_init_count++;
    if (sj_init_count == 1) {
        sjcrs_lvl = 0;
    }
}
