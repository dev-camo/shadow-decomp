#include "Game/fn_803C82_record_inline.h"

extern void *memset(void *destination, int value, unsigned int size);

void LSC_Destroy(Fn803C82RecordView *object) {
    if (object != 0) {
        fn_803C82_reset_record(object);
        object->byte_0 = 0;
        /* Observed clear length; the view does not claim the original allocation type. */
        memset(object, 0, 0x238);
    }
}
