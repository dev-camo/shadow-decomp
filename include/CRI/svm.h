#ifndef CRI_SVM_H
#define CRI_SVM_H

/* CRI server manager (svm.c) public interface. */
#include "CRI/cri_xpt.h"

/* server types (svtype) */
#define SVM_SVTYPE_VSYNC 2
#define SVM_SVTYPE_FS 4
#define SVM_SVTYPE_MAIN 5
#define SVM_SVTYPE_MWIDLE 6

#ifdef __cplusplus
extern "C" {
#endif

Sint32 SVM_TestAndSet(Sint32 *flag);
void SVM_Init(void);
void SVM_Finish(void);
Sint32 SVM_ExecSvrMwIdle(void);
Sint32 SVM_ExecSvrMain(void);
Sint32 SVM_ExecSvrFs(void);
Sint32 SVM_ExecSvrVsync(void);
void SVM_SetCbUnlock(void (*func)(void *obj), void *obj);
void SVM_SetCbLock(void (*func)(void *obj), void *obj);
void SVM_GotoSvrBorder(Sint32 svtype);
void SVM_SetCbBdr(Sint32 svtype, void (*func)(void *obj), void *obj);
void SVM_SetCbSvrId(Sint32 svtype, Sint32 id, Sint32 (*func)(void *obj), void *obj,
                    const Char8 *name);
void SVM_DelCbSvr(Sint32 svtype, Sint32 id);
Sint32 SVM_SetCbSvr(Sint32 svtype, Sint32 (*func)(void *obj), void *obj, const Char8 *name);
void SVM_CallErr1(Char8 *msg);
void SVM_CallErr(Char8 *fmt, ...);
void SVM_Unlock(void);
void SVM_Lock(void);

#ifdef __cplusplus
}
#endif

#endif
