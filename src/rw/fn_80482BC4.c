/* RenderWare strings: the engine's string functions, the free list of short strings, and string
 * stream chunks. The original file name is unknown. */
#include "rw/rwcore.h"
#include <stdio.h>
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* chunk IDs */
#define rwID_STRING 0x2
#define rwID_UNICODESTRING 0x13

/* The string module's ID (inferred) */
#define rwID_STRINGMODULE 0x412

/* The oldest stream version strings are read from */
#define rwSTREAMOLDESTVERSION 0x35000

/* The bytes a string chunk is read in at a time */
#define rwSTRINGREADCHUNK 64
#define rwUNICODESTRINGREADCHUNK 128

/* The size of the strings in the string free list */
#define rwSTRINGFREELISTENTRYSIZE 0x21

/* The core's error plugin ID (inferred) */
#define rwCOREERRORPLUGIN 1

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The engine globals of the string module */
typedef struct rwStringGlobals {
    /* 0x0 */ RwFreeList *stringFreeList;
} rwStringGlobals;

#define RWSTRINGGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwStringGlobals, lbl_805F15B4, lbl_805F15A0.globalsOffset)->var)

RwFreeList lbl_805E2AF8; /* the string free list */

RwInt32 lbl_805EDEC8 = 16; /* strings per free list block */
RwInt32 lbl_805EDECC = 1;  /* free list blocks to preallocate */

RwModuleInfo lbl_805F15A0; /* the string module */

/* The string a NULL string is written as */
static const RwChar lbl_805FB2D0[] = "";

void *fn_80482BC4(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15A0.globalsOffset = offset;
    RWSTRINGGLOBAL(stringFreeList) =
        fn_8047FC04(rwSTRINGFREELISTENTRYSIZE, lbl_805EDEC8, 4, lbl_805EDECC, &lbl_805E2AF8,
                    rwMEMHINTDUR_GLOBAL | rwID_STRINGMODULE);
    if (RWSTRINGGLOBAL(stringFreeList) == NULL) {
        return NULL;
    }
    lbl_805F15A0.numInstances++;
    return instance;
}

void *fn_80482C50(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWSTRINGGLOBAL(stringFreeList) != NULL) {
        fn_8047FCDC(RWSTRINGGLOBAL(stringFreeList));
    }
    lbl_805F15A0.numInstances--;
    return instance;
}

/* Compares strings ignoring the case of letters */
RwInt32 fn_80482CA4(const RwChar *string1, const RwChar *string2) {
    if (string1 != NULL && string2 != NULL) {
        RwChar c1;
        RwChar c2;

        do {
            c1 = *string1;
            c2 = *string2;
            if (c1 >= 'A' && c1 <= 'Z') {
                c1 += 'a' - 'A';
            }
            if (c2 >= 'A' && c2 <= 'Z') {
                c2 += 'a' - 'A';
            }
            if (c1 != c2) {
                return c1 - c2;
            }
            string1++;
            string2++;
        } while (c1 != '\0' && c2 != '\0');

        if (c1 != c2) {
            return c1 - c2;
        }
    }
    return 0;
}

/* Converts a string's letters to upper case */
RwChar *fn_80482D3C(RwChar *string) {
    if (string != NULL) {
        RwChar ch;
        RwChar *c = string;

        while ((ch = *c) != '\0') {
            if (ch >= 'a' && ch <= 'z') {
                *c = ch - ('a' - 'A');
            }
            c++;
        }
    }
    return string;
}

/* Converts a string's letters to lower case */
RwChar *fn_80482D7C(RwChar *string) {
    if (string != NULL) {
        RwChar ch;
        RwChar *c = string;

        while ((ch = *c) != '\0') {
            if (ch >= 'A' && ch <= 'Z') {
                *c = ch + ('a' - 'A');
            }
            c++;
        }
    }
    return string;
}

/* The first occurrence of a character in a string, or NULL */
RwChar *fn_80482DBC(const RwChar *string, RwInt32 findThis) {
    const RwChar *found = NULL;
    RwChar c = (RwChar)findThis;
    RwChar ch;

    do {
        ch = *string;
        if (ch == c) {
            found = string;
            break;
        }
        string++;
    } while (ch != '\0');
    return (RwChar *)found;
}

/* The last occurrence of a character in a string, or NULL */
RwChar *fn_80482DF0(const RwChar *string, RwInt32 findThis) {
    const RwChar *found = NULL;
    RwChar c = (RwChar)findThis;
    RwChar ch;

    do {
        ch = *string;
        if (ch == c) {
            found = string;
        }
        string++;
    } while (ch != '\0');
    return (RwChar *)found;
}

/* Sets the engine's string functions: the C library's, with the engine's own case-insensitive
 * comparison, case conversion and character searches */
