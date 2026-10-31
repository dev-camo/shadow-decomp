#ifndef GAME_FN_8042_BYTE_ORDER_H
#define GAME_FN_8042_BYTE_ORDER_H

#include "Game/fn_8042_name_registry.h"

/* Matching workaround: a macro instead of an inline function, because an inline function would
 * keep the inline functions that use it, such as FixEntryHeader, from being inlined, which
 * changes fn_80410630.o and fn_80411714.o. Revisit if the original's form of the test, or another
 * inline setting for these units, is found.
 *
 * Whether the byte-order flag that a file stores differs from the setting that fn_804211CC
 * returns, so that the loader has to reverse the bytes of the file's values (inferred from the
 * pack and resource group loaders). */
#define NEEDS_BYTE_SWAP(byteOrderFlag)                                                             \
    (((byteOrderFlag) && !fn_804211CC()) || (!(byteOrderFlag) && fn_804211CC()))

#endif
