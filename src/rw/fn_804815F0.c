/* RenderWare plugin registries: plugins' data in objects of a type, its construction, destruction
 * and copying, and its stream chunks. The original file name is unknown. */
#include "rw/rwcore.h"

/* errors (inferred) */
#define E_RW_PLUGININIT 0x80000017 /* plugins cannot be registered now */

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

RwFreeList lbl_805E2AD0; /* the free list of registry entries */

RwInt32 lbl_805EDEC0 = 64; /* registry entries per free list block */
RwInt32 lbl_805EDEC4 = 1;  /* free list blocks to preallocate */

RwFreeList *lbl_805F1590; /* the free list of registry entries, while open */
RwUInt32 lbl_805F158C;    /* the registries in lbl_805F1588 */

/* With a replaced free list allocator, the registries plugins were added to, so that closing can
 * empty them */
RwPluginRegistry **lbl_805F1588;

/* The registry entry of a plugin, or NULL */
static inline RwPluginRegEntry *PluginRegistryFindEntry(const RwPluginRegistry *registry,
                                                        RwUInt32 pluginID) {
    RwPluginRegEntry *entry = registry->firstRegEntry;

    while (entry != NULL) {
        if (entry->pluginID == pluginID) {
            break;
        }
        entry = entry->nextRegEntry;
    }
    return entry;
}

/* Sets a plugin's stream chunk callbacks; returns the plugin's offset, or -1 when it is not
 * registered */
RwInt32 fn_804815F0(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkReadCallBack read, RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    RwPluginRegEntry *entry = PluginRegistryFindEntry(registry, pluginID);

    if (entry != NULL) {
        entry->readCB = read;
        entry->writeCB = write;
        entry->getSizeCB = getSize;
        return entry->offset;
    }
    return -1;
}

/* Sets the callback a plugin gets after an object's plugin chunks are read */
RwInt32 fn_80481634(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkAlwaysCallBack callBack) {
    RwPluginRegEntry *entry = PluginRegistryFindEntry(registry, pluginID);

    if (entry != NULL) {
        entry->alwaysCB = callBack;
        return entry->offset;
    }
    return -1;
}

/* Sets a plugin's rights callback */
RwInt32 fn_80481670(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkRightsCallBack callBack) {
    RwPluginRegEntry *entry = PluginRegistryFindEntry(registry, pluginID);

    if (entry != NULL) {
        entry->rightsCB = callBack;
        return entry->offset;
    }
    return -1;
}

/* Reads an object's extension chunk: each plugin's chunk goes to its read callback, or is
 * skipped; then each plugin's always callback is called */
