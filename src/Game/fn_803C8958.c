#include "Game/fn_803C82_pool_types.h"
#include "Game/fn_803C82_record_types.h"

/* Chosen consumed-register interfaces; original full types/arity/returns unknown. */
extern void LSC_LockCrs(void *context);
extern void LSC_UnlockCrs(void *context);
extern void LSC_EntryErrFunc(unsigned int first_word, unsigned int second_word);
extern void *memset(void *destination, int value, unsigned int size);

void fn_803C8958(void) {
    /* Chosen opaque stack word; original context type/extent unknown. */
    unsigned int context;
    LSC_LockCrs(&context);
    if (--lbl_805B65B0 == 0) {
        int index = 0;
        for (; index < 32; index++) {
            unsigned char *entry = &lbl_805B65B4[index * 0x238];
            if ((int)entry[0] == 1) {
                LSC_Destroy((Fn803C82RecordView *)entry);
            }
        }
        memset(lbl_805B65B4, 0, 0x4700);
        LSC_EntryErrFunc(0, 0);
    }
    LSC_UnlockCrs(&context);
}
