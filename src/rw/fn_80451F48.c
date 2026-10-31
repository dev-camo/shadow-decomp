/* RenderWare user data plugin (plugin ID 0x11F): the user data arrays of geometries, world
 * sectors, frames, cameras, lights, materials and textures, their streams and accessors, and the
 * attachment of the plugin. The original file name is unknown. */
#include "rw/rpusrdat.h"
#include <string.h>
#include "layout_check.h"

#define rwMEMHINT_USERDATA (rwMEMHINTDUR_EVENT | rpID_USERDATAPLUGIN)
#define rwMEMHINT_STRING (rwMEMHINTDUR_EVENT | rwID_STRING)

/* duplicates a string, or gives NULL for NULL */
#define rwstrdup(result, string)                                                                   \
    do {                                                                                           \
        (result) = NULL;                                                                           \
        if ((string) != NULL) {                                                                    \
            (result) = RwMalloc(rwstrlen(string) + 1, rwMEMHINT_STRING);                           \
            if ((result) != NULL) {                                                                \
                rwstrcpy((result), (string));                                                      \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* The plugin's globals, declared in reverse address order (the compiler places them in .sbss
 * last first): the user data module's registration, then the plugin offsets of the user data list
 * and its stream extension per object type (inferred from the attach order) */
RwModuleInfo lbl_805F1490;
RwInt32 lbl_805F148C;
RwInt32 lbl_805F1488; /* texture */
RwInt32 lbl_805F1484;
RwInt32 lbl_805F1480; /* material */
RwInt32 lbl_805F147C;
RwInt32 lbl_805F1478; /* light */
RwInt32 lbl_805F1474;
RwInt32 lbl_805F1470; /* camera */
RwInt32 lbl_805F146C;
RwInt32 lbl_805F1468; /* frame */
RwInt32 lbl_805F1464;
RwInt32 lbl_805F1460; /* world sector */
RwInt32 lbl_805F145C;
RwInt32 lbl_805F1458; /* geometry */

#define USERDATALIST(object, offset) ((RpUserDataList *)((RwUInt8 *)(object) + (offset)))

/* engine plugin constructor */
void *fn_80451F48(void *object, RwInt32 offset, RwInt32 size) {
    lbl_805F1490.numInstances++;
    return object;
}

/* engine plugin destructor */
void *fn_80451F5C(void *object, RwInt32 offset, RwInt32 size) {
    lbl_805F1490.numInstances--;
    return object;
}

/* Reads a user data array from a stream */
RwStream *fn_80451F70(RpUserDataArray *array, RwStream *stream) {
    RwInt32 length;
    RwChar **strings;
    RwInt32 i;

    if (fn_8047D868(stream, &length, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length > 0) {
        array->name = RwMalloc(length, rwMEMHINT_USERDATA);
        if (array->name == NULL) {
            return NULL;
        }
        if (fn_80480D4C(stream, array->name, length) == 0) {
            return NULL;
        }
    }
    if (fn_8047D868(stream, &array->format, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &array->numElements, sizeof(RwInt32)) == NULL) {
        return NULL;
    }

    switch (array->format) {
    case rpINTUSERDATA:
        array->data = RwMalloc(array->numElements * sizeof(RwInt32), rwMEMHINT_USERDATA);
        if (array->data == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, array->data, array->numElements * sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        break;
    case rpREALUSERDATA:
        array->data = RwMalloc(array->numElements * sizeof(RwReal), rwMEMHINT_USERDATA);
        if (array->data == NULL) {
            return NULL;
        }
        if (fn_8047D784(stream, array->data, array->numElements * sizeof(RwReal)) == NULL) {
            return NULL;
        }
        break;
    case rpSTRINGUSERDATA:
        array->data = RwMalloc(array->numElements * sizeof(RwChar *), rwMEMHINT_USERDATA);
        strings = array->data;
        if (strings == NULL) {
            return NULL;
        }
        for (i = 0; i < array->numElements; i++) {
            if (fn_8047D868(stream, &length, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
            if (length > 0) {
                *strings = RwMalloc(length, rwMEMHINT_USERDATA);
                if (*strings == NULL) {
                    return NULL;
                }
                if (fn_80480D4C(stream, *strings, length) == 0) {
                    return NULL;
                }
            } else {
                *strings = NULL;
            }
            strings++;
        }
        break;
    default:
        return NULL;
    }
    return stream;
}

/* Writes a user data array to a stream */
RwStream *fn_80452228(RpUserDataArray *array, RwStream *stream) {
    RwInt32 length;
    RwChar **strings;
    RwInt32 i;

    length = array->name != NULL ? rwstrlen(array->name) + 1 : 0;
    if (fn_8047D458(stream, &length, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length > 0 && fn_80480ED0(stream, array->name, length) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, &array->format, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, &array->numElements, sizeof(RwInt32)) == NULL) {
        return NULL;
    }

    switch (array->format) {
    case rpINTUSERDATA:
        if (fn_8047D458(stream, array->data, array->numElements * sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        break;
    case rpREALUSERDATA:
        if (fn_8047D350(stream, array->data, array->numElements * sizeof(RwReal)) == NULL) {
            return NULL;
        }
        break;
    case rpSTRINGUSERDATA:
        strings = array->data;
        for (i = 0; i < array->numElements; i++) {
            length = *strings != NULL ? rwstrlen(*strings) + 1 : 0;
            if (fn_8047D458(stream, &length, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
            if (length > 0 && fn_80480ED0(stream, *strings, length) == NULL) {
                return NULL;
            }
            strings++;
        }
        break;
    default:
        return NULL;
    }
    return stream;
}

/* Frees the arrays of a user data list and empties it */
inline void UserDataListDestroy(RpUserDataList *list) {
    RwChar **strings;
    RpUserDataArray *array;
    RwInt32 j;
    RwInt32 i;

    if (list->arrays != NULL) {
        for (i = 0; i < list->numElements; i++) {
            array = &list->arrays[i];
            if (array->name != NULL) {
                RwFree(array->name);
                array->name = NULL;
            }
            if (array->format == rpSTRINGUSERDATA) {
                strings = array->data;
                for (j = 0; j < array->numElements; j++) {
                    if (*strings != NULL) {
                        RwFree(*strings);
                    }
                    strings++;
                }
            }
            if (array->data != NULL) {
                RwFree(array->data);
                array->data = NULL;
            }
        }
        RwFree(list->arrays);
    }
    list->arrays = NULL;
    list->numElements = 0;
}

/* Copies a user data list over another, duplicating its names, data and strings */
void fn_80452434(RpUserDataList *dst, const RpUserDataList *src) {
    const RpUserDataArray *srcArray;
    RpUserDataArray *dstArray;
    RwChar **srcStrings;
    RwInt32 dataSize;
    RwInt32 i;
    RwInt32 j;
    RwChar **dstStrings;

    UserDataListDestroy(dst);
    dst->numElements = src->numElements;
    if (dst->numElements <= 0) {
        return;
    }

    dst->arrays = RwMalloc(dst->numElements * sizeof(RpUserDataArray), rwMEMHINT_USERDATA);
    for (i = 0; i < dst->numElements; i++) {
        srcArray = &src->arrays[i];
        dstArray = &dst->arrays[i];
        dstArray->format = srcArray->format;
        dstArray->numElements = srcArray->numElements;
        if (srcArray->name != NULL) {
            rwstrdup(dstArray->name, srcArray->name);
        }
        if (srcArray->data == NULL) {
            continue;
        }

        dataSize = dstArray->numElements * fn_80452C98(dstArray->format);
        dstArray->data = RwMalloc(dataSize, rwMEMHINT_USERDATA);
        if (dstArray->format == rpSTRINGUSERDATA) {
            srcStrings = srcArray->data;
            dstStrings = dstArray->data;
            for (j = 0; j < dstArray->numElements; j++) {
                if (*srcStrings == NULL) {
                    *dstStrings = NULL;
                } else {
                    rwstrdup(*dstStrings, *srcStrings);
                }
                srcStrings++;
                dstStrings++;
            }
        } else {
            memcpy(dstArray->data, srcArray->data, dataSize);
        }
    }
}

/* object plugin constructor: an empty list */
void *fn_80452730(void *object, RwInt32 offset, RwInt32 size) {
    RpUserDataList *list = USERDATALIST(object, offset);

    list->numElements = 0;
    list->arrays = NULL;
    return object;
}

/* object plugin destructor */
void *fn_80452744(void *object, RwInt32 offset, RwInt32 size) {
    RpUserDataList *list = USERDATALIST(object, offset);

    UserDataListDestroy(list);
    return object;
}

/* object plugin copy */
void *fn_80452860(void *dst, const void *src, RwInt32 offset, RwInt32 size) {
    fn_80452434(USERDATALIST(dst, offset), USERDATALIST(src, offset));
    return dst;
}

/* object stream read: the number of arrays, then the arrays */
RwStream *fn_80452898(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RwStream *arrayStream = stream;
    RwInt32 numElements;
    RwInt32 i;
    RpUserDataList *list = USERDATALIST(object, offset);

    if (list != NULL && fn_8047D868(stream, &numElements, sizeof(RwInt32)) != NULL) {
        list->numElements = numElements;
        list->arrays = RwMalloc(list->numElements * sizeof(RpUserDataArray), rwMEMHINT_USERDATA);
        for (i = 0; i < list->numElements; i++) {
            arrayStream = fn_80451F70(&list->arrays[i], arrayStream);
        }
    }
    return stream;
}

/* object stream write: the number of arrays with a format, then those arrays */
RwStream *fn_8045294C(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpUserDataList *list = USERDATALIST(object, offset);
    RwStream *arrayStream = stream;
    RwUInt32 numElements; /* compared unsigned */
    RwInt32 i;

    if (list != NULL && list->numElements > 0) {
        numElements = 0;
        for (i = 0; i < list->numElements; i++) {
            if (list->arrays[i].format != rpNAUSERDATAFORMAT) {
                numElements++;
            }
        }
        if (numElements != 0 &&
            fn_8047D458(stream, (RwInt32 *)&numElements, sizeof(RwInt32)) != NULL) {
            for (i = 0; i < list->numElements; i++) {
                if (list->arrays[i].format != rpNAUSERDATAFORMAT) {
                    arrayStream = fn_80452228(&list->arrays[i], arrayStream);
                }
            }
        }
    }
    return stream;
}

/* The stream size of a user data array */
inline RwInt32 UserDataArrayStreamGetSize(const RpUserDataArray *array) {
    RwChar **strings;
    RwInt32 i;
    RwInt32 size = 0;

    if (array != NULL) {
        /* the name length, then the name with its terminator */
        size = sizeof(RwInt32);
        if (array->name != NULL) {
            size = rwstrlen(array->name) + 1 + sizeof(RwInt32);
        }
        /* the format and the number of elements */
        size += sizeof(RwInt32) + sizeof(RwInt32);

        switch (array->format) {
        case rpINTUSERDATA:
            size += array->numElements * sizeof(RwInt32);
            break;
        case rpREALUSERDATA:
            size += array->numElements * sizeof(RwReal);
            break;
        case rpSTRINGUSERDATA:
            strings = array->data;
            for (i = 0; i < array->numElements; i++) {
                size += sizeof(RwInt32);
                if (*strings != NULL) {
                    size += rwstrlen(*strings) + 1;
                }
                strings++;
            }
            break;
        }
    }
    return size;
}

/* object stream size: the arrays with a format and, if there are any, their count */
RwInt32 fn_80452A40(const void *object, RwInt32 offset, RwInt32 size) {
    const RpUserDataList *list = USERDATALIST(object, offset);
    RwInt32 streamSize = 0;
    RwInt32 i;

    if (list != NULL && list->numElements > 0) {
        for (i = 0; i < list->numElements; i++) {
            if (list->arrays[i].format != rpNAUSERDATAFORMAT) {
                streamSize += UserDataArrayStreamGetSize(&list->arrays[i]);
            }
        }
        if (streamSize > 0) {
            streamSize += sizeof(RwInt32);
        }
    }
    return streamSize;
}

RwInt32 fn_80452B90(RpGeometry *geometry) {
    RwInt32 count = 0;
    RpUserDataList *list = USERDATALIST(geometry, lbl_805F1458);
    RwInt32 i;

    for (i = 0; i < list->numElements; i++) {
        if (list->arrays[i].data != NULL) {
            count++;
        }
    }
    return count;
}

RpUserDataArray *fn_80452BD8(RpGeometry *geometry, RwInt32 index) {
    RpUserDataList *list = USERDATALIST(geometry, lbl_805F1458);

    if (index < list->numElements) {
        return &list->arrays[index];
    }
    return NULL;
}

RwInt32 fn_80452C04(RpWorldSector *sector) {
    RwInt32 count = 0;
    RpUserDataList *list = USERDATALIST(sector, lbl_805F1460);
    RwInt32 i;

    for (i = 0; i < list->numElements; i++) {
        if (list->arrays[i].data != NULL) {
            count++;
        }
    }
    return count;
}

RpUserDataArray *fn_80452C4C(RpWorldSector *sector, RwInt32 index) {
    RpUserDataList *list = USERDATALIST(sector, lbl_805F1460);

    if (index < list->numElements) {
        return &list->arrays[index];
    }
    return NULL;
}

RwChar *fn_80452C78(RpUserDataArray *array) {
    return array->name;
}

RwInt32 fn_80452C80(RpUserDataArray *array) {
    return array->format;
}

RwChar *fn_80452C88(RpUserDataArray *array, RwInt32 index) {
    return ((RwChar **)array->data)[index];
}

RwInt32 fn_80452C98(RwInt32 format) {
    switch (format) {
    case rpINTUSERDATA:
        return sizeof(RwInt32);
    case rpREALUSERDATA:
        return sizeof(RwReal);
    case rpSTRINGUSERDATA:
        return sizeof(RwChar *);
    }
    return 0;
}

RwBool fn_80452CDC(void) {
    if (fn_80485D78(0, rpID_USERDATAPLUGIN, fn_80451F48, fn_80451F5C) < 0) {
        return FALSE;
    }

    lbl_805F1458 = fn_8045994C(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1458 < 0) {
        return FALSE;
    }
    lbl_805F145C = fn_8045998C(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F145C < 0) {
        return FALSE;
    }

    /* the original checks the geometry offsets again here, not the world sector ones */
    lbl_805F1460 = fn_8045F2F8(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1458 < 0) {
        return FALSE;
    }
    lbl_805F1464 = fn_8045F338(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F145C < 0) {
        return FALSE;
    }

    lbl_805F1468 = fn_804878D4(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1468 < 0) {
        return FALSE;
    }
    lbl_805F146C = fn_804836EC(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F146C < 0) {
        return FALSE;
    }

    lbl_805F1470 = fn_804859B8(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1470 < 0) {
        return FALSE;
    }
    lbl_805F1474 = fn_80483554(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F1474 < 0) {
        return FALSE;
    }

    lbl_805F1478 = fn_8045AAA0(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1478 < 0) {
        return FALSE;
    }
    lbl_805F147C = fn_8045AAE0(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F147C < 0) {
        return FALSE;
    }

    lbl_805F1480 = fn_8045B2CC(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1480 < 0) {
        return FALSE;
    }
    lbl_805F1484 = fn_8045B30C(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F1484 < 0) {
        return FALSE;
    }

    lbl_805F1488 = fn_8048D280(sizeof(RpUserDataList), rpID_USERDATAPLUGIN, fn_80452730,
                               fn_80452744, fn_80452860);
    if (lbl_805F1488 < 0) {
        return FALSE;
    }
    lbl_805F148C = fn_80483AFC(rpID_USERDATAPLUGIN, fn_80452898, fn_8045294C, fn_80452A40);
    if (lbl_805F148C < 0) {
        return FALSE;
    }
    return TRUE;
}
