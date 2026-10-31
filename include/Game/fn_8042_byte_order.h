#ifndef FN_8042_BYTE_ORDER_H
#define FN_8042_BYTE_ORDER_H

#include "Game/fn_8042_name_registry.h"

/* Whether the byte-order flag that a file stores differs from the setting that fn_804211CC
 * returns, so that the loader has to reverse the bytes of the file's values (inferred from the
 * pack and resource group loaders). A macro: an inline function would keep the inline functions
 * that use it from being inlined. */
#define NEEDS_BYTE_SWAP(byteOrderFlag)                                                             \
    (((byteOrderFlag) && !fn_804211CC()) || (!(byteOrderFlag) && fn_804211CC()))

#endif
