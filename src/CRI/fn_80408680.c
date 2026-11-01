/* CRI Sofdec MW player: sets up the ARGB8888 converters in the player's SFX handle and converts a
 * frame.
 *
 * The original file name is unknown. In the RE4 GameCube decompilation the player's ARGB8888
 * entry (mwsfx_ARGB8888PLN.c) is linked directly after mwsfdsst.c, as this function is here, but
 * its body differs, so neither that file name nor its function name is used.
 */
#include "CRI/cft.h"
#include "CRI/mwsfd.h"

/* Sets up the ARGB8888 converters in the converter handle of the player (unit width 4, the alpha
 * converter, the three alpha table makers and the colour adjustment table maker), then converts
 * the frame frm into buf with CFT_Ycc420plnToArgb8888, which fn_804035A8 registers as the
 * conversion function. */
void fn_80408680(MWPLY mwply, MWS_FRM *frm, Uint8 *buf) {
    SFX_OBJ *sfx = mwply->sfx;

    SFX_SetUnitWidth(sfx, 4);
    SFX_SetAlpFn(sfx, CFT_Ycc420plnToA256V);
    SFX_SetMakeLumiFn(sfx, CFT_MakeArgb8888AlpLumiTbl);
    SFX_SetMakeAlp3211Fn(sfx, CFT_MakeArgb8888Alp3211Tbl);
    SFX_SetMakeAlp3110Fn(sfx, CFT_MakeArgb8888Alp3110Tbl);
    SFX_SetTblFn(sfx, CFT_MakeArgb8888ColAdjTbl);
    fn_804035A8(mwply, frm, buf, CFT_Ycc420plnToArgb8888);
}
