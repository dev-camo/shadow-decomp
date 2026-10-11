#include "Game/fn_803C81_query_types.h"
extern const char lbl_80516ECC[];
extern const char lbl_80516EF8[];
extern void LSC_CallErrFunc(const char *format, ...);

unsigned int LSC_GetNumStm(const Fn803C81QueryView *object) {
    if (object == 0) {
        LSC_CallErrFunc(lbl_80516ECC);
        return (unsigned int)-1;
    } else {
        return object->word_24;
    }
}
int LSC_GetStat(const Fn803C81QueryView *object) {
    if (object == 0) {
        LSC_CallErrFunc(lbl_80516EF8);
        return -1;
    } else {
        return object->byte_1;
    }
}
