#ifndef PTMF_H
#define PTMF_H
#include "layout_check.h"

typedef struct __ptmf {
    long this_delta; // adjustment added to the object pointer
    long v_offset;   // vtable offset
    union {
        void *f_addr;   // function address
        long ve_offset; // virtual function entry offset (of vtable)
    } f_data;
} __ptmf;
CHECK_SIZEOF(__ptmf, 0xC);

#endif /* PTMF_H */
