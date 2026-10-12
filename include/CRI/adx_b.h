#ifndef CRI_ADX_B_H
#define CRI_ADX_B_H

/* CRI ADXB: the ADX/WAV/AIFF/AU/AHX/SPSD block decoder object (adx_bsc.c, adx_b*.c) and the ADX
 * block expander handle it drives (adx_xpnd.c). Field names and offsets from the RE4 GameCube
 * decompilation; unknown fields are padded.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

typedef struct ADXPD_OBJ *ADXPD;

/* write-window callback: returns the PCM buffer, *pos the write position in it, *nsmpl the samples
 * that fit, *x70 the samples left before the trap point */
typedef Sint16 *(*ADXB_GETWR_FUNC)(void *obj, Sint32 *pos, Sint32 *nsmpl, Sint32 *x70);

/* ADPCM block decoder object (adx_xpnd.c) */
/* ADXPD_OBJ.stat */
#define ADXPD_STAT_STOP 0
#define ADXPD_STAT_START 1
#define ADXPD_STAT_EXEC 2
#define ADXPD_STAT_DONE 3

/* ADXPD_OBJ.mode */
#define ADXPD_MODE_MONO 1
#define ADXPD_MODE_STE 2

typedef struct ADXPD_OBJ {
    Sint32 used;      /* 0x00 */
    Sint32 id;        /* 0x04 */
    Sint32 x08;       /* 0x08 */
    Sint32 stat;      /* 0x0C */
    Sint32 nblk;      /* 0x10 decoded blocks */
    Sint32 mode;      /* 0x14 */
    void *in;         /* 0x18 */
    Sint32 nblk_in;   /* 0x1C */
    Sint16 *out0;     /* 0x20 */
    Sint16 *out1;     /* 0x24 */
    Sint16 dly[2][2]; /* 0x28 */
    Sint16 k1;        /* 0x30 */
    Sint16 k2;        /* 0x32 */
    Sint16 ext1;      /* 0x34 */
    Sint16 ext2;      /* 0x36 */
    Sint16 ext3;      /* 0x38 */
    Sint16 pad3a;
} ADXPD_OBJ;

/* ADXB_OBJ.stat */
#define ADXB_STAT_STOP 0
#define ADXB_STAT_DECODE 1
#define ADXB_STAT_WRITE 2
#define ADXB_STAT_DONE 3

/* container type ids stored in x98 and returned by ADXB_GetFormat */
#define ADXB_TYPE_ADX 0
#define ADXB_TYPE_WAV 1
#define ADXB_TYPE_SPSD 2
#define ADXB_TYPE_AIFF 3
#define ADXB_TYPE_AU 4
#define ADXB_TYPE_AHX 10

/* ADX header format field values (not the container type above) */
#define ADXB_FMT_ADX 0
#define ADXB_FMT_AHX 1
#define ADXB_FMT_PCM16 2
#define ADXB_FMT_PCM8 3
#define ADXB_FMT_SPSD 4

