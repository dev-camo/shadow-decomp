#include "Game/fn_803C8_record_types.h"

/* Inferred raw counter and external byte-span views; original types are unknown. */
extern unsigned int sjuni_init_cnt;
extern unsigned char sjuni_obj[];
extern void *memset(void *destination, int value, unsigned int size);
/* Chosen side-effect projections; original full arity/returns unknown. */
void fn_803C8E2C(void);
void fn_803C8E5C(void);

/* Inferred side-effect interfaces; original full arity/returns unknown. */
void SJUNI_Finish(void) {
    SJCRS_Lock();
    if (--sjuni_init_cnt == 0) {
        memset(&sjuni_obj, 0, 0xC00);
    }
    SJCRS_Unlock();
    fn_803C8E2C();
}

void SJUNI_Init(void) {
    fn_803C8E5C();
    SJCRS_Lock();
    if ((int)sjuni_init_cnt == 0) {
        memset(&sjuni_obj, 0, 0xC00);
    }
    sjuni_init_cnt++;
    SJCRS_Unlock();
}
