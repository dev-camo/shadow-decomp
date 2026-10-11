/* CRI ADXB AHX hooks (adx_bahx.c): the AHX (MPEG-2 audio layer 2 based voice codec) decoder is a
 * separate module that registers its functions with ADXB_EntryAhxFunc; ADXB forwards the AHX-format
 * calls through these pointers. The AHX module is not linked in this game (adxb->ahx is always 0).
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/adx_b.h"

void (*ahxsetsjifunc)(void *ahx, void *sji);
void (*ahxsetdecsmplfunc)(void *ahx, Sint32 nsmpl);
void (*ahxtermsupplyfunc)(void *ahx);
void (*ahxexecfunc)(ADXB adxb);

/* dead-stripped by the linker (nothing calls it); fixes the .bss order of the four pointers */
void ADXB_EntryAhxFunc(void (*setsji)(void *, void *), void (*setdecsmpl)(void *, Sint32),
                       void (*termsupply)(void *), void (*exec)(ADXB)) {
    ahxsetsjifunc = setsji;
    ahxsetdecsmplfunc = setdecsmpl;
    ahxtermsupplyfunc = termsupply;
    ahxexecfunc = exec;
}

/* End of input for an AHX decoder attached to the handle. */
void ADXB_AhxTermSupply(ADXB adxb) {
    if (adxb->ahx != 0) {
        ahxtermsupplyfunc(adxb->ahx);
    }
}

/* One decode step of an AHX stream (through the registered hook). */
void ADXB_ExecOneAhx(ADXB adxb) {
    ahxexecfunc(adxb);
}

/* Per-tick decode limit: forwarded to the AHX decoder and kept in samples and 96-sample units. */
void ADXB_SetAhxDecSmpl(ADXB adxb, Sint32 nsmpl) {
    if (adxb->ahx != 0) {
        ahxsetdecsmplfunc(adxb->ahx, nsmpl);
    }
    adxb->nsmpl = nsmpl;
    adxb->nsmpl96 = nsmpl / 96;
}

/* Input stream joint of the AHX decoder. */
void ADXB_SetAhxInSj(ADXB adxb, void *sji) {
    if (adxb->ahx != 0) {
        ahxsetsjifunc(adxb->ahx, sji);
    }
}
