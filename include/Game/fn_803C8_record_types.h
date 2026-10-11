#ifndef FN_803C8_RECORD_TYPES_H
#define FN_803C8_RECORD_TYPES_H

#include "msl_string.h"

/* Inferred callback interface: a raw context word and a signed argument.
 * Original full argument lists, return types and context type are unknown.
 */
typedef void (*Fn803C8RecordCallback)(unsigned int context, int argument);

/* View of a CRI SJMEM stream-joint object.
 * Inferred accessed-field view; original field names and types unknown. */
typedef struct Fn803C8RecordView {
    /* 0x00 */ unsigned char unknown_0[4];
    /* 0x04 */ unsigned int word_4;
    /* 0x08 */ unsigned int word_8;
    /* 0x0C */ unsigned int word_C;
    /* 0x10 */ unsigned int word_10;
    /* 0x14 */ unsigned int word_14;
    /* 0x18 */ unsigned int word_18;
    /* 0x1C */ Fn803C8RecordCallback callback_1C;
    /* 0x20 */ unsigned int argument_20;
} Fn803C8RecordView;

/* Inferred two-word descriptor view; original full type/extent are unknown. */
typedef struct Fn803C8DescriptorView {
    /* 0x00 */ unsigned int word_0;
    /* 0x04 */ int word_4;
} Fn803C8DescriptorView;

#ifdef __cplusplus
extern "C" {
#endif

/* Inferred side-effect interfaces; original full argument lists/returns unknown. */
void SJERR_CallErr(const char *text);
void SJCRS_Lock(void);
void SJCRS_Unlock(void);
extern const char lbl_80517120[];

/* Inferred raw-word reader and four-register query interfaces. */
unsigned int SJMEM_GetBufSize(const Fn803C8RecordView *record);
int SJMEM_IsGetChunk(const Fn803C8RecordView *record, int mode, int requested, int *actual);
/* Inferred update interface from three consumed registers; original return type is unknown. */
void SJMEM_UngetChunk(Fn803C8RecordView *record, int mode, Fn803C8DescriptorView *descriptor);
/* Inferred descriptor-filter interface; original full arity and return type are unknown. */
void SJMEM_PutChunk(const Fn803C8RecordView *record, int mode, Fn803C8DescriptorView *descriptor);

/* Inferred mode interface from four consumed registers; original full arity/return unknown. */
void SJMEM_GetChunk(Fn803C8RecordView *record, int mode, int requested,
                    Fn803C8DescriptorView *descriptor);

/* Inferred raw-word results and accessed-word interfaces; original full types/arity unknown. */
unsigned int SJMEM_GetNumData(const Fn803C8RecordView *record, int mode);
unsigned int SJMEM_GetUuid(const Fn803C8RecordView *record);
/* Inferred side-effect interfaces; original full arity and returns are unknown. */
void SJMEM_Reset(Fn803C8RecordView *record);
void SJMEM_EntryErrFunc(Fn803C8RecordView *record, Fn803C8RecordCallback callback,
                        unsigned int context);
void SJMEM_Destroy(Fn803C8RecordView *record);

#ifdef __cplusplus
}
#endif

#endif
