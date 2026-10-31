#ifndef _MSL_COMMON_SIGNAL_H
#define _MSL_COMMON_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*__signal_func_ptr)(int);

#define SIG_DFL ((__signal_func_ptr)0)
#define SIG_IGN ((__signal_func_ptr)1)

int raise(int sig);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_SIGNAL_H */