/* CRI ADXAMP (adx_amp.c): amplitude/volume processing stage between the decoder output and the
 * renderer stream joints. Never created in this game (ADXT_OBJ.amp stays NULL); the functions that
 * survived are called only through ADXT's NULL-guarded amp hooks.
 *
 * Source from the RE4 GameCube decompilation. Only these four functions are in this game.
 */
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_amp.h"
#include "CRI/sj.h"
#include "layout_check.h"

struct ADXAMP_OBJ {
    Sint8 used;      /* 0x00 */
    Sint8 stat;      /* 0x01 */
    Sint8 nch;       /* 0x02 */
    Sint8 rsv;       /* 0x03 */
    SJ sji[2];       /* 0x04 */
    SJ sjo[2];       /* 0x0C */
    Sint32 nsmpl[2]; /* 0x14 */
    Sint32 rsv1C;    /* 0x1C */
    Sint32 sfreq;    /* 0x20 */
    Float32 vol;     /* 0x24 */
    Sint32 rsv28;    /* 0x28 */
    Sint32 total;    /* 0x2C */
};
CHECK_SIZEOF(struct ADXAMP_OBJ, 0x30);
typedef struct ADXAMP_OBJ ADXAMP_OBJ;

/* Records the sampling rate. */
void ADXAMP_SetSfreq(ADXAMP_OBJ *amp, Sint32 sfreq) {
    amp->sfreq = sfreq;
}

/* Stops the stage (stat 0). */
void ADXAMP_Stop(ADXAMP_OBJ *amp) {
    amp->stat = 0;
}

/* Resets the sample counters and zero-fills / resets the input and output stream joints, then runs
 * (stat 2). Called by adxt_start_sj when a handle has an amp. */
void ADXAMP_Start(ADXAMP_OBJ *amp) {
    SJCK ck;
    Sint32 i;
    SJ sj;
    Sint32 n;

    for (i = 0; i < amp->nch; i++) {
        amp->nsmpl[i] = 0;
    }
    amp->total = 0;
    for (i = 0; i < amp->nch; i++) {
        sj = amp->sji[i];
        SJ_Reset(sj);
        n = SJ_GetNumData(sj, 0);
        SJ_GetChunk(sj, 0, n, &ck);
        memset(ck.data, 0, ck.len);
        SJ_UngetChunk(sj, 0, &ck);
    }
    for (i = 0; i < amp->nch; i++) {
        sj = amp->sjo[i];
        SJ_Reset(sj);
        n = SJ_GetNumData(sj, 0);
        SJ_GetChunk(sj, 0, n, &ck);
        memset(ck.data, 0, ck.len);
        SJ_UngetChunk(sj, 0, &ck);
    }
    amp->stat = 2;
}

/* Clears the object under the ADX lock. */
void ADXAMP_Destroy(ADXAMP_OBJ *amp) {
    if (amp != 0) {
        ADXCRS_Lock();
        memset(amp, 0, sizeof(ADXAMP_OBJ));
        ADXCRS_Unlock();
    }
}
