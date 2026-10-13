#ifndef RW_RTDICT_H
#define RW_RTDICT_H

#include "rw/rwcore.h"

/* RenderWare dictionaries: named entries of one kind (a schema), kept in dictionaries that are read
 * and written as stream chunks. The type and field names are inferred from the schemas' data and
 * the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

/* The callbacks a schema has for its entries (inferred from the functions it is given) */
typedef void (*RtDictEntryAddRefCallBack)(void *entry);
typedef RwBool (*RtDictEntryDestroyCallBack)(void *entry);
typedef RwChar *(*RtDictEntryGetNameCallBack)(void *entry);
typedef RwUInt32 (*RtDictEntryStreamGetSizeCallBack)(void *entry);
typedef void *(*RtDictEntryStreamReadCallBack)(RwStream *stream);
typedef RwBool (*RtDictEntryStreamWriteCallBack)(void *entry, RwStream *stream);

/* A kind of dictionary entry (0x34 bytes) */
typedef struct RtDictSchema {
    /* 0x00 */ const RwChar *name;
    /* 0x04 */ RwUInt32 dictChunkType;
    /* 0x08 */ RwUInt32 entryChunkType;
    /* 0x0C */ RwUInt32 compatibilityVersion; /* the oldest stream version read */
    /* 0x10 */ void *field_0x10;
    /* 0x14 */ void *current; /* the current dictionary (fn_804790C4) */
    /* 0x18 */ RtDictEntryAddRefCallBack addRefCB;
    /* 0x1C */ RtDictEntryDestroyCallBack destroyCB;
    /* 0x20 */ RtDictEntryGetNameCallBack getNameCB;
    /* 0x24 */ RtDictEntryStreamGetSizeCallBack streamGetSizeCB;
    /* 0x28 */ RtDictEntryStreamReadCallBack streamReadCB;
    /* 0x2C */ void *field_0x2C;
    /* 0x30 */ RtDictEntryStreamWriteCallBack streamWriteCB;
} RtDictSchema;

/* dictionaries (inferred) */
RtDictSchema *fn_80479040(RtDictSchema *schema);   /* init */
RwBool fn_8047909C(RtDictSchema *schema);          /* destruct */
void *fn_804790C4(RtDictSchema *schema);           /* the current dictionary */
void *fn_804790D4(void *dict, const RwChar *name); /* the entry of that name, or NULL */

#ifdef __cplusplus
}
#endif

#endif
