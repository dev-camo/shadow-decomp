/* RenderWare dictionary toolkit: dictionary schemas, their current dictionary, finding entries by
 * name, and reading and destroying dictionaries. The original file name is unknown. */
#include "rw/rtdict.h"

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* The toolkit's error plugin ID (inferred) */
#define rwID_DICTTOOLKIT 0x1BF

/* Reports an error of the toolkit; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_DICTTOOLKIT;                                                        \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

RtDictSchema *fn_80479040(RtDictSchema *schema) {
    schema->dictionaries =
        fn_8047DC90(sizeof(RtDict *), rwMEMHINTDUR_GLOBAL | schema->dictChunkType);
    if (schema->dictionaries == NULL) {
        return NULL;
    }
    schema->current = NULL;
    return schema;
}

RwBool fn_8047909C(RtDictSchema *schema) {
    fn_8047DFC8(schema->dictionaries);
    return TRUE;
}

RtDict *fn_804790C4(RtDictSchema *schema) {
    return schema->current;
}

RtDictSchema *fn_804790CC(RtDictSchema *schema, RtDict *dict) {
    schema->current = dict;
    return schema;
}

void *fn_804790D4(RtDict *dict, const RwChar *name) {
    void **slot;
    void *entry;
    RtDictEntryGetNameCallBack getName = dict->schema->getNameCB;
    void **end = fn_8047E0E8(dict->entries);

    for (slot = fn_8047E0E0(dict->entries); slot != end; slot++) {
        entry = *slot;
        if (rwstrcmp(getName(entry), name) == 0) {
            return entry;
        }
    }
    return NULL;
}

/* Adds a dictionary to its schema's list */
static inline RtDictSchema *DictSchemaAddDict(RtDictSchema *schema, RtDict *dict) {
    RtDict **slot = fn_8047DDBC(schema->dictionaries, rwMEMHINTDUR_EVENT | schema->dictChunkType);

    if (slot == NULL) {
        return NULL;
    }
    *slot = dict;
    return schema;
}

/* Removes a dictionary from its schema's list, moving the last one into its place */
static inline RtDictSchema *DictSchemaRemoveDict(RtDictSchema *schema, RtDict *dict) {
    RtDict **slot = fn_8047E0E0(schema->dictionaries);
    RtDict **end = fn_8047E0E8(schema->dictionaries);

    for (; slot != end; slot++) {
        if (dict == *slot) {
            *slot = *(RtDict **)fn_8047E0CC(schema->dictionaries,
                                            fn_8047E03C(schema->dictionaries) - 1);
            fn_8047DF9C(schema->dictionaries, 1);
            break;
        }
    }
    return schema;
}

/* Sets up a new dictionary of a schema and adds it to the schema's list */
static inline RtDict *DictInit(RtDict *dict, RtDictSchema *schema) {
    dict->entries = fn_8047DC90(sizeof(void *), rwMEMHINTDUR_EVENT | schema->dictChunkType);
    if (dict->entries == NULL) {
        return NULL;
    }
    dict->schema = schema;
    if (DictSchemaAddDict(schema, dict) == NULL) {
        fn_8047DFC8(dict->entries);
        return NULL;
    }
    return dict;
}

static inline RtDict *DictSchemaCreateDict(RtDictSchema *schema) {
    RtDict *dict = RwMalloc(sizeof(RtDict), rwMEMHINTDUR_EVENT | schema->dictChunkType);

    if (dict == NULL) {
        RWERROR((E_RW_NOMEM, sizeof(RtDict)));
        return NULL;
    }
    if (DictInit(dict, schema) == NULL) {
        RwFree(dict);
        return NULL;
    }
    return dict;
}

/* Adds an entry to a dictionary, which takes a reference to it */
static inline RtDict *DictAddEntry(RtDict *dict, void *entry) {
    *(void **)fn_8047DDBC(dict->entries, rwMEMHINTDUR_EVENT | dict->schema->dictChunkType) = entry;
    dict->schema->addRefCB(entry);
    return dict;
}

/* Destroys a dictionary, releasing its entries */
RwBool fn_80479174(RtDict *dict) {
    void **end;
    void **entry;

    if (dict == dict->schema->current) {
        dict->schema->current = NULL;
    }
    DictSchemaRemoveDict(dict->schema, dict);
    end = fn_8047E0E8(dict->entries);
    entry = fn_8047E0E0(dict->entries);
    while (entry != end) {
        dict->schema->destroyCB(*entry++);
    }
    fn_8047DFC8(dict->entries);
    RwFree(dict);
    return TRUE;
}

/* Reads a dictionary and its entries from a stream, after its chunk header */
RtDict *fn_80479284(RtDictSchema *schema, RwStream *stream) {
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 numEntries;
    RtDict *dict;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version < schema->compatibilityVersion) {
        return schema->streamReadCompatibilityCB(stream);
    }
    if (fn_80480D4C(stream, &numEntries, length) == 0) {
        return NULL;
    }
    fn_8047D254(&numEntries, sizeof(RwUInt32));

    dict = DictSchemaCreateDict(schema);
    if (dict == NULL) {
        return NULL;
    }
    while (numEntries-- != 0) {
        void *entry;

        if (!fn_8047CF50(stream, schema->entryChunkType, &length, &version) ||
            (entry = schema->streamReadCB(stream)) == NULL) {
            fn_80479174(dict);
            return NULL;
        }
        DictAddEntry(dict, entry);
        /* the dictionary now holds the only needed reference */
        schema->destroyCB(entry);
    }
    return dict;
}
