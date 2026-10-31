/* RenderWare lists: fixed-size entries in one array that grows as entries are added. The original
 * file name is unknown. */
#include "rw/rwcore.h"

/* The entries a new list has room for */
#define rwSLISTINITIALENTRIES 20

RwSList *fn_8047DC90(RwInt32 size, RwUInt32 hint) {
    RwSList *sList = RwMalloc(sizeof(RwSList), rwMEMHINTDUR_EVENT);

    if (sList == NULL) {
        RWERRORCORE((E_RW_NOMEM, sizeof(RwSList)));
        return NULL;
    }
    sList->numElementsFilled = 0;
    sList->numElementsAlloced = rwSLISTINITIALENTRIES;
    sList->entrySize = size;
    sList->listElements =
        RwMalloc(sList->numElementsAlloced * sList->entrySize, hint | rwMEMHINTFLAG_RESIZABLE);
    if (sList->listElements == NULL) {
        RWERRORCORE((E_RW_NOMEM, sList->numElementsAlloced * sList->entrySize));
        RwFree(sList);
        return NULL;
    }
    return sList;
}

RwSList *fn_8047DDB0(RwSList *sList) {
    sList->numElementsFilled = 0;
    return sList;
}

/* Adds an entry, growing the array by a quarter when it is full */
void *fn_8047DDBC(RwSList *sList, RwUInt32 hint) {
    void *entry;

    if (sList->numElementsFilled >= sList->numElementsAlloced) {
        RwUInt8 *newElements = RwRealloc(
            sList->listElements,
            sList->entrySize * (sList->numElementsAlloced + sList->numElementsAlloced / 4), hint);

        if (newElements == NULL) {
            RWERRORCORE((E_RW_NOMEM, sList->entrySize * (sList->numElementsAlloced +
                                                         sList->numElementsAlloced / 4)));
            return NULL;
        }
        sList->listElements = newElements;
        sList->numElementsAlloced += sList->numElementsAlloced / 4;
    }
    entry = &sList->listElements[sList->numElementsFilled * sList->entrySize];
    sList->numElementsFilled++;
    return entry;
}

/* Adds entries, growing the array by a quarter more than needed when they do not fit */
void *fn_8047DEA0(RwSList *sList, RwInt32 size, RwUInt32 hint) {
    void *entries;

    if (sList->numElementsFilled + size >= sList->numElementsAlloced) {
        RwUInt8 *newElements = RwRealloc(
            sList->listElements,
            sList->entrySize * (sList->numElementsAlloced + size + sList->numElementsAlloced / 4),
            hint);

        if (newElements == NULL) {
            RWERRORCORE((E_RW_NOMEM, sList->entrySize * (sList->numElementsAlloced + size +
                                                         sList->numElementsAlloced / 4)));
            return NULL;
        }
        sList->listElements = newElements;
        sList->numElementsAlloced =
            sList->numElementsAlloced + size + sList->numElementsAlloced / 4;
    }
    entries = &sList->listElements[sList->numElementsFilled * sList->entrySize];
    sList->numElementsFilled += size;
    return entries;
}

RwSList *fn_8047DF9C(RwSList *sList, RwInt32 count) {
    sList->numElementsFilled -= count;
    return sList;
}

void *fn_8047DFAC(RwSList *sList) {
    if (sList->numElementsFilled != 0) {
        return sList->listElements;
    }
    return NULL;
}

RwBool fn_8047DFC8(RwSList *sList) {
    if (sList->listElements != NULL && sList->numElementsAlloced != 0) {
        RwFree(sList->listElements);
        sList->listElements = NULL;
        sList->numElementsAlloced = 0;
    }
    RwFree(sList);
    return TRUE;
}

RwInt32 fn_8047E03C(RwSList *sList) {
    return sList->numElementsFilled;
}

/* Destroys the list, returning its array if it holds entries (freeing it otherwise) */
void *fn_8047E044(RwSList *sList) {
    void *array = NULL;

    if (sList->numElementsFilled != 0) {
        array = sList->listElements;
    } else if (sList->listElements != NULL) {
        RwFree(sList->listElements);
        sList->listElements = NULL;
        array = NULL;
    }
    RwFree(sList);
    return array;
}

void *fn_8047E0CC(RwSList *sList, RwInt32 index) {
    return &sList->listElements[sList->entrySize * index];
}

void *fn_8047E0E0(RwSList *sList) {
    return sList->listElements;
}

void *fn_8047E0E8(RwSList *sList) {
    return &sList->listElements[sList->entrySize * sList->numElementsFilled];
}
