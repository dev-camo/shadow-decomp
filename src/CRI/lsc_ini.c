/* CRI LSC init / finish (lsc_ini.c, LSC/GC Ver.2.22): the scheduler table lsc_obj and the
 * build string. Names and layout follow the RE4 GameCube decompilation of the same file.
 */

#include <string.h>

#include "CRI/lsc.h"

/* volatile: LSC_Init reads the build string pointer without using it. */
const Char8 *const volatile lsc_build = "\nLSC/GC Ver.2.22 Build:Feb 28 2005 15:48:30\n";

static Sint32 lsc_init_cnt = 0;
LSC_OBJ lsc_obj[LSC_MAX_OBJ];

/* Destroys the live schedulers and clears the table and error callback on the last release. */
void LSC_Finish(void) {
    Sint32 msk;
    LSC_OBJ *lsc;
    Sint32 i;

    LSC_LockCrs(&msk);
    if (--lsc_init_cnt == 0) {
        for (i = 0; i < LSC_MAX_OBJ; i++) {
            lsc = &lsc_obj[i];
            if (lsc->used == 1) {
                LSC_Destroy(lsc);
            }
        }
        memset(lsc_obj, 0, sizeof(lsc_obj));
        LSC_EntryErrFunc(0, 0);
    }
    LSC_UnlockCrs(&msk);
}

/* Clears the table and error callback on the first init. */
void LSC_Init(void) {
    Sint32 msk;

    lsc_build;
    LSC_LockCrs(&msk);
    if (lsc_init_cnt == 0) {
        memset(lsc_obj, 0, sizeof(lsc_obj));
        LSC_EntryErrFunc(0, 0);
    }
    lsc_init_cnt++;
    LSC_UnlockCrs(&msk);
}
