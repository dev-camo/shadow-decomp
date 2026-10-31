/* CRI universal stream joint (sj_uni.c); only Init and Finish survive linking. */

#include <string.h>

#include "CRI/sj.h"

Sint32 sjuni_init_cnt;

/* The object is 0xC04 bytes in the original (its symbol extent); the init and finish functions
 * clear the first 0xC00. */
Uint8 sjuni_obj[0xC00 + 4];

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
    if (sjuni_init_cnt == 0) {
        memset(&sjuni_obj, 0, 0xC00);
    }
    sjuni_init_cnt++;
    SJCRS_Unlock();
}
