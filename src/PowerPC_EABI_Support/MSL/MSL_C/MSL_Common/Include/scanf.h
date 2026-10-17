#ifndef _MSL_COMMON_SCANF_H
#define _MSL_COMMON_SCANF_H

#include <ansi_files.h>
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

enum __ReadProcActions { __GetAChar, __UngetAChar, __TestForError };

enum __WReadProcActions { __GetAwChar, __UngetAwChar, __TestForwcsError };

typedef struct {
    char *CharStr;
    size_t MaxCharCount;
    size_t CharsWritten;
} __OutStrCtrl;
CHECK_SIZEOF(__OutStrCtrl, 0xC);

typedef struct {
    char *NextChar;
    int NullCharDetected;
} __InStrCtrl;
CHECK_SIZEOF(__InStrCtrl, 0x8);

typedef struct {
    wchar_t *wCharStr;
    size_t MaxCharCount;
    size_t CharsWritten;
} __wOutStrCtrl;
CHECK_SIZEOF(__wOutStrCtrl, 0xC);

typedef struct {
    wchar_t *wNextChar;
    int wNullCharDetected;
} __wInStrCtrl;
CHECK_SIZEOF(__wInStrCtrl, 0x8);

int __StringRead(void *str, int ch, int behavior);
int sscanf(const char *buffer, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_SCANF_H */