RwPluginRegistry *fn_804816AC(RwPluginRegistry *registry, RwStream *stream, void *object) {
    RwUInt32 length;
    RwUInt32 version;

    if (!fn_8047CF50(stream, rwID_EXTENSION, &length, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        RwPluginRegEntry *entry;

        while (length > 0) {
            RwUInt32 type;
            RwUInt32 size;

            if (!fn_8047CDB4(stream, &type, &size, NULL, NULL)) {
                return NULL;
            }
            entry = registry->firstRegEntry;
            while (entry != NULL) {
                if (entry->pluginID == type) {
                    break;
                }
                entry = entry->nextRegEntry;
            }
            if (entry != NULL && entry->readCB != NULL) {
                if (!entry->readCB(stream, size, object, entry->offset, entry->size)) {
                    return NULL;
                }
            } else if (!fn_804810FC(stream, size)) {
                return NULL;
            }
            length -= size + rwCHUNKHEADERSIZE;
        }

        for (entry = registry->firstRegEntry; entry != NULL; entry = entry->nextRegEntry) {
            if (entry->alwaysCB != NULL && !entry->alwaysCB(object, entry->offset, entry->size)) {
                return NULL;
            }
        }
        return registry;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}

RwPluginRegistry *fn_80481870(const RwPluginRegistry *registry, RwUInt32 pluginID, void *object,
                              RwUInt32 extraData) {
    RwPluginRegEntry *entry = PluginRegistryFindEntry(registry, pluginID);

    if (entry != NULL && entry->rightsCB != NULL) {
        if (entry->rightsCB(object, entry->offset, entry->size, extraData)) {
            return (RwPluginRegistry *)registry;
        }
        return NULL;
    }
    return NULL;
}

/* The size of an object's plugin chunks, with their headers */
RwInt32 fn_804818FC(const RwPluginRegistry *registry, const void *object) {
    RwInt32 size = 0;
    RwPluginRegEntry *entry;

    for (entry = registry->firstRegEntry; entry != NULL; entry = entry->nextRegEntry) {
        if (entry->getSizeCB != NULL) {
            RwInt32 plugSize = entry->getSizeCB(object, entry->offset, entry->size);

            if (plugSize > 0) {
                size += plugSize + rwCHUNKHEADERSIZE;
            }
        }
    }
    return size;
}

/* Writes an object's extension chunk with each plugin's chunk */
const RwPluginRegistry *fn_80481980(const RwPluginRegistry *registry, RwStream *stream,
                                    const void *object) {
    RwPluginRegEntry *entry;

    if (!fn_8047CEF0(stream, rwID_EXTENSION, fn_804818FC(registry, object), RW_STREAM_VERSION,
                     RW_STREAM_BUILD)) {
        return NULL;
    }
    for (entry = registry->firstRegEntry; entry != NULL; entry = entry->nextRegEntry) {
        if (entry->getSizeCB != NULL && entry->writeCB != NULL) {
            RwInt32 size = entry->getSizeCB(object, entry->offset, entry->size);

            if (size > 0) {
                if (!fn_8047CEF0(stream, entry->pluginID, size, RW_STREAM_VERSION,
                                 RW_STREAM_BUILD)) {
                    return NULL;
                }
                if (!entry->writeCB(stream, size, object, entry->offset, entry->size)) {
                    return NULL;
                }
            }
        }
    }
    return registry;
}

/* Skips an object's extension chunk */
const RwPluginRegistry *fn_80481AD4(const RwPluginRegistry *registry, RwStream *stream) {
    RwUInt32 length;

    if (!fn_8047CF50(stream, rwID_EXTENSION, &length, NULL)) {
        return NULL;
    }
    while (length > 0) {
        RwUInt32 size;

        if (!fn_8047CDB4(stream, NULL, &size, NULL, NULL)) {
            return NULL;
        }
        if (!fn_804810FC(stream, size)) {
            return NULL;
        }
        length -= size + rwCHUNKHEADERSIZE;
    }
    return registry;
}

RwBool fn_80481B98(void) {
    lbl_805F1590 = fn_8047FC04(sizeof(RwPluginRegEntry), lbl_805EDEC0, 4, lbl_805EDEC4,
                               &lbl_805E2AD0, rwMEMHINTDUR_GLOBAL);
    if (lbl_805F1590 == NULL) {
        return FALSE;
    }
    lbl_805F158C = 0;
    return TRUE;
}

/* Frees a registry entry, after emptying its registry */
void fn_80481BF4(void *mem, void *data) {
    RwPluginRegEntry *entry = (RwPluginRegEntry *)mem;
    RwPluginRegistry *registry = entry->parentRegistry;

    if (registry->firstRegEntry != NULL) {
        registry->sizeOfStruct = registry->origSizeOfStruct;
        entry->parentRegistry->firstRegEntry = NULL;
        entry->parentRegistry->lastRegEntry = NULL;
    }
    RwFreeListFree((RwFreeList *)data, entry);
}

/* Frees all registry entries and empties the registries */
RwBool fn_80481C58(void) {
    if (lbl_805F1590 != NULL) {
        fn_804800DC(lbl_805F1590, fn_80481BF4, lbl_805F1590);

        if (lbl_805F15B4->rwfreelistalloc != fn_8047FDB0) {
            RwUInt32 i;

            for (i = 0; i < lbl_805F158C; i++) {
                RwPluginRegistry *registry = NULL;
                RwPluginRegEntry *entry = lbl_805F1588[i]->firstRegEntry;

                if (entry != NULL) {
                    registry = entry->parentRegistry;
                }
                while (entry != NULL) {
                    RwPluginRegEntry *next = entry->nextRegEntry;

                    /* The original passes no free list */
                    RwFreeListFree(NULL, entry);
                    entry = next;
                }
                if (registry != NULL && registry->firstRegEntry != NULL) {
                    registry->sizeOfStruct = registry->origSizeOfStruct;
                    registry->firstRegEntry = NULL;
                    registry->lastRegEntry = NULL;
                }
            }
            if (lbl_805F1588 != NULL) {
                RwFree(lbl_805F1588);
                lbl_805F1588 = NULL;
            }
        }

        fn_8047FCDC(lbl_805F1590);
        lbl_805F1590 = NULL;
    }
    return TRUE;
}

/* The default construct, destruct and copy callbacks, which do nothing */
void *fn_80481D8C(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

void *fn_80481D90(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

void *fn_80481D94(void *dst, const void *src, RwInt32 offset, RwInt32 size) {
    return dst;
}

RwInt32 fn_80481D98(RwPluginRegistry *registry, RwUInt32 pluginID) {
    RwPluginRegEntry *entry;

    for (entry = registry->firstRegEntry; entry != NULL; entry = entry->nextRegEntry) {
        if (entry->pluginID == pluginID) {
            return entry->offset;
        }
    }
    return -1;
}

/* Adds a registry to the registries closing must empty, unless it is there (inferred) */
static inline void PluginRegistryTrack(RwPluginRegistry *registry) {
    RwUInt32 i;

    for (i = 0; i < lbl_805F158C; i++) {
        if (registry == lbl_805F1588[i]) {
            break;
        }
    }
    if (lbl_805F158C == i) {
        RwPluginRegistry **registries =
            RwMalloc((lbl_805F158C + 1) * sizeof(RwPluginRegistry *), rwMEMHINTDUR_GLOBAL);

        i = 0;
        if (lbl_805F1588 != NULL) {
            while (i < lbl_805F158C) {
                registries[i] = lbl_805F1588[i];
                i++;
            }
            RwFree(lbl_805F1588);
            lbl_805F1588 = NULL;
        }
        registries[i] = registry;
        lbl_805F1588 = registries;
        lbl_805F158C++;
    }
}

/* Registers a plugin of size bytes in the registry's objects */
RwInt32 fn_80481DC8(RwPluginRegistry *registry, RwInt32 size, RwUInt32 pluginID,
                    RwPluginObjectConstructor constructor, RwPluginObjectDestructor destructor,
                    RwPluginObjectCopy copy) {
    RwPluginRegEntry *entry;

    if (lbl_805F1590 == NULL) {
        return -1;
    }
    if (fn_80485D64()) {
        RWERROR((E_RW_PLUGININIT));
        return -1;
    }

    /* With a replaced free list allocator, closing must find the registry */
    if (lbl_805F15B4->rwfreelistalloc != fn_8047FDB0) {
        PluginRegistryTrack(registry);
    }

    for (entry = registry->firstRegEntry; entry != NULL; entry = entry->nextRegEntry) {
        if (entry->pluginID == pluginID) {
            RWERROR((E_RW_PLUGININIT));
            return entry->offset;
        }
    }

    {
        RwInt32 newSize = registry->sizeOfStruct + ((size + 3) & ~3);

        if (registry->maxSizeOfStruct != 0 && newSize > registry->maxSizeOfStruct) {
            return -1;
        }
        entry = RwFreeListAlloc(lbl_805F1590, rwMEMHINTDUR_GLOBAL);
        if (entry != NULL) {
            entry->offset = registry->sizeOfStruct;
            registry->sizeOfStruct = newSize;
            entry->size = size;
            entry->pluginID = pluginID;
            entry->readCB = NULL;
            entry->writeCB = NULL;
            entry->getSizeCB = NULL;
            entry->alwaysCB = NULL;
            entry->rightsCB = NULL;
            entry->constructCB = constructor != NULL ? constructor : fn_80481D8C;
            entry->destructCB = destructor != NULL ? destructor : fn_80481D90;
            entry->copyCB = copy != NULL ? copy : fn_80481D94;
            entry->errStrCB = NULL;
            entry->nextRegEntry = NULL;
            entry->prevRegEntry = NULL;
            entry->parentRegistry = registry;
            if (registry->firstRegEntry == NULL) {
                registry->firstRegEntry = entry;
                registry->lastRegEntry = entry;
            } else {
                registry->lastRegEntry->nextRegEntry = entry;
                entry->prevRegEntry = registry->lastRegEntry;
                registry->lastRegEntry = entry;
            }
            return entry->offset;
        }
        return -1;
    }
}

/* Constructs the plugins' data in an object; when one fails, the ones before it are destructed */
RwPluginRegistry *fn_8048209C(const RwPluginRegistry *registry, void *object) {
    RwPluginRegEntry *entry = registry->firstRegEntry;

    while (entry != NULL) {
        if (!entry->constructCB(object, entry->offset, entry->size)) {
            RwPluginRegEntry *constructed;

            for (constructed = entry->prevRegEntry; constructed != NULL;
                 constructed = constructed->prevRegEntry) {
                constructed->destructCB(object, constructed->offset, constructed->size);
            }
            return NULL;
        }
        entry = entry->nextRegEntry;
    }
    return (RwPluginRegistry *)registry;
}

/* Destructs the plugins' data, last registered first */
RwPluginRegistry *fn_80482144(const RwPluginRegistry *registry, void *object) {
    RwPluginRegEntry *entry = registry->lastRegEntry;

    while (entry != NULL) {
        entry->destructCB(object, entry->offset, entry->size);
        entry = entry->prevRegEntry;
    }
    return (RwPluginRegistry *)registry;
}

RwPluginRegistry *fn_804821B0(const RwPluginRegistry *registry, void *dst, const void *src) {
    RwPluginRegEntry *entry = registry->firstRegEntry;

    while (entry != NULL) {
        entry->copyCB(dst, src, entry->offset, entry->size);
        entry = entry->nextRegEntry;
    }
    return (RwPluginRegistry *)registry;
}
