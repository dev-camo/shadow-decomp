#ifndef _STDDEF_H_
#define _STDDEF_H_

#define offsetof(type, member) ((size_t)&(((type *)0)->member))

/* unsigned int, not MWCC's C++ size type unsigned long: the matched C objects (CRI, MetroTRK,
 * RenderWare) change code when built with unsigned long (see #180). */
typedef unsigned int size_t;

#ifndef NULL
#define NULL 0L
#endif

#endif
