#include "Game/fn_803C8_record_types.h"

/* Inferred raw counter and external byte-span views; original types are unknown. */
extern unsigned int sjmem_init_cnt;
extern unsigned char sjmem_obj[];
extern const char lbl_80517278[];
extern void *memset(void *destination, int value, unsigned int size);
/* Chosen side-effect projections; original full arity/returns unknown. */
void fn_803C8E2C(void);
void fn_803C8E5C(void);

/* Inferred side-effect interfaces; original full arity/returns unknown. */
void SJMEM_Finish(void) {
    SJCRS_Lock();
    if (--sjmem_init_cnt == 0) {
        memset(&sjmem_obj, 0, 0x480);
    }
    SJCRS_Unlock();
    fn_803C8E2C();
}

void SJMEM_Init(void) {
    fn_803C8E5C();
    SJCRS_Lock();
    if ((int)sjmem_init_cnt == 0) {
        memset(&sjmem_obj, 0, 0x480);
    }
    sjmem_init_cnt++;
    SJCRS_Unlock();
}

/* Callback-compatible projection; ignored registers do not establish original types/arity. */
void SJMEM_Error(unsigned int context, int argument) {
    (void)context;
    (void)argument;
    SJERR_CallErr(lbl_80517278);
}
