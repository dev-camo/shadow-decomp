#ifndef CRI_SJ_H
#define CRI_SJ_H

/* CRI stream joint (SJ) interface: a handle points to an object whose first word is its
 * interface table. Layout from sj_mem.c/sj_rbf.c and the RE4 GameCube decompilation.
 */
#include "CRI/cri_xpt.h"
#include "layout_check.h"

typedef struct SJ_OBJ *SJ;

typedef struct {
    Uint8 *data; /* 0x00 */
    Sint32 len;  /* 0x04 */
} SJCK;
CHECK_SIZEOF(SJCK, 0x8);

typedef struct {
    Uint8 data[16]; /* 0x00 */
} SJUUID;
CHECK_SIZEOF(SJUUID, 0x10);

/* chunk ids */
#define SJ_CK_FREE 0
#define SJ_CK_DATA 1

/* error code passed to the error callback in place of the message */
#define SJ_ERR_PRM ((Char8 *)-3)

typedef struct {
    void *QueryInterface;                                                        /* 0x00 */
    void *AddRef;                                                                /* 0x04 */
    void *Release;                                                               /* 0x08 */
    void (*Destroy)(SJ sj);                                                      /* 0x0C */
    const SJUUID *(*GetUuid)(SJ sj);                                             /* 0x10 */
    void (*Reset)(SJ sj);                                                        /* 0x14 */
    void (*GetChunk)(SJ sj, Sint32 id, Sint32 nbyte, SJCK *ck);                  /* 0x18 */
    void (*UngetChunk)(SJ sj, Sint32 id, SJCK *ck);                              /* 0x1C */
    void (*PutChunk)(SJ sj, Sint32 id, SJCK *ck);                                /* 0x20 */
    Sint32 (*GetNumData)(SJ sj, Sint32 id);                                      /* 0x24 */
    Sint32 (*IsGetChunk)(SJ sj, Sint32 id, Sint32 nbyte, Sint32 *rbyte);         /* 0x28 */
    void (*EntryErrFunc)(SJ sj, void (*func)(void *obj, Char8 *msg), void *obj); /* 0x2C */
} SJ_IF;
CHECK_SIZEOF(SJ_IF, 0x30);

struct SJ_OBJ {
    SJ_IF *vtbl; /* 0x00 */
};
CHECK_SIZEOF(struct SJ_OBJ, 0x4);

#define SJ_Destroy(sj) (*(sj)->vtbl->Destroy)(sj)
#define SJ_Reset(sj) (*(sj)->vtbl->Reset)(sj)
#define SJ_GetChunk(sj, id, nbyte, ck) (*(sj)->vtbl->GetChunk)((sj), (id), (nbyte), (ck))
#define SJ_PutChunk(sj, id, ck) (*(sj)->vtbl->PutChunk)((sj), (id), (ck))
#define SJ_UngetChunk(sj, id, ck) (*(sj)->vtbl->UngetChunk)((sj), (id), (ck))
#define SJ_GetNumData(sj, id) (*(sj)->vtbl->GetNumData)((sj), (id))
#define SJ_GetUuid(sj) (*(sj)->vtbl->GetUuid)(sj)

#ifdef __cplusplus
extern "C" {
#endif

/* sj_utl.c */
void SJ_SplitChunk(SJCK *ck, Sint32 nbyte, SJCK *ck1, SJCK *ck2);
Sint8 *SJ_SearchTag(SJCK *cki, Char8 *kw, Char8 *endkw, SJCK *cko);

/* sj_crs.c */
void SJCRS_Lock(void);
void SJCRS_Unlock(void);
void SJCRS_Init(void);
void SJCRS_Finish(void);

/* sj_mem.c */
SJ SJMEM_Create(void *buf, Sint32 bsize);
Sint32 SJMEM_GetBufSize(SJ handle);
void SJMEM_Init(void);
void SJMEM_Finish(void);

/* sj_rbf.c */
SJ SJRBF_Create(void *buf, Sint32 bsize, Sint32 xsize);
Sint32 SJRBF_GetFlowCnt(SJ handle, Sint32 id, Sint32 dir);
Sint32 SJRBF_GetXtrSize(SJ handle);
Sint32 SJRBF_GetBufSize(SJ handle);
void *SJRBF_GetBufPtr(SJ handle);
void SJRBF_Init(void);
void SJRBF_Finish(void);

/* sj_err.c */
void SJERR_CallErr(Char8 *msg);

/* sj_uni.c */
void SJUNI_Init(void);
void SJUNI_Finish(void);

#ifdef __cplusplus
}
#endif

#endif
