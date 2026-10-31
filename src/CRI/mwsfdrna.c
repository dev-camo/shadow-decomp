/* CRI Sofdec MW player audio output accessors (mwsfdrna.c): volume of the movie audio, forwarded
 * to the SFD audio driver (SFD_SetOutVol -> SFAOAP -> ADXT).
 *
 * Names follow the RE4 GameCube decompilation of the same file.
 */
#include "CRI/mwsfd.h"

/* Volume of the movie audio (1/10 dB, <= 0). */
Sint32 MWSFRNA_GetOutVol(MWPLY mwply) {
    return SFD_GetOutVol(mwply->sfd);
}

/* Sets the volume of the movie audio (1/10 dB, <= 0). */
void MWSFRNA_SetOutVol(MWPLY mwply, Sint32 vol) {
    SFD_SetOutVol(mwply->sfd, vol);
}
