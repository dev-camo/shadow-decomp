#ifndef CRI_MPV_H
#define CRI_MPV_H

/* CRI Sofdec MPEG video decoder (MPV) object layout. Names follow the RE4 GameCube decompilation
 * of MPV 1.933; offsets are this game's MPV 1.958 layout, measured from its MPV code (one more
 * IDCT function pointer at 0x2E8, the motion compensation buffer starting at 0x3A0, and three
 * words after the user data fields).
 */

#include "CRI/cri_xpt.h"
#include "CRI/dct.h"
#include "CRI/sj.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *p;  /* 0x00 */
    Sint32 n; /* 0x04 */
} MPVCMC_REF;
CHECK_SIZEOF(MPVCMC_REF, 0x8);

/* an output block table: chroma block count (-1: none) and the six block descriptors */
typedef struct {
    Sint32 ccnt;      /* 0x00 */
    MPVCMC_REF oi[6]; /* 0x04 */
} MPVCMC_OUTBLK;
CHECK_SIZEOF(MPVCMC_OUTBLK, 0x34);

/* error record (MPVERR_InitErrInf clears all five words); in a handle's record the last two words
 * are counters of the header decoder, which MPV_DecodeFrmSj reports per call: x0c counts finished
 * pictures and pictures whose slice loop ends with mb_addr != mb_last, and has the handle's x13a0
 * added after each picture; x10 counts finished pictures while cond[1] is set */
typedef struct {
    void (*func)(void *obj, Sint32 code); /* 0x00 */
    void *obj;                            /* 0x04 */
    Sint32 code;                          /* 0x08 */
    Sint32 x0c;                           /* 0x0C */
    Sint32 x10;                           /* 0x10 */
} MPVERR_INF;
CHECK_SIZEOF(MPVERR_INF, 0x14);

/* picture attributes (0x80 bytes; MPV_OBJ + 0x1D0, copied out by MPV_GetPicAtr). x68 is set by
 * each picture header from the handle's counters: x1398 << 16 for I and P pictures,
 * (x1398 - 1) << 16 | x139c for other pictures; 0 after a GOP or sequence header. */
typedef struct {
    Sint32 width;             /* 0x00 horizontal_size */
    Sint32 height;            /* 0x04 vertical_size */
    Sint32 mb_width;          /* 0x08 */
    Sint32 mb_height;         /* 0x0C */
    Sint32 frame_rate;        /* 0x10 picture_rate code */
    Sint32 temp_ref;          /* 0x14 temporal_reference */
    Sint32 pic_type;          /* 0x18 picture_coding_type */
    Sint32 tc_drop;           /* 0x1C GOP time code: drop_frame_flag */
    Sint32 tc_hour;           /* 0x20 */
    Sint32 tc_min;            /* 0x24 */
    Sint32 tc_sec;            /* 0x28 */
    Sint32 tc_pic;            /* 0x2C */
    Sint32 ngop;              /* 0x30 GOP headers seen */
    Sint32 nseq;              /* 0x34 sequence headers seen */
    Sint32 x38;               /* 0x38 */
    Sint32 x3c;               /* 0x3C */
    Sint32 x40;               /* 0x40 */
    Sint32 x44;               /* 0x44 */
    Sint32 bitrate;           /* 0x48 */
    Sint32 vbv_size;          /* 0x4C */
    Sint16 x50;               /* 0x50 */
    Sint16 x52;               /* 0x52 */
    Sint8 x54;                /* 0x54 */
    Sint8 x55;                /* 0x55 */
    Sint8 x56;                /* 0x56 */
    Sint8 x57;                /* 0x57 */
    Sint8 x58;                /* 0x58 */
    Uint8 aspect;             /* 0x59 pel_aspect_ratio */
    Uint8 constrained;        /* 0x5A constrained_parameters_flag */
    Uint8 x5b;                /* 0x5B */
    Uint8 x5c;                /* 0x5C */
    Uint8 x5d;                /* 0x5D */
    Sint8 x5e;                /* 0x5E */
    Sint8 x5f;                /* 0x5F */
    Sint8 x60;                /* 0x60 */
    Uint8 x61;                /* 0x61 */
    Uint8 x62;                /* 0x62 */
    Uint8 x63;                /* 0x63 */
    Uint8 x64;                /* 0x64 */
    Uint8 pad65[3];           /* 0x65 */
    Sint32 x68;               /* 0x68 picture number (see above) */
    Uint8 pad6C[0x80 - 0x6C]; /* 0x6C */
} MPV_PICATR;
CHECK_SIZEOF(MPV_PICATR, 0x80);

