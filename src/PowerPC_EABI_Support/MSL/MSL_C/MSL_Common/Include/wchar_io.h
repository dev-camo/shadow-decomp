#ifndef _MSL_COMMON_WCHAR_IO_H
#define _MSL_COMMON_WCHAR_IO_H

#include <ansi_files.h>

#ifdef __cplusplus
extern "C" {
#endif

int fwide(FILE *file, int mode);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_WCHAR_IO_H */
