/* CRI ADX GameCube extension (adx_gc.c): per-handle switch for the AX sampling-rate correction.
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/adx_rna.h"
#include "CRI/adx_t.h"

/* Enables (1) or disables the renderer's DSP rate correction for this handle's voices. */
void ADXGC_SetAdjsfreqFlg(ADXT_OBJ *adxt, Sint32 flg) {
    ADXRNA_SetAdjsfreqFlg(adxt->rna, flg);
}