/* a frame's planes as mpv_umc.c addresses them: the two chroma planes, then luma, and the chroma
 * and luma pitches in bytes (name inferred) */
typedef struct {
    Uint8 *pln[3]; /* 0x00 */
    Sint16 cpitch; /* 0x0C */
    Sint16 ypitch; /* 0x0E */
} MPV_RFB;
CHECK_SIZEOF(MPV_RFB, 0x10);

/* frame parameter block passed to MPV_DecodeFrmSj (0x30 bytes). The forward-predicted macroblocks
 * read rfb[0], the backward-predicted ones rfb[1]; skipped P picture macroblocks are copied from
 * rfb[0] to rfb[1]. */
typedef struct {
    MPV_RFB rfb[2];     /* 0x00 */
    Uint8 *buf;         /* 0x20 output frame (MPVUMC_InitOutRfb lays out MPV_OBJ.out in it) */
    MPV_PICATR *picatr; /* 0x24 */
    Sint32 x28;         /* 0x28 MPVERR_INF.x0c counted during the call */
    Sint32 x2c;         /* 0x2C MPVERR_INF.x10 counted during the call */
} MPV_FRM;
CHECK_SIZEOF(MPV_FRM, 0x30);

/* run/level VLC table selector (mpv_bdec.c) */
typedef struct {
    Uint32 *tbl; /* 0x00 */
    Sint32 bits; /* 0x04 */
} MPV_RUNLEVEL;
CHECK_SIZEOF(MPV_RUNLEVEL, 0x8);

typedef struct MPV_OBJ *MPV;

/* buffers of the motion compensation work (mpvlib_InitHn; name inferred) */
typedef struct {
    Uint8 *clip;  /* 0x00 clip table zero point */
    Sint16 *buf0; /* 0x04 the handle's mcbuf (IDCT output) */
    Uint8 *work;  /* 0x08 the handle's work area (prediction) */
    Uint8 *work2; /* 0x0C work + 0x180 (second prediction) */
} MPVMC_WORK;
CHECK_SIZEOF(MPVMC_WORK, 0x10);

/* motion compensation work (MPV_OBJ + 0xCC, 0x54 bytes; mpv_mc.c / mpv_mcy.c / mpv_umc.c) */
typedef struct MPVMC {
    void (*oneref08[4])(struct MPVMC *mc); /* 0x00 8x8 block: 1p, H2, V2, 4p */
    Uint8 pad10[8];                        /* 0x10 */
    Uint32 *dst;                           /* 0x18 */
    Uint8 pad1C[4];                        /* 0x1C */
    Sint32 stride;                         /* 0x20 */
    Uint8 *src;                            /* 0x24 */
    Uint8 *src2;                           /* 0x28 second reference row / block */
    Uint8 pad2C[8];                        /* 0x2C */
    void (*oneref16[4])(struct MPVMC *mc); /* 0x34 16x16 block */
    MPVMC_WORK wk;                         /* 0x44 */
} MPVMC;
CHECK_SIZEOF(MPVMC, 0x54);

/* block decoder parameters (mpv_cdec.c -> intra/nintra block decode function; the first six are
 * the run/level decode state of mpvabdec.c) */
typedef struct {
    Sint32 run;     /* 0x00 */
    Sint32 level;   /* 0x04 */
    Sint32 sign;    /* 0x08 */
    Sint32 len;     /* 0x0C code length in bits */
    Sint32 idx0;    /* 0x10 zigzag position of the first coefficient */
    Sint32 idx;     /* 0x14 zigzag position of the last coefficient */
    Sint32 pad18;   /* 0x18 */
    Float32 *dst;   /* 0x1C block coefficient buffer (64 coefficients) */
    Uint8 *iqm;     /* 0x20 quantiser matrix */
    Sint32 qscale;  /* 0x24 */
    Sint32 *dcpred; /* 0x28 intra DC predictor of the component */
    Sint8 *dctbl;   /* 0x2C intra DC size table (entries read unsigned) */
    Sint32 nintra;  /* 0x30 */
} MPV_BLKPRM;
CHECK_SIZEOF(MPV_BLKPRM, 0x34);

