/* RenderWare resources: instanced data the engine caches for objects, allocated from a heap and
 * freed least recently used first when the heap is full. The original file name is unknown. */
#include "rw/rwcore.h"

/* The resources module's ID (inferred) */
#define rwID_RESOURCESMODULE 0x40B

/* A resource entry could not be allocated (inferred); reported with the size asked for */
#define E_RW_RESOURCEENTRYALLOC 0xC

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

RwModuleInfo lbl_805F1578;

/* Initializes the module's globals at offset in the engine (inferred): creates the heap, unless
 * its size is 0, and empties the lists */
static inline RwResourcesGlobals *ResourcesInit(RwInt32 offset) {
    RwUInt32 heapSize = lbl_805F15B4->resArenaInitSize;
    RwResourcesGlobals *globals = RWPLUGINOFFSET(RwResourcesGlobals, lbl_805F15B4, offset);

    if (heapSize != 0) {
        globals->memHeap = RwMalloc(heapSize, rwMEMHINTDUR_GLOBAL | rwID_RESOURCESMODULE);
        if (globals->memHeap == NULL) {
            RWERROR((E_RW_NOMEM, heapSize));
            return NULL;
        }
        if (!fn_80482984(globals->memHeap, heapSize)) {
            RwFree(globals->memHeap);
            RWERROR((E_RW_RESOURCEENTRYALLOC, 0));
            return NULL;
        }
    } else {
        globals->memHeap = NULL;
    }

    rwLinkListInitialize(&globals->entriesA);
    rwLinkListInitialize(&globals->entriesB);
    globals->usedEntries = &globals->entriesA.link;
    globals->freeEntries = &globals->entriesB.link;
    globals->maxSize = heapSize;
    globals->currentSize = 0;
    globals->reusageSize = 0;
    return globals;
}

/* Opens the resources module: its heap, of the engine's resources size, and its lists */
void *fn_8048053C(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1578.globalsOffset = offset;
    if (ResourcesInit(offset) == NULL) {
        return NULL;
    }
    lbl_805F1578.numInstances++;
    return instance;
}

/* Closes the resources module: frees all entries and the heap */
void *fn_80480690(void *instance, RwInt32 offset, RwInt32 size) {
    fn_80480B08();
    if (RWRESOURCESGLOBAL(memHeap) != NULL) {
        fn_804829D4(RWRESOURCESGLOBAL(memHeap));
        RwFree(RWRESOURCESGLOBAL(memHeap));
        RWRESOURCESGLOBAL(memHeap) = NULL;
    }
    lbl_805F1578.numInstances--;
    return instance;
}

/* Frees an entry, after telling its owner; entries outside the lists are not in the heap */
RwBool fn_8048071C(RwResEntry *entry) {
    if (entry->destroyNotify != NULL) {
        entry->destroyNotify(entry);
    }
    if (entry->ownerRef != NULL) {
        *entry->ownerRef = NULL;
    }
    if (entry->link.next != NULL) {
        rwLinkListRemoveLLLink(&entry->link);
        RWRESOURCESGLOBAL(currentSize) -= entry->size;
        fn_804829DC(entry);
    } else {
        RwFree(entry);
    }
    return TRUE;
}

/* Starts a frame: the entries used last frame join those used before it, and the emptied list
 * collects the entries used this frame */
void fn_804807CC(void) {
    RwResourcesGlobals *globals =
        RWPLUGINOFFSET(RwResourcesGlobals, lbl_805F15B4, lbl_805F1578.globalsOffset);
    RwLLLink *usedEntries;
    RwLLLink *freeEntries;
    RwLLLink *first;

    freeEntries = globals->freeEntries;
    usedEntries = globals->usedEntries;
    first = freeEntries->next;

    if (first != freeEntries) {
        if (usedEntries->next == usedEntries) {
            /* The used list takes all of the free list's entries */
            usedEntries->next = first;
            usedEntries->next->prev = usedEntries;
            usedEntries->prev = freeEntries->prev;
            usedEntries->prev->next = usedEntries;
            freeEntries->next = freeEntries;
            freeEntries->prev = freeEntries;
        } else {
            /* The free list's entries follow the used list's */
            RwLLLink *last = usedEntries->prev;

            last->next = first;
            first->prev = last;
            last = freeEntries->prev;
            last->next = usedEntries;
            usedEntries->prev = last;
            freeEntries->next = freeEntries;
            freeEntries->prev = freeEntries;
        }
    }

    RWRESOURCESGLOBAL(usedEntries) = freeEntries;
    RWRESOURCESGLOBAL(freeEntries) = usedEntries;
    RWRESOURCESGLOBAL(reusageSize) = 0;
}

/* Allocates an entry of size bytes for an owner, freeing the least recently used entries until
 * it fits; ownerRef, when not NULL, is set to the entry and cleared when it is freed */
RwResEntry *fn_80480878(void *owner, RwResEntry **ownerRef, RwInt32 size,
                        RwResEntryDestroyNotify destroyNotify) {
    RwLLLink *oldestUsed;
    RwBool full = FALSE;

    while (!full) {
        RwResEntry *entry = fn_80482A94(RWRESOURCESGLOBAL(memHeap), size + sizeof(RwResEntry));

        if (entry != NULL) {
            entry->link.next = RWRESOURCESGLOBAL(usedEntries)->next;
            entry->link.prev = RWRESOURCESGLOBAL(usedEntries);
            RWRESOURCESGLOBAL(usedEntries)->next->prev = &entry->link;
            RWRESOURCESGLOBAL(usedEntries)->next = &entry->link;
            entry->owner = owner;
            entry->size = size;
            entry->ownerRef = ownerRef;
            entry->destroyNotify = destroyNotify;
            RWRESOURCESGLOBAL(currentSize) += size;
            if (ownerRef != NULL) {
                *ownerRef = entry;
            }
            return entry;
        } else {
            RwResourcesGlobals *globals =
                RWPLUGINOFFSET(RwResourcesGlobals, lbl_805F15B4, lbl_805F1578.globalsOffset);

            if (globals->freeEntries->prev != globals->freeEntries) {
                /* The least recently used entry from before this frame */
                RwResEntry *oldest = (RwResEntry *)globals->freeEntries->prev;

                fn_8048071C(oldest);
            } else {
                /* The least recently used entry from this frame */
                oldestUsed = globals->usedEntries->prev;
                if (oldestUsed != globals->usedEntries) {
                    globals->reusageSize += ((RwResEntry *)oldestUsed)->size;
                    fn_8048071C((RwResEntry *)oldestUsed);
                } else {
                    full = TRUE;
                }
            }
        }
    }

    if (ownerRef != NULL) {
        *ownerRef = NULL;
    }
    RWERROR((E_RW_RESOURCEENTRYALLOC, size));
    return NULL;
}

/* Frees all entries */
RwBool fn_80480B08(void) {
    RwLLLink *cur;
    RwLLLink *end;

    /* Joins the lists, so that one walk reaches all entries */
    RWRESOURCESGLOBAL(entriesA).link.prev->next = RWRESOURCESGLOBAL(entriesB).link.next;
    cur = RWRESOURCESGLOBAL(entriesA).link.next;
    end = &RWRESOURCESGLOBAL(entriesB).link;
    while (cur != end) {
        RwResEntry *entry = (RwResEntry *)cur;

        cur = cur->next;
        fn_8048071C(entry);
    }

    rwLinkListInitialize(&RWRESOURCESGLOBAL(entriesA));
    rwLinkListInitialize(&RWRESOURCESGLOBAL(entriesB));
    RWRESOURCESGLOBAL(reusageSize) = 0;
    return TRUE;
}
