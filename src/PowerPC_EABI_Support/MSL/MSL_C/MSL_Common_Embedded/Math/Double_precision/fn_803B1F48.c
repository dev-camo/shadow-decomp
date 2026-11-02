/* Public double-precision square-root wrapper; the original filename is unknown. */
#include "fdlibm.h"

double sqrt(double x) {
    return __ieee754_sqrt(x);
}
