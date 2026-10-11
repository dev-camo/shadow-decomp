/* CRI memory copy (mcp_not.c): the portable MEM_Copy over the C library memcpy.
 *
 * Source from the RE4 GameCube decompilation, unchanged.
 */
#include "CRI/cri_xpt.h"

extern void *memcpy(void *destination, const void *source, unsigned int size);

/* Copies nbytes from src to dst. */
void MEM_Copy(void *dst, const void *src, Uint32 nbytes) {
    memcpy(dst, src, nbytes);
}