typedef Sint32 (*MPV_BLKDEC_FUNC)(MPV mpv, MPV_BLKPRM *prm);

/* motion vector parameters of one direction (mpv_hdec.c picture header, MPVDEC_ResetMv) */
typedef struct {
    Sint32 full_pel; /* 0x00 full_pel_*_vector */
    Sint32 r_size;   /* 0x04 f_code - 1 */
    Sint32 shift;    /* 0x08 27 - r_size */
    Sint32 f;        /* 0x0C 1 << r_size */
    Sint32 pred[2];  /* 0x10 predictors (h, v) */
    Sint32 vec[2];   /* 0x18 decoded vector (h, v) */
    Sint32 pad;      /* 0x20 */
} MPV_MV;
CHECK_SIZEOF(MPV_MV, 0x24);

/* user data stream joints (MPV_SetUsrSj) */
typedef struct {
    SJ sj;                              /* 0x00 */
    void (*func)(void *obj, Sint32 id); /* 0x04 */
    void *obj;                          /* 0x08 */
} MPV_USRSJ;
CHECK_SIZEOF(MPV_USRSJ, 0xC);

typedef struct MPV_OBJ {
    Uint32 bbuf;         /* 0x000 bit reader state saved around the block decoders */
    Uint32 nbuf;         /* 0x004 */
    Sint32 bitpos;       /* 0x008 */
    Uint32 *bitptr;      /* 0x00C */
    Uint32 *rl_8;        /* 0x010 run/level VLC tables (mpv_lib.c) */
    Uint32 *rl_4;        /* 0x014 */
    Uint32 *rl_2;        /* 0x018 */
    Uint32 *rl_1;        /* 0x01C */
    Uint32 *rl_0a;       /* 0x020 */
    Uint32 *rl_0b;       /* 0x024 */
    Uint32 *rl_0c;       /* 0x028 */
    Sint8 *zigzag_tbl;   /* 0x02C */
    Uint16 *bitmsk_tbl;  /* 0x030 */
    Float32 *scale_tbl;  /* 0x034 */
    Uint8 *group_tbl_p;  /* 0x038 */
    void *rl_p;          /* 0x03C */
    Uint8 *clip_base;    /* 0x040 */
    MPV_BLKPRM blkprm;   /* 0x044 */
    DCT_PA dctpa;        /* 0x078 */
    MPVMC mc;            /* 0x0CC */
    MPVCMC_OUTBLK ob_rt; /* 0x120 */
    MPVCMC_OUTBLK ob;    /* 0x154 */
    Sint32 stat;         /* 0x188 */
    Sint32 x18c;         /* 0x18C */
    Sint32 cond[16];     /* 0x190 [3] diagonal half-pel by V2 (mpv_umc), [5] chroma (mpv_cmc) */
    MPV_PICATR picatr;   /* 0x1D0 */
    MPVERR_INF errinf;   /* 0x250 */
    MPV_FRM frm;         /* 0x264 */
    MPV_RFB out;         /* 0x294 output frame planes (MPVUMC_InitOutRfb) */
    Sint32 aspect;       /* 0x2A4 pel_aspect_ratio */
    Sint32 bitrate;      /* 0x2A8 */
    Sint32 vbv_size;     /* 0x2AC */
    Sint32 constrained;  /* 0x2B0 constrained_parameters_flag */
    Sint32 linkflg1;     /* 0x2B4 closed_gop */
    Sint32 linkflg2;     /* 0x2B8 broken_link */
    Sint32 vbv_delay;    /* 0x2BC */
    Sint32 (*dec_mbs_func)(MPV mpv, SJ sj); /* 0x2C0 slice macroblock decoder of the picture type */
    void (*skip_func)(MPV mpv, Sint32 n);   /* 0x2C4 n: macroblock address increment */
    Sint32 (*intra_blocks)(MPV mpv);        /* 0x2C8 */
    Sint32 (*nintra_blocks)(MPV mpv);       /* 0x2CC */
    void (*mc_intra_func)(MPV mpv);         /* 0x2D0 */
    void (*mc_func[4])(MPV mpv); /* 0x2D4 [0] current, [1] backward, [2] forward, [3] both */
    void (*x2e4)(DCT_PA *pa);    /* 0x2E4 IDCT of an intra macroblock (DCT_FsriTrans6Blk) */
    void (*x2e8)(DCT_PA *pa);    /* 0x2E8 IDCT of the coded blocks (DCT_FsriTransCbp) */
    Sint32 qscale;               /* 0x2EC */
    MPV_MV fwd;                  /* 0x2F0 */
    MPV_MV bwd;                  /* 0x314 */
    Sint32 mb_addr;              /* 0x338 macroblock address */
    Sint32 mb_y;                 /* 0x33C */
    Sint32 mb_x;                 /* 0x340 */
    Sint32 mb_last;              /* 0x344 mb_width * mb_height - 1 */
    Sint32 mbtype; /* 0x348 macroblock_type flags (1 intra, 2 pattern, 4 bwd, 8 fwd, 0x10 quant) */
    Sint32 cbp_code;             /* 0x34C coded block pattern */
    Sint32 dcpred[3];            /* 0x350 */
    Sint32 m2v_mode;             /* 0x35C 0 unknown, 1 MPEG-1, 2 MPEG-2 */
    Sint32 stc[3];               /* 0x360 "STCCODE" user data values */
    Sint16 *tbl36c[6];           /* 0x36C IDCT output blocks in mcbuf */
    Uint8 pad384[0x3A0 - 0x384]; /* 0x384 */
    Sint16 mcbuf[6][64];         /* 0x3A0 six 8x8 IDCT output blocks */
    Float32 blk[6][64];          /* 0x6A0 coefficient blocks */
    Uint8 intra_iqm[64];         /* 0xCA0 */
    Uint8 nintra_iqm[64];        /* 0xCE0 */
    Uint8 work[0x1320 - 0xD20];  /* 0xD20 (MPVCMC_InitObj) */
    Sint32 pic_done;             /* 0x1320 set when a picture has been decoded */
    Sint32 npic;                 /* 0x1324 */
    SJCK ck;                     /* 0x1328 chunk being parsed */
    Sint32 bitofs;               /* 0x1330 bit offset of the slice data in its first byte */
    Sint32 dcprec;               /* 0x1334 "IDCPREC" (0: 8 bit, 3: 11 bit DC) */
    MPV_BLKDEC_FUNC intra_func;  /* 0x1338 */
    MPV_BLKDEC_FUNC nintra_func; /* 0x133C */
    Sint32 pad1340;              /* 0x1340 */
    Sint32 x1344;                /* 0x1344 copy of cond[7] */
    Sint8 *dctbl_y;              /* 0x1348 intra DC size tables */
    Sint8 *dctbl_c;              /* 0x134C */
    void *m2v;                   /* 0x1350 */
    Sint32 hdrtype;              /* 0x1354 header being decoded: 1 sequence, 2 GOP, 3 picture */
    MPV_USRSJ usr[4];            /* 0x1358 */
    Uint8 *picusr_buf;           /* 0x1388 */
    Sint32 picusr_bufsiz;        /* 0x138C */
    Sint32 picusr_len;           /* 0x1390 */
    Sint32 pad1394;              /* 0x1394 */
    Sint32 x1398;                /* 0x1398 I/P pictures since the last GOP or sequence header */
    Sint32 x139c;                /* 0x139C other pictures since then or the last I/P picture */
    Sint32 x13a0;                /* 0x13A0 */
} MPV_OBJ;
/* The library exports this size as mpvlib_siz_mpvobj, and MPV_Init passes it as an immediate to
 * MPVDEC_CheckVersion; handles are 0x13C0 apart. */
