#include "Game/fn_803C82_pool_types.h"

/* Chosen side-effect interfaces; original full types/arity/returns unknown. */
extern void fn_803C8A74(void *entry);
extern void LSC_LockCrs(void *context);
extern void LSC_UnlockCrs(void *context);

void fn_803C8228(void) {
    /* Chosen opaque stack word; original context type/extent unknown. */
    unsigned int context;
    int index;
    LSC_LockCrs(&context);
    for (index = 0; index < 32; index++) {
        unsigned char *entry = &lbl_805B65B4[index * 0x238];
        if ((int)entry[0] == 1) {
            fn_803C8A74(entry);
        }
    }
    LSC_UnlockCrs(&context);
}