typedef struct {
    Sint16 used;        /* 0x00 */
    Sint16 x02;         /* 0x02 */
    Sint32 stat;        /* 0x04 */
    ADXPD pd;           /* 0x08 */
    Sint8 x0c;          /* 0x0C */
    Sint8 bps;          /* 0x0D bits per sample */
    Sint8 nch;          /* 0x0E */
    Sint8 x0f;          /* 0x0F */
    Sint32 fmt;         /* 0x10 */
    Sint32 sfreq;       /* 0x14 */
    Sint32 total_nsmpl; /* 0x18 */
    Sint16 x1c;         /* 0x1C */
    Uint8 pad1e[2];
    Sint32 x20;                                                /* 0x20 */
    Sint16 x24;                                                /* 0x24 */
    Sint16 x26;                                                /* 0x26 */
    Sint32 x28;                                                /* 0x28 */
    Sint32 x2c;                                                /* 0x2C */
    Sint32 x30;                                                /* 0x30 */
    Sint32 x34;                                                /* 0x34 */
    Sint32 x38;                                                /* 0x38 */
    Sint32 x3c;                                                /* 0x3C */
    Sint32 x40;                                                /* 0x40 */
    Sint32 x44;                                                /* 0x44 */
    Sint16 *inbuf;                                             /* 0x48 */
    Sint32 inbuf_nsmpl;                                        /* 0x4C */
    Sint32 out_nch;                                            /* 0x50 */
    Sint32 x54;                                                /* 0x54 */
    Sint32 out_fmt;                                            /* 0x58 */
    Sint16 *pcmbuf;                                            /* 0x5C */
    Sint32 pcmbuf_nsmpl;                                       /* 0x60 */
    Sint32 pcmbuf_chofst;                                      /* 0x64 */
    Sint32 wr_pos;                                             /* 0x68 */
    Sint32 wr_nsmpl;                                           /* 0x6C */
    Sint32 wr_x70;                                             /* 0x70 */
    Sint32 x74;                                                /* 0x74 */
    ADXB_GETWR_FUNC getwr_func;                                /* 0x78 */
    void *getwr_obj;                                           /* 0x7C */
    void (*addwr_func)(void *obj, Sint32 nbyte, Sint32 nsmpl); /* 0x80 */
    void *addwr_obj;                                           /* 0x84 */
    Sint32 x88;                                                /* 0x88 */
    Sint32 x8c;                                                /* 0x8C */
    Sint32 dec_nsmpl;                                          /* 0x90 */
    Sint32 dec_nbyte;                                          /* 0x94 */
    Sint16 x98;                                                /* 0x98 */
    Sint16 x9a;                                                /* 0x9A */
    Sint16 x9c;                                                /* 0x9C */
    Uint8 pad9e[2];
    Sint16 key[3];     /* 0xA0 encryption keys (k0, km, ka) */
    Sint16 extprm[3];  /* 0xA6 decoder extension parameters */
    Sint16 dly[2][2];  /* 0xAC decoder history (snapshot) */
    void *ahx;         /* 0xB4 */
    Sint32 nsmpl;      /* 0xB8 */
    Sint32 nsmpl96;    /* 0xBC */
    Sint32 ainf_len;   /* 0xC0 */
    Uint8 ainf[16];    /* 0xC4 */
    Sint16 def_outvol; /* 0xD4 */
    Sint16 def_pan[2]; /* 0xD6 */
    Uint8 padda[2];
    void *xdc; /* 0xDC (adx_sjd: pl2setsfreqfunc called when set) */
    Uint8 pade0[8];
    Sint32 cb_nbyte; /* 0xE8 dec_nbyte at the last callback */
    Sint32 xec;      /* 0xEC */
    void (*cb_func)(void *obj, Sint32 nbyte, Sint32 nsmpl); /* 0xF0 decode callback */
    void *cb_obj;                                           /* 0xF4 */
} ADXB_OBJ;

#define ADXB_MAX_OBJ 16

typedef ADXB_OBJ *ADXB;

