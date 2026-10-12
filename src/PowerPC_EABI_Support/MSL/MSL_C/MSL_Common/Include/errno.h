#ifndef MSL_COMMON_SRC_ERRNO_H
#define MSL_COMMON_SRC_ERRNO_H

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

/* values from MSL's errno.h */
#define ENOERR 0
#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EDEADLK 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34
#define EAGAIN 35
#define ESIGPARM 36
#define EFPOS 40
#define ENAMETOOLONG 63
#define ENOTEMPTY 66
#define ENOLCK 77
#define ENOSYS 78
#define EILSEQ 88
#define EUNKNOWN 99

extern int errno;

#ifdef __cplusplus
}
#endif

#endif /* MSL_COMMON_SRC_ERRNO_H */
