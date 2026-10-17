#ifndef _MSL_COMMON_ARITH_H
#define _MSL_COMMON_ARITH_H
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct div_t {
    int quot; /* quotient */
    int rem;  /* remainder */
} div_t;
CHECK_SIZEOF(div_t, 0x8);

int abs(int n);
long int labs(long int n);
div_t div(int numerator, int denominator);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_ARITH_H */
