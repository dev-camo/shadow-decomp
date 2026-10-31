#ifndef _MSL_COMMON_BUFFER_IO_H
#define _MSL_COMMON_BUFFER_IO_H

#include <cstdio>

enum { __align_buffer, __dont_align_buffer };

#ifdef __cplusplus
extern "C" {
#endif

void __prep_buffer(FILE *file);
int __flush_buffer(FILE *file, size_t *bytes_flushed);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_BUFFER_IO_H */
