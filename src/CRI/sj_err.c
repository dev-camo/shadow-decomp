/* CRI stream-joint error reporting (sj_err.c). */

#include "CRI/sj.h"
#include "CRI/svm.h"

/* Reports a stream-joint error message through the server manager's error callback. */
void SJERR_CallErr(Char8 *msg) {
    SVM_CallErr1(msg);
}
