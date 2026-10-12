#ifndef _MSL_COMMON_WSCANF_H
#define _MSL_COMMON_WSCANF_H

#include <wstring.h>
#include <cstddef>
#include "scanf.h" /* __wOutStrCtrl and the other string-control types */

#ifdef __cplusplus
extern "C" {
#endif

wchar_t __wStringRead(wString *src, wchar_t param_1, wchar_t param_2);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_WSCANF_H */
