/* CRI UTY GameCube helpers (uty_ppc.c): save and restore of the graphics quantisation registers
 * GQR2..GQR7 around the paired-single decoder kernels (the frame decoder, fn_803D7B38), so the
 * game's own GQR setup survives a frame decode.
 * The function names come from the symbols of the same CRI library in another GameCube title.
 */
#include "CRI/uty.h"

/* Restores GQR2..GQR7 from the save block. */
void UTY_PopGqr(UTY_GQR *gqr) {
    register Uint32 g2 = gqr->gqr[2];
    register Uint32 g3 = gqr->gqr[3];
    register Uint32 g4 = gqr->gqr[4];
    register Uint32 g5 = gqr->gqr[5];
    register Uint32 g6 = gqr->gqr[6];
    register Uint32 g7 = gqr->gqr[7];

    // clang-format off
    asm {
        mtspr GQR2, g2
        mtspr GQR3, g3
        mtspr GQR4, g4
        mtspr GQR5, g5
        mtspr GQR6, g6
        mtspr GQR7, g7
    }
    // clang-format on
}

/* Saves GQR2..GQR7 into the save block. */
void UTY_PushGqr(UTY_GQR *gqr) {
    register Uint32 g2;
    register Uint32 g3;
    register Uint32 g4;
    register Uint32 g5;
    register Uint32 g6;
    register Uint32 g7;

    // clang-format off
    asm {
        mfspr g2, GQR2
        mfspr g3, GQR3
        mfspr g4, GQR4
        mfspr g5, GQR5
        mfspr g6, GQR6
        mfspr g7, GQR7
    }
    // clang-format on
    gqr->gqr[2] = g2;
    gqr->gqr[3] = g3;
    gqr->gqr[4] = g4;
    gqr->gqr[5] = g5;
    gqr->gqr[6] = g6;
    gqr->gqr[7] = g7;
}
