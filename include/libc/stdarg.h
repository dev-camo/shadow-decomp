#ifndef _STDARG_H_
#define _STDARG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MWERKS__
#include "va_list_struct.h"

/* When MSL's va_list header is included as well, its va_list and macros are used */
#ifndef MSL_VA_LIST_H
typedef __va_list va_list;

#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_arg(ap, t) (*((t *)__va_arg(ap, _var_arg_typeof(t))))
#define va_end(ap) (void)0
#endif

#else
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#endif

#ifdef __cplusplus
}
#endif

#endif