CHECK_SIZEOF(MPV_OBJ, 0x13A4);

/* shared table area behind the handle array (MPV_Init; mpvlib_siz_mpvixa, 0x1C60 bytes) */
typedef struct {
    Uint8 pad0[0x1100];         /* 0x0000 */
    Uint16 bitmsk[16];          /* 0x1100 */
    Sint8 zigzag[64];           /* 0x1120 */
    Float32 scale[64];          /* 0x1160 dequantisation scale per coefficient */
    Uint8 group_tbl[32];        /* 0x1260 */
    MPV_RUNLEVEL rl[6];         /* 0x1280 */
    Uint8 vlc[0x1860 - 0x12B0]; /* 0x12B0 VLC table area (MPVVLC_Init) */
    Uint8 clip[0x400];          /* 0x1860 copy of the clip table (MPV_Init) */
} MPV_IXA;
CHECK_SIZEOF(MPV_IXA, 0x1C60);

Sint8 *MPV_SearchDelim(Sint8 *p, Sint32 n, Sint32 mask);
/* mpv_vlc.c: VLC decode tables used by the slice decoder (mpv_dec.c) */
extern Sint16 *mpvvlc_mbai_i_0;
extern Sint16 *mpvvlc_mbai_i_1;
extern Sint16 *mpvvlc_mbai_p_0;
extern Sint16 *mpvvlc_mbai_p_1;
extern Sint16 *mpvvlc_mbai_b_0;
extern Sint16 *mpvvlc_mbai_b_1;
extern Sint16 *mpvvlc_p_mbtype;
extern Sint16 *mpvvlc_b_mbtype;
extern Sint16 *mpvvlc_cbp;
extern Sint16 *mpvvlc_motion_0;
extern Sint16 *mpvvlc_motion_1;
/* mpv_vlc.c: run/level decode tables used by the block decoder (mpv_bdec.c) */
extern Uint32 *mpvvlc_run_level_8;
extern Uint32 *mpvvlc_run_level_0a;
extern Uint32 *mpvvlc_run_level_0b;
extern Uint32 *mpvvlc_run_level_0c;
extern Uint32 *mpvvlc_run_level_1;
extern Uint32 *mpvvlc_run_level_2;
extern Uint32 *mpvvlc_run_level_4;
/* mpv_vlc.c: intra DC size tables (8 bit and 11 bit DC precision) */
extern Sint8 *mpvvlc_y_dcsiz;
extern Sint8 *mpvvlc_c_dcsiz;
extern Sint8 *mpvvlc2_y_dcsiz;
extern Sint8 *mpvvlc2_c_dcsiz;

