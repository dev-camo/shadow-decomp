#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <stdio.h>

/* wchar_t is a keyword in C++ */
#ifndef __cplusplus
typedef unsigned short wchar_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif
int fwide(FILE *stream, int mode);
#ifdef __cplusplus
}
#endif

#endif
