/* RenderWare GameCube driver: testing whether the GPU has passed a draw sync token. The original
 * file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

volatile RwUInt16 lbl_805F1678;

/* Whether the GPU has passed the draw that issued token. Tokens are issued in increasing order from
 * lbl_805EDF40 and wrap below rwGAMECUBEMAXSYNCTOKEN, so a token has been passed if it lies in the
 * range from the next token to be issued up to the last token GX read back, taking the wrap into
 * account. A read back value outside the token range means no issued token has been passed yet. */
RwBool fn_8049A78C(RwUInt16 token) {
    lbl_805F1678 = GXReadDrawSync();
    if (lbl_805F1678 >= rwGAMECUBEMAXSYNCTOKEN) {
        return FALSE;
    }

    if (lbl_805EDF40 >= lbl_805F1678) {
        /* the issued tokens have not wrapped since the token that was read back */
        return token <= lbl_805F1678 || token > lbl_805EDF40;
    }
    return token > lbl_805EDF40 && token <= lbl_805F1678;
}
