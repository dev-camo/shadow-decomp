/* CRI memory copy (mcp_not.c): the portable MEM_Copy over the C library memcpy.
 *
 * Source from the RE4 GameCube decompilation, unchanged.
 */
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/uty.h"

/* Copies nbytes from src to dst. */
void MEM_Copy(void *dst, const void *src, Uint32 nbytes) {
    memcpy(dst, src, nbytes);
}
