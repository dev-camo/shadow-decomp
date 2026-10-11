#ifndef FN_803C9_RECORD_TYPES_H
#define FN_803C9_RECORD_TYPES_H

#include "Game/fn_803C8_record_types.h"

/* View of a CRI SJRBF ring-buffer stream-joint object.
 * Inferred accessed-field view; original field names and types unknown.
 * The indexed reader's shifts establish byte strides, not an original C array type.
 */
typedef struct Fn803C9RecordView {
    /* 0x00 */ unsigned char unknown_0[4];
    /* 0x04 */ unsigned int word_4;
    /* 0x08 */ unsigned int word_8;
    /* 0x0C */ unsigned int word_C;
    /* 0x10 */ unsigned int word_10;
    /* 0x14 */ unsigned int word_14;
    /* 0x18 */ unsigned int word_18;
    /* 0x1C */ unsigned int word_1C;
    /* 0x20 */ unsigned int word_20;
    /* 0x24 */ unsigned int word_24;
    /* 0x28 */ unsigned int word_28;
    /* 0x2C */ unsigned int word_2C;
    /* 0x30 */ unsigned int word_30;
    /* 0x34 */ unsigned int word_34;
    /* 0x38 */ Fn803C8RecordCallback callback_38;
    /* 0x3C */ unsigned int argument_3C;
} Fn803C9RecordView;

#ifdef __cplusplus
extern "C" {
#endif
extern const char lbl_80517288[];
/* Body-inferred raw-word/shift interfaces; original full types/arity unknown. */
unsigned int SJRBF_GetFlowCnt(const Fn803C9RecordView *record, unsigned int row,
                              unsigned int column);
unsigned int SJRBF_GetXtrSize(const Fn803C9RecordView *record);
unsigned int SJRBF_GetBufSize(const Fn803C9RecordView *record);
unsigned int SJRBF_GetBufPtr(const Fn803C9RecordView *record);
/* Inferred signed comparison/output interface; original full types/arity unknown. */
int SJRBF_IsGetChunk(const Fn803C9RecordView *record, int mode, int requested, int *actual);
#ifdef __cplusplus
}
#endif
#endif
