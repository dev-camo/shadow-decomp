/* CRI GCCI settings (gcci_sub.c): the DVD read mode used by gcci.c (0: asynchronous
 * DVDReadAsyncPrio, otherwise blocking DVDReadPrio) and the root directory that gcCiOpen and
 * gcCiGetFileSize prepend to GCD-device file names (empty in this game).
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/cvfs.h"

/* Matching workaround: explicit_zero_data. The original keeps the zero-initialised gcg_ci_rdmode in
 * .data; without the pragma MWCC places it in .bss. Revisit when the original's compiler flags or
 * initialiser are known. */
#pragma explicit_zero_data on

Sint32 gcg_ci_rdmode = 0;
Char8 gcg_ci_root_dir[256];

/* Sets gcg_ci_rdmode; ADXGC_SetupDvdFs passes its parameters' read mode, or 0 when called
 * without parameters (as this game calls it). */
void gcCiSetRdMode(void *obj, Sint32 a, Sint32 b, Sint32 mode) {
    gcg_ci_rdmode = mode;
}
