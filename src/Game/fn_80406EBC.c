#include "CRI/mwstm.h"
#include "CRI/sfd.h"

typedef struct Fn80406EBCObject {
    /* 0x00 */ unsigned char padding[60];
    /* 0x3C */ void *field_3C;
    /* 0x40 */ void *field_40;
} Fn80406EBCObject;

void fn_80406EBC(Fn80406EBCObject *object) {
    void *field_40 = object->field_40;
    void *field_3C = object->field_3C;

    if (field_40 != 0 && MWSTM_GetStat(field_40) == ADXSTM_STAT_END) {
        SFD_TermSupply(field_3C);
    }
}
