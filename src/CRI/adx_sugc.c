/* CRI ADX GameCube SDK setup (adx_sugc.c, ADXGCSDK Ver.20Apr2004Patch2): registers the CVFS
 * devices the ADX file layer reads from. Body from the RE4 GameCube decompilation of the same
 * file.
 */
#include "CRI/adx.h"
#include "CRI/cvfs.h"
#include "layout_check.h"

typedef struct {
    Sint32 rdmode; /* 0x00 */
} ADXGC_DVDFS_PRM;
CHECK_SIZEOF(ADXGC_DVDFS_PRM, 0x4);

/* Matching workaround: the build string pointer is volatile and ADXGC_SetupDvdFs reads it without
 * using the value. The original function has that unused load (lwz r0,0x3C(r31) at offset 0x1C),
 * so its source referenced the pointer; declared without volatile, the read is dropped and
 * ADXGC_SetupDvdFs is 46 instead of 48 instructions (46 words differ). Revisit when the original
 * reference to the string is found. */
const Char8 *const volatile adxgcsdk_build =
    "\nADXGCSDK Ver.20Apr2004Patch2 Build:Feb 28 2005 15:49:05\n";

void adxgc_err_dvd(void *obj, const Char8 *msg, void *hn);

/* Adds the memory-file device "MFS" and the DVD device "GCD" to the CVFS, makes GCD the default
 * device and sets its read mode (prm->rdmode, 0 without parameters).
 */
void ADXGC_SetupDvdFs(ADXGC_DVDFS_PRM *prm) {
    adxgcsdk_build;
    cvFsEntryErrFunc(adxgc_err_dvd, 0);
    cvFsAddDev("MFS", mfCiGetInterface, 0);
    cvFsEntryErrFunc(adxgc_err_dvd, 0);
    cvFsAddDev("GCD", gcCiGetInterface, 0);
    cvFsSetDefDev("GCD");
    if (prm != 0) {
        gcCiSetRdMode(0, 0, 0, prm->rdmode);
    } else {
        gcCiSetRdMode(0, 0, 0, 0);
    }
}

/* CVFS error callback: forwards the message to the ADX error callback. */
void adxgc_err_dvd(void *obj, const Char8 *msg, void *hn) {
    ADXERR_CallErrFunc1(msg);
}
