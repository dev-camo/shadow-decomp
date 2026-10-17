#ifndef FN_80406B70_WORK_H
#define FN_80406B70_WORK_H
#include "layout_check.h"

/* Global work block lbl_805DD9CC (0x15B4 bytes); only the fields read or written by
 * fn_80406B70, fn_80406B80 and fn_80407930 are known. */
typedef struct Fn80406B70Work {
    /* 0x00 */ unsigned char padding0[0x38];
    /* 0x38 */ int field_38;
    /* 0x3C */ int field_3C;
    /* 0x40 */ unsigned char padding40[0x18];
    /* 0x58 */ void *field_58;
} Fn80406B70Work;
CHECK_SIZEOF(Fn80406B70Work, 0x5C);

#ifdef __cplusplus
extern "C" {
#endif

extern Fn80406B70Work lbl_805DD9CC;

Fn80406B70Work *fn_80406EB0(void);

#ifdef __cplusplus
}
#endif

#endif
