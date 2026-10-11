#ifndef CRI_SJ_H
#define CRI_SJ_H

/* CRI stream joint (SJ) interface: a handle points to an object whose first word is its
 * interface table. Layout from sj_mem.c/sj_rbf.c and the RE4 GameCube decompilation.
 */
#include "CRI/cri_xpt.h"

typedef struct SJ_OBJ *SJ;

typedef struct {
    Uint8 *data;
    Sint32 len;
} SJCK;

typedef struct {
    Uint8 data[16];
} SJUUID;

/* chunk ids */
#define SJ_CK_FREE 0
#define SJ_CK_DATA 1

/* error code passed to the error callback in place of the message */
#define SJ_ERR_PRM ((Char8 *)-3)

typedef struct {
    void *QueryInterface;
    void *AddRef;
    void *Release;
    void (*Destroy)(SJ sj);
    const SJUUID *(*GetUuid)(SJ sj);
    void (*Reset)(SJ sj);
    void (*GetChunk)(SJ sj, Sint32 id, Sint32 nbyte, SJCK *ck);
    void (*UngetChunk)(SJ sj, Sint32 id, SJCK *ck);
    void (*PutChunk)(SJ sj, Sint32 id, SJCK *ck);
    Sint32 (*GetNumData)(SJ sj, Sint32 id);
    Sint32 (*IsGetChunk)(SJ sj, Sint32 id, Sint32 nbyte, Sint32 *rbyte);
    void (*EntryErrFunc)(SJ sj, void (*func)(void *obj, Char8 *msg), void *obj);
} SJ_IF;

struct SJ_OBJ {
    SJ_IF *vtbl;
};

#define SJ_Destroy(sj) (*(sj)->vtbl->Destroy)(sj)
#define SJ_Reset(sj) (*(sj)->vtbl->Reset)(sj)
#define SJ_GetChunk(sj, id, nbyte, ck) (*(sj)->vtbl->GetChunk)((sj), (id), (nbyte), (ck))
#define SJ_PutChunk(sj, id, ck) (*(sj)->vtbl->PutChunk)((sj), (id), (ck))
#define SJ_UngetChunk(sj, id, ck) (*(sj)->vtbl->UngetChunk)((sj), (id), (ck))
#define SJ_GetNumData(sj, id) (*(sj)->vtbl->GetNumData)((sj), (id))

#ifdef __cplusplus
extern "C" {
#endif

/* sj_crs.c */
void SJCRS_Lock(void);
void SJCRS_Unlock(void);
void SJCRS_Init(void);
void SJCRS_Finish(void);

/* sj_mem.c */
SJ SJMEM_Create(void *buf, Sint32 bsize);
void SJMEM_Init(void);
void SJMEM_Finish(void);

/* sj_rbf.c */
SJ SJRBF_Create(void *buf, Sint32 bsize, Sint32 xsize);
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
