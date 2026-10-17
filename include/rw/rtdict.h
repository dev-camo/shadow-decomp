#ifndef RW_RTDICT_H
#define RW_RTDICT_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* RenderWare dictionaries: named entries of one kind (a schema), kept in dictionaries that are read
 * and written as stream chunks. The type and field names are inferred from the schemas' data and
 * the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RtDict RtDict;

/* The callbacks a schema has for its entries (inferred from the functions it is given) */
typedef void (*RtDictEntryAddRefCallBack)(void *entry);
typedef RwBool (*RtDictEntryDestroyCallBack)(void *entry);
typedef RwChar *(*RtDictEntryGetNameCallBack)(void *entry);
typedef RwUInt32 (*RtDictEntryStreamGetSizeCallBack)(void *entry);
typedef void *(*RtDictEntryStreamReadCallBack)(RwStream *stream);
typedef RwBool (*RtDictEntryStreamWriteCallBack)(void *entry, RwStream *stream);
/* reads a whole dictionary in a stream format older than the schema's compatibility version */
typedef RtDict *(*RtDictStreamReadCompatibilityCallBack)(RwStream *stream);

/* A kind of dictionary entry (0x34 bytes) */
typedef struct RtDictSchema {
    /* 0x00 */ const RwChar *name;
    /* 0x04 */ RwUInt32 dictChunkType;
    /* 0x08 */ RwUInt32 entryChunkType;
    /* 0x0C */ RwUInt32 compatibilityVersion; /* lower versions: streamReadCompatibilityCB */
    /* 0x10 */ RwSList *dictionaries;         /* of RtDict pointers */
    /* 0x14 */ RtDict *current;               /* the current dictionary (fn_804790C4) */
    /* 0x18 */ RtDictEntryAddRefCallBack addRefCB;
    /* 0x1C */ RtDictEntryDestroyCallBack destroyCB;
    /* 0x20 */ RtDictEntryGetNameCallBack getNameCB;
    /* 0x24 */ RtDictEntryStreamGetSizeCallBack streamGetSizeCB;
    /* 0x28 */ RtDictEntryStreamReadCallBack streamReadCB;
    /* 0x2C */ RtDictStreamReadCompatibilityCallBack streamReadCompatibilityCB;
    /* 0x30 */ RtDictEntryStreamWriteCallBack streamWriteCB;
} RtDictSchema;
CHECK_SIZEOF(RtDictSchema, 0x34);

/* A dictionary: its schema and its entries (8 bytes) */
struct RtDict {
    /* 0x0 */ RtDictSchema *schema;
    /* 0x4 */ RwSList *entries; /* of entry pointers */
};
CHECK_SIZEOF(struct RtDict, 0x8);

/* dictionaries (inferred) */
RtDictSchema *fn_80479040(RtDictSchema *schema);               /* init */
RwBool fn_8047909C(RtDictSchema *schema);                      /* destruct */
RtDict *fn_804790C4(RtDictSchema *schema);                     /* the current dictionary */
RtDictSchema *fn_804790CC(RtDictSchema *schema, RtDict *dict); /* sets the current dictionary */
void *fn_804790D4(RtDict *dict, const RwChar *name);           /* the entry of that name, or NULL */
RwBool fn_80479174(RtDict *dict);                              /* destroys */
RtDict *fn_80479284(RtDictSchema *schema, RwStream *stream);   /* reads */

#ifdef __cplusplus
}
#endif

#endif
