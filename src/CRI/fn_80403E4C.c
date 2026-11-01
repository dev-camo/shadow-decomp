/* CRI Sofdec MW player glue for the SFX frame converter library: stops the converter, routes its
 * errors to the player's error handler and starts it.
 *
 * This is the tail of the original mwsfdsfx.c, so the unit keeps a placeholder name. The same file
 * also holds 0x80403E04-0x80403E4C (USA) directly before it: MWSFSFX_Destroy and MWSFSFX_Create
 * (CRI/mwsfdsfx.c), which call SFX_Destroy and SFX_Create, and fn_80403E44 (Game/fn_80403E44.c),
 * which returns the converter's minimum work size. The names of mwsfsfx_SfxErrCbFn and
 * MWSFSFX_Init follow RE4's GameCube symbols of that file.
 */
#include <stddef.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/sfx.h"

/* Stops the converter library. The name is inferred: the counterpart of MWSFSFX_Init. */
void MWSFSFX_Finish(void) {
    SFX_Finish();
}

/* Error callback installed by MWSFSFX_Init: passes the converter's message to the player's error
 * handler. The object argument is unused. */
void mwsfsfx_SfxErrCbFn(void *obj, const Char8 *msg) {
    MWSFSVM_Error(msg);
}

/* Starts the converter library and routes its errors to the player. */
void MWSFSFX_Init(void) {
    SFX_Init();
    SFX_SetErrFn(mwsfsfx_SfxErrCbFn, NULL);
}
