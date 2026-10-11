#include "Game/fn_803C82_record_types.h"

extern const char lbl_80516D18[];
extern const char lbl_80516D44[];
extern void LSC_CallErrFunc(const char *format, ...);

void LSC_SetFlowLimit(Fn803C82RecordView *object, int value) {
    if (object == 0) {
        LSC_CallErrFunc(lbl_80516D18);
    } else if (value < 0 || value > (int)object->word_18) {
        LSC_CallErrFunc(lbl_80516D44, value);
    } else {
        object->word_14 = value;
    }
}
