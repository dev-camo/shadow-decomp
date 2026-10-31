#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <stddef.h>

/* Opaque here: the complete record is MSL's struct _FILE (ansi_files.h). */
typedef struct _FILE FILE;

/* C linkage, so C++ callers reach the C library's functions rather than mangled names */
#ifdef __cplusplus
extern "C" {
#endif

int puts(const char *s);
int printf(const char *, ...);
int sprintf(char *s, const char *format, ...);
int vprintf(const char *format, va_list arg);
int vsprintf(char *s, const char *format, va_list arg);
int sscanf(const char *s, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

#ifdef __cplusplus
}
#endif

#endif
