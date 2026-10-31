/* CRI Sofdec SFX picture user data (sfx_sud.c): the start and stop of the SUD module (sud_lib.c)
 * for the SFX library. The file name is the one that holds SFXSUD_Init in the RE4 GameCube
 * decompilation; only SFXSUD_Init and SFXSUD_Finish are linked in this game.
 */
#include "CRI/sfx.h"
#include "CRI/sud.h"

/* Name inferred: the counterpart of SFXSUD_Init. */
void SFXSUD_Finish(void) {
    SUD_Finish();
}

void SFXSUD_Init(void) {
    SUD_Init();
}
