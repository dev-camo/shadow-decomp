#ifndef CRI_ADX_SJD_H
#define CRI_ADX_SJD_H

/* CRI ADX stream-joint decoder (adx_sjd.c) entry points used by the ADXT layer. The decoder
 * handle stays opaque outside adx_sjd.c.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

typedef struct ADXSJD_OBJ *ADXSJD;

#ifdef __cplusplus
extern "C" {
#endif

void ADXSJD_Init(void);
void ADXSJD_Finish(void);
ADXSJD ADXSJD_Create(SJ sji, Sint32 nch, SJ *sjo);
void ADXSJD_Destroy(ADXSJD sjd);
Sint32 ADXSJD_GetStat(ADXSJD sjd);
void ADXSJD_SetInSj(ADXSJD sjd, SJ sji);
void ADXSJD_SetMaxDecSmpl(ADXSJD sjd, Sint32 nsmpl);
void ADXSJD_TermSupply(ADXSJD sjd);
void ADXSJD_Start(ADXSJD sjd);
void ADXSJD_Stop(ADXSJD sjd);
void ADXSJD_ExecHndl(ADXSJD sjd);
void ADXSJD_ExecServer(void);
Sint32 ADXSJD_GetDecDtLen(ADXSJD sjd);
Sint32 ADXSJD_GetDecNumSmpl(ADXSJD sjd);
void ADXSJD_SetDecPos(ADXSJD sjd, Sint32 pos);
void ADXSJD_SetLnkSw(ADXSJD sjd, Sint32 sw);
void ADXSJD_EntryTrapFunc(ADXSJD sjd, void (*fn)(void *obj), void *obj);
void ADXSJD_SetTrapNumSmpl(ADXSJD sjd, Sint32 nsmpl);
void ADXSJD_SetTrapCnt(ADXSJD sjd, Sint32 cnt);
void ADXSJD_SetTrapDtLen(ADXSJD sjd, Sint32 len);
Sint32 ADXSJD_GetFormat(ADXSJD sjd);
Sint32 ADXSJD_GetSfreq(ADXSJD sjd);
Sint32 ADXSJD_GetNumChan(ADXSJD sjd);
Sint32 ADXSJD_GetOutBps(ADXSJD sjd);
Sint32 ADXSJD_GetBlkSmpl(ADXSJD sjd);
Sint32 ADXSJD_GetTotalNumSmpl(ADXSJD sjd);
Sint32 ADXSJD_GetNumLoop(ADXSJD sjd);

/* Not yet decompiled; used only by unreferenced (linker-stripped) ADXT functions. */
void ADXSJD_SetKeyString(ADXSJD sjd, Char8 *str);
Sint32 ADXSJD_GetFmtBps(ADXSJD sjd);
Sint32 ADXSJD_GetHdrLen(ADXSJD sjd);
Sint32 ADXSJD_GetLpStartPos(ADXSJD sjd);
Sint32 ADXSJD_GetLpStartOfst(ADXSJD sjd);
Sint32 ADXSJD_GetLpEndPos(ADXSJD sjd);
Sint32 ADXSJD_GetLpEndOfst(ADXSJD sjd);
Sint16 ADXSJD_GetDefOutVol(ADXSJD sjd);
Sint16 ADXSJD_GetDefPan(ADXSJD sjd, Sint32 ch);
void *ADXSJD_GetSpsdInfo(ADXSJD sjd);
void ADXSJD_TakeSnapshot(ADXSJD sjd);
void ADXSJD_RestoreSnapshot(ADXSJD sjd);

#ifdef __cplusplus
}
#endif

#endif
