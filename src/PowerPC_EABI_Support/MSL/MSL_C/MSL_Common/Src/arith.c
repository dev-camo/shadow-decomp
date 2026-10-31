#include "arith.h"

int abs(int n) {
    if (n < 0)
        return (-n);
    else
        return (n);
}

long labs(long __x) {
    return __x > 0 ? __x : -__x;
}
