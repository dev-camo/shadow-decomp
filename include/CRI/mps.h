#ifndef CRI_MPS_H
#define CRI_MPS_H

/* CRI Sofdec MPEG system-stream demultiplexer (MPS) object layout. Layout and names from the RE4
 * GameCube decompilation.
 */

#include "CRI/cri_xpt.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MPS_OBJ *MPS;

/* The MPS records are 4-byte aligned in the original: SFMPS_Init hands MPS_Init a work buffer at an
 * address that is 4 modulo 8 (sfmps_libwork), and the 64-bit fields are reached with word accesses.
 * The packing keeps every size and offset. */
#pragma pack(push, 4)

/* pack header (16 bytes: holds the 64-bit SCR) */
typedef struct {
    Sint64 scr;      /* 0x00 */
    Sint32 rsv;      /* 0x08 */
    Sint32 mux_rate; /* 0x0C 22-bit pack-header mux rate in 50-byte/s units (-1: none seen) */
} MPS_PACKHD;
CHECK_SIZEOF(MPS_PACKHD, 0x10);

/* system header (32 bytes) */
typedef struct {
    Sint32 raw[8]; /* 0x00 */
} MPS_SYSHD;
CHECK_SIZEOF(MPS_SYSHD, 0x20);

/* packet header (40 bytes: holds the 64-bit PTS/DTS) */
typedef struct {
    Sint64 pts;    /* 0x00 */
    Sint64 dts;    /* 0x08 */
    Sint32 raw[6]; /* 0x10 */
} MPS_PKETHD;
CHECK_SIZEOF(MPS_PKETHD, 0x28);

/* MPS_OBJ.used */
#define MPS_HN_FREE 1
#define MPS_HN_USED 2

typedef struct MPS_OBJ {
    Sint32 used;                           /* 0x00 */
    void (*errfn)(void *obj, Sint32 code); /* 0x04 */
    void *errobj;                          /* 0x08 */
    Sint32 errcode;                        /* 0x0C */
    Sint32 x10;                            /* 0x10 (2) */
    Sint32 x14;                            /* 0x14 */
    MPS_PACKHD packhd;                     /* 0x18 */
    MPS_SYSHD last_syshd;                  /* 0x28 */
    MPS_SYSHD syshd[3];                    /* 0x48 */
    MPS_PKETHD pkethd;                     /* 0xA8 */
    Sint32 xd0;                            /* 0xD0 */
    /* 0xD4 header decoder (MPSDEC_DecHdMpeg1) */
    Sint32 (*dechd_func)(struct MPS_OBJ *mps, Uint8 *adr, Sint32 len, Sint32 *used, Sint32 *flags);
    Sint32 xd8;     /* 0xD8 */
    Sint32 xdc;     /* 0xDC */
    Sint32 xe0;     /* 0xE0 */
    void *sysfn;    /* 0xE4 system header callback (MPS_SetSystemFn) */
    void *sysobj;   /* 0xE8 */
    void *psmapfn;  /* 0xEC (MPS_SetPsMapFn) */
    void *psmapobj; /* 0xF0 */
    void *pesfn;    /* 0xF4 PES header callback (MPS_SetPesFn) */
    void *pesobj;   /* 0xF8 */
    Sint32 xfc;     /* 0xFC */
} MPS_OBJ;
CHECK_SIZEOF(MPS_OBJ, 0x100);

/* MPS_PKETHD.raw */
#define MPS_PKT_STMID 0
#define MPS_PKT_TYPE 1
#define MPS_PKT_IDX 2
#define MPS_PKT_LEN 3
#define MPS_PKT_STDBUF 4
#define MPS_PKT_PAYLOAD 5

/* start codes as MPS_CheckDelim classifies them */
#define MPS_DELIM_PACK 0x10000
#define MPS_DELIM_SYSHD 0x20000
#define MPS_DELIM_PKET 0x40000
#define MPS_DELIM_END 0x80000

/* library work (mps_lib.c): header + num_hn handles */
typedef struct {
    void (*errfn)(void *obj, Sint32 code); /* 0x00 */
    void *errobj;                          /* 0x04 */
    Sint32 errcode;                        /* 0x08 */
    Sint32 num_hn;                         /* 0x0C */
    MPS_OBJ hn[1];                         /* 0x10 */
} MPSLIB_WORK;
CHECK_SIZEOF(MPSLIB_WORK, 0x110);

#pragma pack(pop)

extern MPSLIB_WORK *MPSLIB_libwork;

Sint32 MPSLIB_CheckHn(MPS mps);
Sint32 MPSLIB_SetErr(MPS mps, Sint32 code);
MPS MPS_Create(void);
Sint32 MPS_Destroy(MPS mps);
Uint32 MPS_CheckDelim(Uint8 *p);
Sint32 MPS_DecHd(MPS mps, Uint8 *adr, Sint32 len, Sint32 *used, Sint32 *flags);
Sint32 MPS_GetPackHd(MPS mps, MPS_PACKHD *hd);
Sint32 MPS_GetSysHd(MPS mps, MPS_SYSHD *hd, Sint32 no);
Sint32 MPS_GetLastSysHd(MPS mps, MPS_SYSHD *hd);
Sint32 MPS_GetPketHd(MPS mps, MPS_PKETHD *hd);
void MPS_SetPesFn(MPS mps, void *fn, void *obj);
void MPS_SetPsMapFn(MPS mps, void *fn, void *obj);
void MPS_SetSystemFn(MPS mps, void *fn, void *obj);

Sint32 MPS_Init(Sint32 num_hn, void *work);
void MPS_Finish(void);
Sint32 MPS_SetErrFn(MPS mps, void (*fn)(void *obj, Sint32 code), void *obj);

/* mps_dec.c */
Sint32 MPSDEC_DecHdMpeg1(MPS mps, Uint8 *adr, Sint32 len, Sint32 *used, Sint32 *flags);
void MPSDEC_Init(void);
void MPSDEC_Finish(void);

/* mps_get.c */
void MPSGET_Init(void);
void MPSGET_Finish(void);

#ifdef __cplusplus
}
#endif

#endif
