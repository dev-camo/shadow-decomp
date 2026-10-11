/* CRI universal stream joint (sj_uni.c); only Init and Finish survive linking. */

#include "CRI/sj.h"

/* Inferred raw counter view; the original C type is unknown.
 * An explicit zero definition preserves MWCC's BSS symbol order.
 */
unsigned int sjuni_init_cnt = 0;

/* Observed clearing covers 0xC00 bytes. Preserve the four remaining
 * allocation bytes without asserting original pool or slot types.
 */
unsigned char sjuni_obj[0xC00 + 4];

extern void *memset(void *destination, int value, unsigned int size);

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
