#ifndef CRI_ADX_RNA_H
#define CRI_ADX_RNA_H

/* CRI ADX renderer front end (adx_rnaa.c, ADXRNA_*) and the GameCube AX renderer it forwards to
 * (AXRNA_*). The renderer handle stays opaque. Signatures follow the RE4 decompilation.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

typedef struct AXRNA_OBJ *AXRNA;
typedef struct RNARES_OBJ *RNARES; /* name inferred */

#ifdef __cplusplus
extern "C" {
#endif

/* adx_rnaa.c */
void ADXRNA_SetAdjsfreqFlg(AXRNA rna, Sint32 flg);
Sint32 ADXRNA_SetStmHdInfo(AXRNA rna, void *hdinfo);
void ADXRNA_SetTotalNumSmpl(AXRNA rna, Sint32 nsmpl);
Sint32 ADXRNA_DiscardData(AXRNA rna, Sint32 nsmpl);
void ADXRNA_SetBitPerSmpl(AXRNA rna, Sint32 bps);
void ADXRNA_SetOutBalance(AXRNA rna, Sint32 bal);
void ADXRNA_SetOutPan(AXRNA rna, Sint32 ch, Sint32 pan);
void ADXRNA_SetOutVol(AXRNA rna, Sint32 vol);
void ADXRNA_SetSfreq(AXRNA rna, Sint32 sfreq);
void ADXRNA_SetNumChan(AXRNA rna, Sint32 nch);
void ADXRNA_ExecServer(void);
Sint32 ADXRNA_GetNumRoom(AXRNA rna);
Sint32 ADXRNA_GetNumData(AXRNA rna);
void ADXRNA_SetPlaySw(AXRNA rna, Sint32 sw);
void ADXRNA_SetTransSw(AXRNA rna, Sint32 sw);
void ADXRNA_Destroy(AXRNA rna);
AXRNA ADXRNA_Create(SJ *sjo, Sint32 nch, void *work);
void ADXRNA_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj);
void ADXRNA_Finish(void);
void ADXRNA_Init(void);

/* rna_err.c */
void RNAERR_CallErrFunc(Char8 *msg);
void RNAERR_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj);

/* rna_res.c: ARAM buffers of the AX renderer */
Sint32 RNARES_GetBufSize(RNARES res);
Uint32 RNARES_GetBuf(RNARES res);
void RNARES_Destroy(RNARES res);
RNARES RNARES_Create(void);
void RNARES_Finish(void);
void RNARES_Init(void);

/* AX renderer */
void AXRNA_SetAdjsfreqFlg(AXRNA rna, Sint32 flg);
Sint32 AXRNA_SetStmHdInfo(AXRNA rna, void *hdinfo);
Sint32 AXRNA_DiscardData(AXRNA rna, Sint32 nsmpl);
void AXRNA_SetBitPerSmpl(AXRNA rna, Sint32 bps);
void AXRNA_SetOutPan(AXRNA rna, Sint32 ch, Sint32 pan);
void AXRNA_SetOutVol(AXRNA rna, Sint32 vol);
void AXRNA_SetSfreq(AXRNA rna, Sint32 sfreq);
void AXRNA_SetNumChan(AXRNA rna, Sint32 nch);
void AXRNA_ExecServer(void);
Sint32 AXRNA_GetNumRoom(AXRNA rna);
Sint32 AXRNA_GetNumData(AXRNA rna);
void AXRNA_SetPlaySw(AXRNA rna, Sint32 sw);
void AXRNA_SetTransSw(AXRNA rna, Sint32 sw);
void AXRNA_Destroy(AXRNA rna);
AXRNA AXRNA_Create(SJ *sj, Sint32 maxnch);
void AXRNA_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj);
void AXRNA_Finish(void);
void AXRNA_Init(void);

/* rna_crs.c */
void GCRNA_UnlockCs(void);
void GCRNA_LockCs(void);

#ifdef __cplusplus
}
#endif

#endif