Sint32 MPV_CheckDelim(void *ptr);
/* MPV_CheckDelim start code classes */
#define MPV_DLM_SLICE 0x01  /* any slice */
#define MPV_DLM_SLICE1 0x02 /* the first slice (0x101) */
#define MPV_DLM_PIC 0x04
#define MPV_DLM_GOP 0x08
#define MPV_DLM_EXT 0x10
#define MPV_DLM_UD 0x20
#define MPV_DLM_SEQ 0x40
#define MPV_DLM_END 0x80
Sint32 MPV_IsEmptyPpic(Sint8 *buf, Sint32 nbyte, Sint32 nmb);
Sint32 MPV_IsEmptyBpic(Sint8 *buf, Sint32 nbyte, Sint32 nmb);
Sint32 MPVLIB_CheckHn(MPV_OBJ *mpv);
Sint32 MPVERR_SetCode(MPV_OBJ *mpv, Sint32 code);

/* mpv_frm.c */
Sint32 MPV_SkipFrmSj(void *hn, SJ sj);
Sint32 MPV_DecodeFrmSj(void *hn, SJ sj, MPV_FRM *frm);
void MPVFRM_Init(void);

/* mpv_hdec.c */
void MPVHDEC_Init(void);
void MPV_SetUsrSj(MPV mpv, Sint32 id, SJ sj, void (*func)(void *obj, Sint32 id), void *obj);
void MPV_SetPicUsrBuf(MPV mpv, Uint8 *buf, Sint32 bufsiz);
void MPV_GetPicUsr(MPV mpv, Uint8 **buf, Sint32 *len);
Sint32 MPV_DecodePicAtrSj(void *hn, SJ sj);
Sint32 MPV_DecodePicAtr(MPV mpv, SJCK *ck, Sint32 *used);
Sint32 mpvhdec_DecShcSj(MPV mpv, SJ sj);
Sint32 mpvhdec_DecPscSj(MPV mpv, SJ sj);
Sint32 mpvhdec_AnalyUd(MPV mpv, Uint8 *buf, Sint32 len);
Sint32 mpvhdec_DecSeqUdsc(MPV mpv, Char8 *buf, Sint32 len);
Sint32 MPV_GoNextDelimSj(SJ sj);
Sint32 MPVHDEC_DecPicture(MPV mpv, SJ sj);
Sint32 MPV_MoveChunk(SJ sj, Sint32 id, Sint32 nbyte);

