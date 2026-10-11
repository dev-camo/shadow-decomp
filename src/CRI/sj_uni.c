/* CRI universal stream joint (sj_uni.c); only Init and Finish survive linking. */

#include "Game/fn_803C8_record_types.h"

/* Inferred raw counter view; the original C type is unknown.
 * An explicit zero definition preserves MWCC's BSS symbol order.
 */
unsigned int sjuni_init_cnt = 0;

/* Observed clearing covers 0xC00 bytes. Preserve the four remaining
 * allocation bytes without asserting original pool or slot types.
 */
unsigned char sjuni_obj[0xC00 + 4];

extern void *memset(void *destination, int value, unsigned int size);
/* Chosen side-effect projections; original full arity/returns unknown. */
void SJCRS_Finish(void);
void SJCRS_Init(void);

/* Inferred side-effect interfaces; original full arity/returns unknown. */
void SJUNI_Finish(void) {
    SJCRS_Lock();
    if (--sjuni_init_cnt == 0) {
        memset(&sjuni_obj, 0, 0xC00);
    }
    SJCRS_Unlock();
    SJCRS_Finish();
}

void SJUNI_Init(void) {
    SJCRS_Init();
    SJCRS_Lock();
    if ((int)sjuni_init_cnt == 0) {
        memset(&sjuni_obj, 0, 0xC00);
    }
    sjuni_init_cnt++;
    SJCRS_Unlock();
}
