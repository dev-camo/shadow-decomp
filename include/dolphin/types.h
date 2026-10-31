#ifndef _DOLPHIN_TYPES_H_
#define _DOLPHIN_TYPES_H_

typedef signed char s8;
typedef unsigned char u8;
typedef signed short int s16;
typedef unsigned short int u16;
typedef signed long s32;
typedef unsigned long u32;
typedef signed long long int s64;
typedef unsigned long long int u64;

typedef float f32;
typedef double f64;

typedef char *Ptr;

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#if defined(__MWERKS__)
#define AT_ADDRESS(addr) : (addr)
#elif defined(__GNUC__)
#define AT_ADDRESS(addr) // GCC has no equivalent; place the object with a linker script instead.
#else
#error unknown compiler
#endif

#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))

#if defined(__MWERKS__) && defined(__cplusplus)
/* C++ is compiled against MSL's C library headers, which also define NULL and INT_MAX. */
#include <climits>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cctype>
#else
#define INT_MAX 2147483647

/* NULL is defined here and in <stddef.h> (`0L`), both under #ifndef NULL, so a unit gets the
 * definition of whichever of the two headers it includes first. `(void *)0` in <stddef.h> changes
 * CRI/mpv_lib.o and breaks the final DOL; unifying both on `0L` changes the code of NonMatching
 * units, so it waits until they are matched. */
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef __MWERKS__
#include "libc/stdio.h"
#include "libc/stdarg.h"
#include "libc/string.h"
#include "libc/ctype.h"
#include "libc/cmath.h"
#else
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#endif
#endif

#ifndef __MWERKS__
#include <math.h>
#endif

#endif