RwBool fn_80482E20(void) {
    lbl_805F15B4->stringFuncs.vecSprintf = sprintf;
    lbl_805F15B4->stringFuncs.vecVsprintf = vsprintf;
    lbl_805F15B4->stringFuncs.vecStrcpy = strcpy;
    lbl_805F15B4->stringFuncs.vecStrncpy = strncpy;
    lbl_805F15B4->stringFuncs.vecStrcat = strcat;
    lbl_805F15B4->stringFuncs.vecStrncat = strncat;
    lbl_805F15B4->stringFuncs.vecStrrchr = fn_80482DF0;
    lbl_805F15B4->stringFuncs.vecStrchr = fn_80482DBC;
    lbl_805F15B4->stringFuncs.vecStrstr = strstr;
    lbl_805F15B4->stringFuncs.vecStrcmp = strcmp;
    lbl_805F15B4->stringFuncs.vecStrncmp = strncmp;
    lbl_805F15B4->stringFuncs.vecStricmp = fn_80482CA4;
    lbl_805F15B4->stringFuncs.vecStrlen = strlen;
    lbl_805F15B4->stringFuncs.vecStrupr = fn_80482D3C;
    lbl_805F15B4->stringFuncs.vecStrlwr = fn_80482D7C;
    lbl_805F15B4->stringFuncs.vecStrtok = strtok;
    lbl_805F15B4->stringFuncs.vecSscanf = sscanf;
    return TRUE;
}

void fn_80482F60(void) {}

/* The size of a string's chunk data: the string and its terminator, padded to 4 bytes */
RwUInt32 fn_80482F64(const RwChar *string) {
    if (string == NULL) {
        string = lbl_805FB2D0;
    }
    return (rwstrlen(string) + 4) & ~3;
}

/* Writes a string chunk */
const RwChar *fn_80482FA4(const RwChar *string, RwStream *stream) {
    RwUInt32 size;

    if (string == NULL) {
        string = lbl_805FB2D0;
    }
    size = fn_80482F64(string);
    if (!fn_8047CEF0(stream, rwID_STRING, size, RW_STREAM_VERSION, RW_STREAM_BUILD)) {
        return NULL;
    }
    if (fn_80480ED0(stream, string, size)) {
        return string;
    }
    return NULL;
}

/* Reads a Unicode string chunk's characters into string, keeping the low byte of each, or into a
 * new string when it is NULL */
static inline RwChar *StringReadUnicode(RwChar *string, RwStream *stream, RwUInt32 length) {
    RwUInt16 buffer[rwUNICODESTRINGREADCHUNK / sizeof(RwUInt16)];
    RwBool allocated = FALSE;
    RwChar *dst;

    if (string == NULL) {
        string = RwMalloc(length, rwMEMHINTDUR_EVENT | rwID_STRING);
        if (string == NULL) {
            RWERROR((E_RW_NOMEM, length));
            return NULL;
        }
        allocated = TRUE;
    }
    dst = string;
    while (length != 0) {
        RwUInt32 chunk = rwUNICODESTRINGREADCHUNK;
        RwUInt32 numChars;
        RwUInt32 i;

        if (length <= rwUNICODESTRINGREADCHUNK) {
            chunk = length;
        }
        if (chunk != fn_80480D4C(stream, buffer, chunk)) {
            if (allocated) {
                RwFree(string);
            }
            return NULL;
        }
        numChars = chunk >> 1;
        length -= chunk;
        for (i = 0; i < numChars; i++) {
            dst[i] = (RwChar)buffer[i];
        }
        dst += numChars;
    }
    return string;
}

/* Finds the next string chunk and reads it into string, or into a new string when it is NULL;
 * other chunks before it are skipped */
RwChar *fn_80483060(RwChar *string, RwStream *stream) {
    RwUInt32 type;
    RwUInt32 length;
    RwUInt32 version;
    RwChar buffer[rwSTRINGREADCHUNK];

    while (fn_8047CDB4(stream, &type, &length, &version, NULL)) {
        RwBool versionOK = FALSE;

        if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
            versionOK = TRUE;
        }
        if (!versionOK) {
            RWERROR((E_RW_BADVERSION));
            return NULL;
        }
        if (type == rwID_STRING) {
            /* Reads the characters into string, or into a new string when it is NULL */
            RwUInt32 remaining = length;
            RwChar *dst;

            if (string == NULL) {
                string = RwMalloc(remaining, rwMEMHINTDUR_EVENT | rwID_STRING);
                if (string == NULL) {
                    RWERROR((E_RW_NOMEM, remaining));
                    return NULL;
                }
            }
            dst = string;
            while (remaining != 0) {
                RwUInt32 chunk = rwSTRINGREADCHUNK;
                RwUInt32 i;

                if (remaining <= rwSTRINGREADCHUNK) {
                    chunk = remaining;
                }
                if (chunk != fn_80480D4C(stream, buffer, chunk)) {
                    return NULL;
                }
                remaining -= chunk;
                for (i = 0; i < chunk; i++) {
                    dst[i] = buffer[i];
                }
                dst += chunk;
            }
            return string;
        }
        if (type == rwID_UNICODESTRING) {
            return StringReadUnicode(string, stream, length);
        }
        if (!fn_804810FC(stream, length)) {
            return NULL;
        }
    }
    return NULL;
}
