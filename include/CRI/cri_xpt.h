#ifndef CRI_CRI_XPT_H
#define CRI_CRI_XPT_H

/* CRI basic types (cri_xpt.h in the CRI middleware SDK). 32-bit integers are long, which
 * matters for MWCC code generation.
 */
typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned long Uint32;
typedef signed long Sint32;
typedef unsigned long long Uint64;
typedef signed long long Sint64;
typedef float Float32;
typedef double Float64;
typedef char Char8;
typedef Sint32 Bool;

#define TRUE 1
#define FALSE 0

#endif