/* mpv_umc.c (not yet decompiled) */
void MPVUMC_Init(void);
void MPVUMC_Finish(void);
void MPVUMC_SetGqr(void);
void MPVUMC_InitOutRfb(MPV_OBJ *mpv);
void MPVUMC_EndOfFrame(MPV_OBJ *mpv);
void MPVUMC_BpicSkipped(MPV mpv, Sint32 n);
void MPVUMC_PpicSkipped(MPV mpv, Sint32 n);
void MPVUMC_BiDirect(MPV mpv);
void MPVUMC_Backward(MPV mpv);
void MPVUMC_Forward(MPV mpv);
void MPVUMC_Intra(MPV mpv);

/* mpv_dec.c */
Sint32 MPVDEC_DecDpicMb(MPV mpv, SJ sj);
Sint32 MPVDEC_DecBpicMb(MPV mpv, SJ sj);
void MPVDEC_ResetDc(MPV mpv);
void MPVDEC_ResetMv(MPV_MV *mv);
Sint32 MPVDEC_DecPpicMb(MPV mpv, SJ sj);
Sint32 MPVDEC_DecIpicMb(MPV mpv, SJ sj);
Sint32 MPVDEC_CheckVersion(const Char8 *ver, Sint32 objsiz, Sint32 picatrsiz);

/* mpv_cdec.c */
Sint32 MPVCDEC_NintraBlocks(MPV mpv);
Sint32 MPVCDEC_IntraBlocks(MPV mpv);
void MPVCDEC_InitFrm(MPV mpv);

/* mpv_vlc.c */
Sint32 MPVVLC_IsVlcSizErr(void);
void MPVVLC_Init(Uint8 *vlc, MPV_IXA *ixa);

/* mpv_m2v.c: MPEG-2 decoder stub */
Sint32 MPVM2V_DecodeFrm(MPV_OBJ *mpv, SJ sj, MPV_FRM *frm);
Sint32 MPVM2V_DecodePicAtr(MPV_OBJ *mpv, SJ sj);
void MPVM2V_SetCond(MPV_OBJ *mpv, Sint32 id, Sint32 val);
void MPVM2V_Destroy(MPV_OBJ *mpv);
void *MPVM2V_Create(MPV_OBJ *mpv);
void MPVM2V_Finish(void);
void MPVM2V_Init(void);

/* mpv_lib.c */
Sint32 MPV_Init(Sint32 nhn, void *work);
void MPV_Finish(void);
MPV MPV_Create(void);
MPV mpvlib_InitHn(MPV mpv);
void MPV_GetDctCnt(MPV mpv, Sint32 *cnt0, Sint32 *cnt1);
Sint32 MPV_Destroy(MPV mpv);
void MPV_MbCbFn(void);
Sint32 MPV_SetCond(MPV mpv, Sint32 id, Sint32 val);
Sint32 MPV_GetCond(MPV mpv, Sint32 id, Sint32 *val);

/* mpv_err.c */
void MPVERR_InitErrInf(MPVERR_INF *inf);
void MPVERR_Init(void);

/* mpv_bdec.c */
void MPVBDEC_Init(MPV_IXA *ixa);

/* mpvabdec.c: block coefficient decoders */
Sint32 MPVABDEC_NintraBlock(MPV mpv, MPV_BLKPRM *prm);
Sint32 MPVABDEC_IntraBlock(MPV mpv, MPV_BLKPRM *prm);
Sint32 MPVABDEC_IntraBlockDc11(MPV mpv, MPV_BLKPRM *prm);

/* mpv_bdec.c: default intra quantiser matrix and zigzag scan */
extern Uint8 mpvbdec_dfl_iqm[64];
extern Sint8 mpvbdec_zigzag[64];

/* mpv_cmc.c */
extern void (*mpvcmc_oneref[8])(MPVMC *mc);
void MPVCMC_SetCcnt(MPV_OBJ *mpv);
void MPVCMC_InitMcOiRt(MPV_OBJ *mpv);
void MPVCMC_InitObj(MPV_OBJ *mpv);
void MPVCMC_Init(void);

/* mpv_mc.c, mpv_mcy.c: motion compensation kernels */
void MPVMC08_Init(MPVMC *mc);
void MPVMC16_Init(MPVMC *mc);
void MPVMC08_OneRef1p_TuneC(MPVMC *mc);
void MPVMC08_OneRefH2_TuneC(MPVMC *mc);
void MPVMC08_OneRefV2_TuneC(MPVMC *mc);
void MPVMC08_OneRef4p_TuneC(MPVMC *mc);

#define MPV_ERR_INVALID_HN 0xFF030200

#ifdef __cplusplus
}
#endif

#endif
