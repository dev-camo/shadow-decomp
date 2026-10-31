#include "CRI/mwsfdsvm.h"

/* Forwards the format string to MWSFSVM_Error; `context` is unused and its type is inferred. */
void fn_80406E88(void *context, const char *format) {
    MWSFSVM_Error(format);
}
