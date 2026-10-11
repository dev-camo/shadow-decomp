/* CRI ADXF cache control (adx_fcch.c): the data-cache invalidate the file layer applies to a
 * buffer before the DVD DMA writes into it. Name from the RE4 decompilation.
 */
#include "CRI/cri_xpt.h"

extern void DCInvalidateRange(void *addr, Uint32 nBytes);

/* Invalidates the data cache over the DMA target range. */
void ADXF_Ocbi(void *addr, Uint32 nbytes) {
    DCInvalidateRange(addr, nbytes);
}
