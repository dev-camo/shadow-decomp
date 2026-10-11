/* Inferred raw 32-bit SDK call interfaces; the restore result is unused. */
extern unsigned int OSDisableInterrupts(void);
extern void OSRestoreInterrupts(unsigned int state);

/* Inferred raw storage views; volatile preserves the observed count reload.
 * The signed zero-test view selects MWCC cmpwi without asserting original signedness.
 */
volatile unsigned int sjcrs_lvl;
unsigned int sjcrs_msk;

/* Inferred side-effect interfaces; original argument lists/return types unknown. */
void SJCRS_Unlock(void) {
    sjcrs_lvl--;
    if ((int)sjcrs_lvl == 0) {
        OSRestoreInterrupts(sjcrs_msk);
    }
}

void SJCRS_Lock(void) {
    if ((int)sjcrs_lvl == 0) {
        sjcrs_msk = OSDisableInterrupts();
    }
    sjcrs_lvl++;
}