#ifdef __cplusplus
extern "C" {
#endif

/* adx_xpnd.c */
void ADXPD_ExecHndl(ADXPD pd);
Sint32 ADXPD_GetNumBlk(ADXPD pd);
void ADXPD_Reset(ADXPD pd);
void ADXPD_Stop(ADXPD pd);
void ADXPD_Start(ADXPD pd);
Sint32 ADXPD_EntryPl2(ADXPD pd, void *in, Sint32 nblk, Sint16 *out0, Sint16 *out1);
Sint32 ADXPD_EntrySte(ADXPD pd, void *in, Sint32 nblk, Sint16 *out0, Sint16 *out1);
Sint32 ADXPD_EntryMono(ADXPD pd, void *in, Sint32 nblk, Sint16 *out0, Sint16 *out1);
Sint32 ADXPD_GetStat(ADXPD pd);
void ADXPD_Destroy(ADXPD pd);
void ADXPD_GetExtPrm(ADXPD pd, Sint16 *e1, Sint16 *e2, Sint16 *e3);
void ADXPD_SetExtPrm(ADXPD pd, Sint16 e1, Sint16 e2, Sint16 e3);
void ADXPD_GetDly(ADXPD pd, Sint16 *d0, Sint16 *d1);
void ADXPD_SetDly(ADXPD pd, Sint16 *d0, Sint16 *d1);
void ADXPD_SetCoef(ADXPD pd, Sint32 sfreq, Sint32 cutoff);
ADXPD ADXPD_Create(void);
void ADXPD_Init(void);

/* adx_bsc.c */
void ADXB_SetAhxExtFunc(void (*func)(void *ahx, Sint16 *key));
void ADXB_SetPl2Func(void (*encode)(ADXB adxb, Sint16 smpl, Sint16 *in, Sint16 *out),
                     void (*reset)(ADXB adxb));
void ADXB_SetDefKey(Sint16 k0, Sint16 km, Sint16 ka);
void ADXB_SetIgnoreHdrErr(Sint32 flg);
void ADXB_ExecHndl(ADXB adxb);
void ADXB_ExecOneAdx(ADXB adxb);
void ADXB_EvokeDecode(ADXB adxb);
Sint32 ADXB_GetDecNumSmpl(ADXB adxb);
Sint32 ADXB_GetDecDtLen(ADXB adxb);
void ADXB_Reset(ADXB adxb);
void ADXB_Stop(ADXB adxb);
void ADXB_Start(ADXB adxb);
void ADXB_EntryData(ADXB adxb, Sint16 *buf, Sint32 nbyte);
Sint32 ADXB_GetStat(ADXB adxb);
void ADXB_RestoreSnapshot(ADXB adxb);
void ADXB_TakeSnapshot(ADXB adxb);
Sint16 ADXB_GetDefPan(ADXB adxb, Sint32 ch);
Sint16 ADXB_GetDefOutVol(ADXB adxb);
Sint32 ADXB_GetAinfLen(ADXB adxb);
Sint32 ADXB_GetLpEndOfst(ADXB adxb);
Sint32 ADXB_GetLpEndPos(ADXB adxb);
Sint32 ADXB_GetLpStartOfst(ADXB adxb);
Sint32 ADXB_GetLpStartPos(ADXB adxb);
Sint32 ADXB_GetNumLoop(ADXB adxb);
Sint32 ADXB_GetTotalNumSmpl(ADXB adxb);
Sint32 ADXB_GetBlkSmpl(ADXB adxb);
Sint32 ADXB_GetOutBps(ADXB adxb);
Sint32 ADXB_GetNumChan(ADXB adxb);
Sint32 ADXB_GetSfreq(ADXB adxb);
Sint32 ADXB_GetFormat(ADXB adxb);
Sint16 *ADXB_GetPcmBuf(ADXB adxb);
void ADXB_EntryGetWrFunc(ADXB adxb, ADXB_GETWR_FUNC func, void *obj);
Sint32 ADXB_DecodeHeader(ADXB adxb, void *buf, Sint32 bsize);
void ADXB_SetDefPrm(ADXB adxb);
Sint32 ADXB_DecodeHeaderAdx(ADXB adxb, Uint8 *buf, Sint32 bsize);
const Char8 *ADXB_GetSignature(void);
void ADXB_Destroy(ADXB adxb);
ADXB ADXB_Create(Sint32 x38, Sint16 *pcmbuf, Sint32 bufsmpl, Sint32 chofst);
void ADXB_Init(void);
Sint32 ADXB_GetIgnoreHdrErr(void);

/* adx_bahx.c */
void ADXB_AhxTermSupply(ADXB adxb);
void ADXB_ExecOneAhx(ADXB adxb);
void ADXB_SetAhxDecSmpl(ADXB adxb, Sint32 nsmpl);
void ADXB_SetAhxInSj(ADXB adxb, void *sji);

/* adx_bwav.c, adx_baif.c, adx_bau.c */
void ADXB_ExecOneWav(ADXB adxb);
Sint32 ADXB_CheckWav(Uint8 *buf);
Sint32 ADXB_DecodeHeaderWav(ADXB adxb, void *buf, Sint32 bsize);
void ADXB_ExecOneAiff(ADXB adxb);
Sint32 ADXB_CheckAiff(Uint8 *buf);
Sint32 ADXB_DecodeHeaderAiff(ADXB adxb, void *buf, Sint32 bsize);
void ADXB_ExecOneAu(ADXB adxb);
Sint32 ADXB_CheckAu(Uint8 *buf);
Sint32 ADXB_DecodeHeaderAu(ADXB adxb, void *buf, Sint32 bsize);

/* adx_bsps.c */
Sint32 ADXB_CheckSpsd(void *buf);
void ADXB_ExecOneSpsd(ADXB adxb);
Sint32 ADXB_DecodeHeaderSpsd(ADXB adxb, void *buf, Sint32 bsize);
Sint32 ADX_DecodeInfoSpsd(Uint8 *buf, Sint32 bsize, Sint16 *hdrlen, Sint8 *x0c, Sint8 *bps,
                          Sint8 *x0f, Sint8 *nch, Sint32 *sfreq, Sint32 *nsmpl, Sint32 *fmt,
                          Sint16 *x9c);

#ifdef __cplusplus
}
#endif

/* byte-order helpers of the WAV/AIFF/AU header parsers (adx_bwav.c, adx_baif.c, adx_bau.c) */
#define LE16(p) (((Uint16)(p)[1] << 8) | (p)[0])
#define LE32(p)                                                                                    \
    ((Uint32)(p)[0] | ((Uint32)(p)[1] << 8) | ((Uint32)(p)[2] << 16) | ((Uint32)(p)[3] << 24))
#define SWAP16(x) ((((x) >> 8) & 0xFF) | (((x) & 0xFF) << 8))
#define SWAP32(x)                                                                                  \
    ((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x) << 24))

#endif
