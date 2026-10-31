/* CRI Sofdec decoder core (SFD) object layout, from the field offsets used by the DOL. */
#ifndef CRI_SFD_H
#define CRI_SFD_H

#include "CRI/adx_rna.h"
#include "CRI/cri_xpt.h"
#include "CRI/sfd_tst.h"
#include "CRI/sj.h"
#include "CRI/uty.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SFD_OBJ *SFD;

/* handle states (SFD_OBJ.stat, SFD_GetHnStat) and state requests (SFD_OBJ.req) */
#define SFD_STAT_NONE 0
#define SFD_STAT_STOP 1
#define SFD_STAT_PREP 2
#define SFD_STAT_STBY 3
#define SFD_STAT_PLAYING 4
#define SFD_STAT_PLAYEND 6

#define SFD_REQ_NONE 0
#define SFD_REQ_STOP 1
#define SFD_REQ_PREP 2
#define SFD_REQ_STBY 3
#define SFD_REQ_START 4
#define SFD_REQ_STARTEND 6
#define SFD_REQ_END 6

/* user-output (usr sj) channel */
typedef struct {
    void *sj;    /* 0x00 */
    void *prm;   /* 0x04 */
    Sint32 rsv1; /* 0x08 */
    Sint32 rsv2; /* 0x0C */
} SFUO_CH;
CHECK_SIZEOF(SFUO_CH, 0x10);

typedef struct {
    Sint32 nch;    /* 0x00 */
    SFUO_CH ch[3]; /* 0x04 */
} SFUO;
CHECK_SIZEOF(SFUO, 0x34);

/* audio output (auto-play) driver work (sfd_aoap.c), at SFD_OBJ + 0x34FC: user callbacks */
typedef struct {
    AXRNA x00;                                           /* 0x00 ADXT renderer (sfd_adxt.c) */
    Sint32 (*SetOutPan)(SFD sfd, Sint32 ch, Sint32 pan); /* 0x04 */
    Sint32 (*GetOutPan)(SFD sfd, Sint32 ch);             /* 0x08 */
    Sint32 (*SetOutVol)(SFD sfd, Sint32 vol);            /* 0x0C */
    Sint32 (*GetOutVol)(SFD sfd);                        /* 0x10 */
    void (*SetSpeed)(SFD sfd, Sint32 speed);             /* 0x14 */
    Sint32 x18;                                          /* 0x18 */
} SFAOAP;
CHECK_SIZEOF(SFAOAP, 0x1C);

/* clock source (sfd_tim.c) */
typedef Sint32 (*SFTIM_FN)(SFD sfd, Sint32 *ncount, Sint32 *tscale);

/* timecode as the user passes it to SFD_Tc2Time (0x20 bytes) */
typedef struct {
    Sint32 type;  /* 0x00 frame rate (index into SFTIM_prate) */
    Sint32 drop;  /* 0x04 drop frame */
    Sint32 hour;  /* 0x08 */
    Sint32 min;   /* 0x0C */
    Sint32 sec;   /* 0x10 */
    Sint32 frm;   /* 0x14 */
    Sint32 frm2;  /* 0x18 added to frm */
    Sint16 x1c;   /* 0x1C */
    Sint16 field; /* 0x1E half frame */
} SFTIM_TC;
CHECK_SIZEOF(SFTIM_TC, 0x20);

/* time with its timecode (0x2C bytes) */
typedef struct {
    Sint32 valid; /* 0x00 */
    SFTIM_TC tc;  /* 0x04 */
    Sint32 val;   /* 0x24 */
    Sint32 unit;  /* 0x28 */
} SFTIM_TTU;
CHECK_SIZEOF(SFTIM_TTU, 0x2C);

/* PTS manager entry kept across a player reset (SFTIM_WORK.ptsm, sfd_ply.c) */
typedef struct {
    void *x00;  /* 0x00 */
    Sint32 x04; /* 0x04 */
    Sint32 x08; /* 0x08 */
} SFPLY_PTSM;
CHECK_SIZEOF(SFPLY_PTSM, 0xC);

/* entries in each concatenated-play ring queue of SFTIM_WORK (indexed modulo this, sfd_con.c) */
#define SFCON_QUE_NUM 32

/* timing work at SFD_OBJ + 0xD30 (SFD_TIM(sfd)): clock sources and times (sfd_tim.c), the
 * concatenated-play queues (sfd_con.c), the time stabiliser work and the PTS manager entry.
 * pts_base is the first picture PTS less its temporal-reference offset, clamped at 0 (video
 * driver); it is saved in and restored from the seek work (sfd_mps.c). */
typedef struct {
    SFTIM_FN timefn[6];  /* 0x000 clock sources (SFTIM_SetTimeFn / cond 0xF) */
    Bool (*isskipfn)();  /* 0x018 SFD_SetUsrIsSkipFn */
    SFTIM_TC tc;         /* 0x01C */
    SFTIM_TTU ttu0;      /* 0x03C (INT_MAX) */
    SFTIM_TTU ttu1;      /* 0x068 (-1) */
    SFTIM_TTU tot;       /* 0x094 (-1) total time of the concatenated files (sfd_see.c) */
    SFTIM_TTU ttu3;      /* 0x0C0 (0) */
    SFTIM_TTU vstart;    /* 0x0EC (-1) video start time */
    SFTIM_TTU vofst;     /* 0x118 (INT_MAX) video start offset (SFTIM_GetVideoStartSample) */
    Sint32 astart_smpl;  /* 0x144 SFTIM_SetStartTime */
    Sint32 astart_sfreq; /* 0x148 */
    Sint32 x14c;         /* 0x14C */
    Sint64 pts_base;     /* 0x150 (-1) video PTS base (inferred, see above) */
    Sint64 as_pts;       /* 0x158 (-1) audio start: audio PTS min - seek work min (sfd_mps.c) */
    Sint32 x160;         /* 0x160 */
    Sint32 ctime;        /* 0x164 accumulated concat time */
    Sint32 ctime_idx;    /* 0x168 */
    Sint32 ctime_que[SFCON_QUE_NUM]; /* 0x16C */
    Sint32 tot_last;                 /* 0x1EC (1) */
    Sint32 x1f0;                     /* 0x1F0 */
    Sint32 que_wr;                   /* 0x1F4 */
    Sint32 que_rd;                   /* 0x1F8 */
    Sint32 tot_que[SFCON_QUE_NUM];   /* 0x1FC */
    Sint32 vterm;                    /* 0x27C (-5) video end time */
    Sint32 vterm_unit;               /* 0x280 (1) */
    Sint32 x284;                     /* 0x284 (-5) */
    Sint32 x288;                     /* 0x288 (1) */
    Sint32 cur;                      /* 0x28C (-1) current time (SFTIM_GetTime) */
    Sint32 cur_unit;                 /* 0x290 (1) */
    Sint32 itime;                    /* 0x294 (-5) */
    Sint32 itime_min;                /* 0x298 (INT_MAX) */
    Sint32 itime_max;                /* 0x29C (0) */
    Sint32 x2a0;                     /* 0x2A0 (INT_MAX) */
    Sint32 x2a4;                     /* 0x2A4 */
    Sint32 vsync;                    /* 0x2A8 vsync clock count */
    Sint32 speed;                    /* 0x2AC (1000) */
    Sint32 x2b0;                     /* 0x2B0 */
    Sint32 x2b4;                     /* 0x2B4 */
    Sint32 x2b8;                     /* 0x2B8 (1) */
    Sint32 x2bc;                     /* 0x2BC (100) frame decision count */
    Float32 x2c0;                    /* 0x2C0 (-1.0) frame time (ft) of the last decision count */
    Sint32 x2c4;                     /* 0x2C4 last frame decision */
    Float32 x2c8;                    /* 0x2C8 (-1.0) frame time (ft) of the last decided frame */
    /* volatile: written on another thread. SFD_VbIn runs as an SVM vsync server (the MW player
     * registers MWSFSVR_VsyncThrdProc with MWSFSVM_EntryIdVfunc), i.e. on the ADXM vsync thread;
     * there sftim_VbInHn updates vcnt and chg_base, and ext_last through clock source 5
     * (sftim_GetTimeExtClock), while the server step reads them. Without the qualifiers the
     * branch layout of SFTIM_IsGetFrmTimeTunit changes. */
    volatile Sint32 vcnt;     /* 0x2CC (-1) vsync count while requested */
    volatile Sint32 chg_base; /* 0x2D0 clock at the last time change */
    SFTIM_FN extfn;           /* 0x2D4 external clock (SFD_SetExtClockFn) */
    volatile Sint32 ext_last; /* 0x2D8 (-5) last external clock count */
    Sint32 ext_cnt;           /* 0x2DC */
    Sint32 ext_unit;          /* 0x2E0 (1) */
    Sint32 ext_wrap;          /* 0x2E4 (-1) */
    void *ext_obj;            /* 0x2E8 */
    Sint32 x2ec;              /* 0x2EC */
    SFTST_WORK tst;           /* 0x2F0 time stabiliser work */
    struct {
        Sint16 rpt;  /* repeat_first_field count of the picture */
        Sint16 acc;  /* accumulated repeated fields */
    } rfld[64];      /* 0x4B0 per temporal reference (sfd_mpv.c) */
    SFPLY_PTSM ptsm; /* 0x5B0 */
} SFTIM_WORK;
CHECK_SIZEOF(SFTIM_WORK, 0x5C0);

typedef SFTIM_WORK *SFTIM;

/* library-wide timing work (SFLIB_WORK.tim) */
typedef struct {
    Sint32 vcnt;  /* 0x00 vsync count (SFTIM_VbIn) */
    Sint32 x04;   /* 0x04 */
    Sint32 vrate; /* 0x08 refresh rate x1000 (59940 = 59.94 Hz) */
} SFTIM_LIB;
CHECK_SIZEOF(SFTIM_LIB, 0xC);

extern const Sint32 SFTIM_prate[9];

/* PTS queue entry (16 bytes, 8-byte aligned) */
typedef struct {
    Sint64 pts; /* 0x00 */
    Uint32 pos; /* 0x08 */
    Uint32 len; /* 0x0C */
} SFPTS_ENT;
CHECK_SIZEOF(SFPTS_ENT, 0x10);

typedef struct {
    SFPTS_ENT *ent; /* 0x00 */
    Sint32 num;     /* 0x04 */
    Sint32 cnt;     /* 0x08 */
    Sint32 wr;      /* 0x0C */
    Sint32 rd;      /* 0x10 */
} SFPTS_QUE;
CHECK_SIZEOF(SFPTS_QUE, 0x14);

/* per-frame picture information handed to the user (0x80 bytes) */
typedef struct {
    Sint32 raw[0x20]; /* 0x00 */
} SFD_VFRM_INF;
CHECK_SIZEOF(SFD_VFRM_INF, 0x80);

/* video frame slot (0x88 bytes), SFD_OBJ + 0x16B0 + n * 0x88; slot n pairs with SFMPV frame n */
typedef struct {
    void *frm;        /* 0x00 */
    Sint32 x04;       /* 0x04 */
    SFD_VFRM_INF inf; /* 0x08 */
} SFD_VFRM;
CHECK_SIZEOF(SFD_VFRM, 0x88);

/* ring-buffer supply of an SFBUF (sfd_buf.c): kind 0 = a memory range (ofst/size), else a stream
 * joint */
typedef struct {
    Sint32 kind;  /* 0x00 */
    SJ sj;        /* 0x04 */
    Uint32 ofst;  /* 0x08 ring buffer start */
    Sint32 size;  /* 0x0C ring buffer size */
    Sint32 xsize; /* 0x10 extra (wrap) size */
    Sint32 x14;   /* 0x14 */
} SFBUF_SUP;
CHECK_SIZEOF(SFBUF_SUP, 0x18);

/* ring buffer state (SFBUF_WORK.u.ring) */
typedef struct {
    SFBUF_SUP sup;    /* 0x00 */
    Uint8 *dlm_pos;   /* 0x18 delimiter position */
    Sint32 dlm_len;   /* 0x1C */
    Sint32 wtot;      /* 0x20 total written (-1: unknown) */
    Sint32 rtot;      /* 0x24 total read */
    SFPTS_QUE ptsque; /* 0x28 */
} SFBUF_RING;
CHECK_SIZEOF(SFBUF_RING, 0x3C);

/* SFBUF_WORK.mode */
#define SFBUF_MODE_VFRM 1
#define SFBUF_MODE_AOUT 2
#define SFBUF_MODE_UO 3
#define SFBUF_MODE_NONE 4
#define SFBUF_MODE_RING 5

/* SFBUF work (0x74 bytes): buffer n links transfer driver in_tr -> out_tr; SFD_OBJ + 0x1310 + n *
 * 0x74 */
typedef struct {
    Sint32 mode;    /* 0x00 SFBUF_MODE_* */
    Sint32 used;    /* 0x04 supplied */
    Sint32 prepflg; /* 0x08 */
    Sint32 termflg; /* 0x0C */
    union {
        SFBUF_RING ring; /* 0x10 */
        struct {
            Uint32 adr;     /* 0x10 */
            Sint32 size;    /* 0x14 */
            Sint32 x18;     /* 0x18 */
            Sint32 x1c;     /* 0x1C */
            SFD_VFRM *vfrm; /* 0x20 */
        } vfrm;
        struct {
            Uint32 adr;     /* 0x10 */
            Sint32 size;    /* 0x14 */
            Sint32 rsv[7];  /* 0x18 */
            Sint32 rsv2[3]; /* 0x34 (cleared by SFBUF_InitHn together with rsv) */
        } aout;
        SFUO_CH uoch[3]; /* 0x10 user-output channels */
    } u;
    Sint32 in_tr;  /* 0x4C transfer driver writing this buffer (9: none) */
    Sint32 out_tr; /* 0x50 transfer driver reading this buffer */
    Sint32 rsv[8]; /* 0x54 */
} SFBUF_WORK;
CHECK_SIZEOF(SFBUF_WORK, 0x74);

#define SFD_BUF_NUM 8

/* transfer/stream driver interface (SFD_tr_in_mem, SFD_tr_vo_manu, ...) */
typedef struct {
    Sint32 (*Init)();       /* 0x00 */
    Sint32 (*Finish)();     /* 0x04 */
    Sint32 (*ExecServer)(); /* 0x08 */
    Sint32 (*Create)();     /* 0x0C */
    Sint32 (*Destroy)();    /* 0x10 */
    Sint32 (*Standby)();    /* 0x14 */
    Sint32 (*Start)();      /* 0x18 */
    Sint32 (*Stop)();       /* 0x1C */
    Sint32 (*Pause)();      /* 0x20 */
    Sint32 (*GetWrite)();   /* 0x24 */
    Sint32 (*AddWrite)();   /* 0x28 */
    Sint32 (*GetRead)();    /* 0x2C */
    Sint32 (*AddRead)();    /* 0x30 */
    Sint32 (*Seek)();       /* 0x34 */
} SFD_TR_IF;
CHECK_SIZEOF(SFD_TR_IF, 0x38);

/* the driver interfaces the MW player registers, in the driver table's order */
extern const SFD_TR_IF SFD_tr_in_mem;
extern const SFD_TR_IF SFD_tr_sd_mps;
extern const SFD_TR_IF SFD_tr_vd_mpv;
extern const SFD_TR_IF SFD_tr_vo_manu;
extern const SFD_TR_IF SFD_tr_ad_adxt;
extern const SFD_TR_IF SFD_tr_ao_auto_p;
extern const SFD_TR_IF SFD_tr_uo;

typedef Sint32 (*SFD_TR_FUNC)();

/* per-transfer-driver slot (0x44 bytes), SFD_OBJ + 0x1F30 + n * 0x44 */
typedef struct {
    Sint32 prepflg;           /* 0x00 */
    Sint32 termflg;           /* 0x04 */
    void *hn;                 /* 0x08 driver handle (tr[8]: SFUO *) */
    SFD_TR_FUNC *trif;        /* 0x0C driver function table */
    Sint32 bufin;             /* 0x10 SFBUF id read by this driver */
    Sint32 bufout;            /* 0x14 SFBUF id written by this driver */
    Sint32 bufout2;           /* 0x18 */
    Sint32 bufout3;           /* 0x1C */
    Sint32 x20;               /* 0x20 (-1) */
    Uint8 pad24[0x44 - 0x24]; /* 0x24 */
} SFD_TR;
CHECK_SIZEOF(SFD_TR, 0x44);

#define SFD_TR_NUM 9
/* transfer driver slots of the ADX audio driver (sfd_adxt.c) and the auto-play audio output driver
 * (sfd_aoap.c) */
#define SFADXT_TR 3
#define SFAOAP_TR 7
#define SFD_COND_NUM 100

/* condition ids (SFSET_GetCond / SFD_SetCond) */
#define SFD_COND_VIDEO_ON 0x05
#define SFD_COND_AUDIO_ON 0x06
#define SFD_COND_PREPSIZE 0x16
#define SFD_COND_VID_STMID 0x1D
#define SFD_COND_AUD_STMID 0x1E
#define SFD_COND_STMID_STRICT 0x37
#define SFD_COND_VID_SELECT 0x3B
#define SFD_COND_HDRCB_FN 0x4B
#define SFD_COND_HDRCB_OBJ 0x4C
/* switch video / audio off when the stream has no elementary stream and no input data once the
 * other stream's output is prepared (sfmps_ProcPrep) */
#define SFD_COND_VIDEO_AUTO_OFF 0x4F
#define SFD_COND_AUDIO_AUTO_OFF 0x50
#define SFD_COND_SYSFN 0x55
#define SFD_COND_SYSOBJ 0x56
#define SFD_COND_PSMAPFN 0x57
#define SFD_COND_PSMAPOBJ 0x58
#define SFD_COND_PESFN 0x5B
#define SFD_COND_PESOBJ 0x5C

#define SFD_VFRM_NUM 16

/* SFMPV_FRM.stat */
#define SFMPV_FRM_FREE 0
#define SFMPV_FRM_ALLOC 1
#define SFMPV_FRM_STBY 2
#define SFMPV_FRM_DRAWN 3
#define SFMPV_FRM_REF 4

/* reference frame buffer planes handed to the MPEG decoder (sfd_mpv.c, 0x10 bytes) */
typedef struct {
    void *cb;      /* 0x00 y + luma plane size */
    void *cr;      /* 0x04 cb + chroma plane size */
    void *y;       /* 0x08 */
    Sint16 cwidth; /* 0x0C chroma row bytes */
    Sint16 ywidth; /* 0x0E luma row bytes */
} SFMPV_PLANE;
CHECK_SIZEOF(SFMPV_PLANE, 0x10);

/* picture user data slot of a decoded frame */
typedef struct {
    void *buf;  /* 0x00 */
    Sint32 len; /* 0x04 */
} SFMPV_PICUSR;
CHECK_SIZEOF(SFMPV_PICUSR, 0x8);

/* decoded frame object of the video driver (0xE8 bytes) */
typedef struct {
    Sint32 stat;          /* 0x00 */
    Sint32 lock;          /* 0x04 */
    void *buf;            /* 0x08 frame buffer (sfmpv_ta_adr_tbl entry) */
    SFTIM_TTU ttu;        /* 0x0C display time (ttu.tc: GOP time code + temporal reference) */
    Sint32 ftime;         /* 0x38 */
    Sint32 tunit;         /* 0x3C */
    Sint32 ndct;          /* 0x40 DCT blocks of the frame */
    Sint32 nbyte;         /* 0x44 */
    Sint32 gopno;         /* 0x48 concatenation count when decoded */
    Sint32 x4c;           /* 0x4C ttu.val */
    Sint32 x50;           /* 0x50 ttu.val + concatenation time */
    SFMPV_PICUSR *picusr; /* 0x54 */
    Sint32 x58;           /* 0x58 -1 when the frame is released for drawing */
    /* 0x5C: the MPV_PICATR of the picture (mpv.h), 0x80 bytes */
    Sint32 pa_width;          /* 0x5C */
    Sint32 pa_height;         /* 0x60 */
    Sint32 pa_mb_width;       /* 0x64 */
    Sint32 pa_mb_height;      /* 0x68 */
    Sint32 pa_frame_rate;     /* 0x6C */
    Sint32 tmpref;            /* 0x70 temporal reference (10-bit) */
    Sint32 pa_pic_type;       /* 0x74 */
    Sint32 pa_tc_drop;        /* 0x78 */
    Sint32 pa_tc_hour;        /* 0x7C */
    Sint32 pa_tc_min;         /* 0x80 */
    Sint32 pa_tc_sec;         /* 0x84 */
    Sint32 pa_tc_pic;         /* 0x88 */
    Sint32 x88;               /* 0x8C GOP headers seen */
    Sint32 x8c;               /* 0x90 sequence headers seen */
    Sint32 pa_x90[3];         /* 0x94 */
    Sint32 pa_x9c[3];         /* 0xA0 */
    Sint16 pa_xa8;            /* 0xAC */
    Sint16 pa_xaa;            /* 0xAE */
    Sint8 pa_xac;             /* 0xB0 */
    Sint8 pa_xad;             /* 0xB1 */
    Sint8 pa_xae;             /* 0xB2 */
    Sint8 pa_xaf;             /* 0xB3 */
    Sint8 pa_xb0;             /* 0xB4 */
    Sint8 pa_xb1;             /* 0xB5 */
    Sint8 pa_xb2;             /* 0xB6 */
    Sint8 pa_xb3;             /* 0xB7 */
    Sint8 pa_xb4;             /* 0xB8 */
    Sint8 pa_xb5;             /* 0xB9 */
    Sint8 pa_xb6;             /* 0xBA */
    Sint8 pa_xb7;             /* 0xBB */
    Sint8 pa_xb8;             /* 0xBC */
    Sint8 pa_xb9;             /* 0xBD */
    Sint8 pa_xba;             /* 0xBE */
    Sint8 pa_xbb;             /* 0xBF */
    Sint8 pa_xbc;             /* 0xC0 */
    Uint8 padc1[0xC4 - 0xC1]; /* 0xC1 */
    Sint32 xc4;               /* 0xC4 last key compared by SFMPVF_HoldFrm */
    Uint8 padc8[0xE0 - 0xC8]; /* 0xC8 */
    Sint64 pts;               /* 0xE0 */
} SFMPV_FRM;
CHECK_SIZEOF(SFMPV_FRM, 0xE8);

/* MPEG video decoder parameters (mwsfd_mpvpara, SFD_SetMpvParaTbl; 0x24 bytes) */
typedef struct {
    Sint32 cwidth;     /* 0x00 chroma plane width (32-byte rounded half width) */
    Sint32 cheight;    /* 0x04 */
    Sint32 width;      /* 0x08 */
    Sint32 height;     /* 0x0C */
    Sint32 x10;        /* 0x10 */
    Sint32 max_width;  /* 0x14 */
    Sint32 max_height; /* 0x18 */
    Sint32 nfrm;       /* 0x1C decoded frame buffers */
    Sint32 x20;        /* 0x20 */
} SFMPV_PARA;
CHECK_SIZEOF(SFMPV_PARA, 0x24);

#define SFMPV_FRM_NUM 16

/* video driver work (tr[2].hn, SFD_OBJ + 0x23A8, 0x10A0 bytes) */
typedef struct {
    struct MPV_OBJ *mpv;          /* 0x000 */
    SFMPV_PARA para;              /* 0x004 */
    void *rfb_adr[2];             /* 0x028 reference frame buffers */
    void *ta_adr[SFMPV_FRM_NUM];  /* 0x030 decoded frame buffers */
    SFMPV_FRM *curfrm;            /* 0x070 frame handed out by SFMPV_GetRead */
    Sint32 picstat;               /* 0x074 decode state (2, 3, 5) */
    Sint32 dlmmask;               /* 0x078 start codes accepted by the decoder (0xC0, 0xC8, 0xCC) */
    Sint32 termflg;               /* 0x07C decoder terminated */
    Sint32 gopstat;               /* 0x080 */
    Sint32 nskip;                 /* 0x084 frames skipped in a row */
    Sint32 nconcat;               /* 0x088 sequence end codes passed */
    Uint8 picatr[0x80];           /* 0x08C MPV_PICATR of the picture being decoded */
    Sint32 last_ngop;             /* 0x10C */
    Sint32 newgop;                /* 0x110 the picture starts a new GOP */
    Sint32 vbvsiz;                /* 0x114 */
    Sint32 pts_tmpref;            /* 0x118 temporal reference of the picture carrying the PTS */
    Sint32 pts_ofst;              /* 0x11C pictures since the PTS */
    Sint32 pts_max;               /* 0x120 */
    Sint32 pad124;                /* 0x0124 */
    SFPTS_ENT ptsent;             /* 0x128 last PTS queue entry seen */
    Sint32 refidx[2];             /* 0x138 reference frame buffer indices (forward, backward) */
    SFMPV_PLANE rfbuf[2];         /* 0x140 */
    SFMPV_FRM *ref[2];            /* 0x160 reference frames (forward, backward) */
    SFMPV_FRM *pendfrm;           /* 0x168 B picture decoded into a standby frame */
    Sint32 skipret;               /* 0x16C */
    Sint32 dlmwait;               /* 0x170 */
    Sint32 tmpref_adj;            /* 0x174 */
    Sint32 nfrm;                  /* 0x178 */
    Sint32 pad17c;                /* 0x017C */
    SFMPV_FRM frm[SFMPV_FRM_NUM]; /* 0x180 */
    void *picusr_buf;             /* 0x1000 */
    Sint32 picusr_num;            /* 0x1004 */
    Sint32 picusr_siz;            /* 0x1008 */
    Uint8 *picusr_dat;            /* 0x100C user data of the picture being decoded */
    Sint32 picusr_len;            /* 0x1010 */
    SFMPV_PICUSR picusr[SFMPV_FRM_NUM]; /* 0x1014 */
    Sint32 pad1094;                     /* 0x1094 */
    Sint64 pts;                         /* 0x1098 PTS of the picture being decoded */
} SFMPV_WORK;
CHECK_SIZEOF(SFMPV_WORK, 0x10A0);

/* 0xA8-byte player information block (SFD_OBJ + 0x950): cleared by sfply_InitHn and passed to the
 * picture hooks (conditions 0x24/0x25). The 64-bit counters are the stream flow statistics: the
 * system stream ones are written by sfd_mps.c and the audio ones by sfd_adxt.c; the video ones
 * are updated only by the video driver (sfd_mpv.c, not decompiled), so their meaning is
 * inferred. */
typedef struct {
    Sint32 ndecpic;   /* 0x00 decoded pictures (SFPLY_AddDecPic) */
    Sint32 nskippic;  /* 0x04 skipped pictures (SFPLY_AddSkipPic) */
    Sint32 x08;       /* 0x08 */
    Sint32 x0c;       /* 0x0C */
    Sint32 x10;       /* 0x10 */
    Sint32 x14;       /* 0x14 */
    Sint32 getcnt;    /* 0x18 frames taken by SFD_GetFrm */
    Sint32 relcnt;    /* 0x1C frames returned by SFD_RelFrm */
    Sint32 bpa;       /* 0x20 buffering pause active */
    Sint32 nbpa;      /* 0x24 buffering pauses so far */
    Sint32 x28;       /* 0x28 */
    Sint32 x2c;       /* 0x2C */
    Sint32 x30;       /* 0x30 */
    Sint64 s_flow;    /* 0x38 system stream ring: flow count (sfd_mps.c) */
    Sint64 s_byte;    /* 0x40 system stream bytes demultiplexed (sfd_mps.c) */
    Sint64 s_skip;    /* 0x48 system stream bytes skipped (sfd_mps.c) */
    Sint64 v_flow;    /* 0x50 video input ring: flow count (inferred) */
    Sint64 v_byte;    /* 0x58 video bytes handed to the decoder (inferred) */
    Sint64 v_skip;    /* 0x60 video bytes skipped up to a start code (inferred) */
    Sint64 a_in_wcnt; /* 0x68 audio input ring: written (sfd_adxt.c) */
    Sint64 a_in_rcnt; /* 0x70 audio input ring: read (sfd_adxt.c) */
    Sint64 a_byte;    /* 0x78 audio bytes handed to the decoder (sfd_adxt.c) */
    Sint64 a_sj_wcnt; /* 0x80 decoder stream joint: written (sfd_adxt.c) */
    Sint64 a_sj_rcnt; /* 0x88 decoder stream joint: read (sfd_adxt.c) */
    Sint64 x90;       /* 0x90 */
    Sint32 rsv[4];    /* 0x98 */
} SFD_PLYINF;
CHECK_SIZEOF(SFD_PLYINF, 0xA8);

/* creation parameters of SFBUF_InitHn: the first 0x2C bytes of SFD_CREPRM */
typedef struct {
    Sint32 x00;     /* 0x00 */
    Uint32 adr;     /* 0x04 work base */
    Sint32 size[7]; /* 0x08 sizes of buffers 0..6 */
    Sint32 x24;     /* 0x24 */
    Sint32 unit;    /* 0x28 ring buffer 0 alignment */
} SFBUF_PRM;
CHECK_SIZEOF(SFBUF_PRM, 0x2C);

/* creation parameters passed to SFTRN_InitHn: trif_tbl[n] is driver n's function table
 * (0 input, 1 system, 2 video, 3 audio, 4 video out, 5 audio out, 6/7 output, 8 user) */
typedef struct {
    SFD_TR_FUNC **trif_tbl; /* 0x00 */
} SFTRN_PRM;
CHECK_SIZEOF(SFTRN_PRM, 0x4);

/* the 15 driver interfaces known to the library (SFTRN_Init) */
typedef struct {
    SFD_TR_IF *tbl[15]; /* 0x00 */
} SFTRN_TRIF_TBL;
CHECK_SIZEOF(SFTRN_TRIF_TBL, 0x3C);

/* error callback state (sfd_lib.c), per handle at SFD_OBJ + 0x9F8 and library-wide */
typedef struct {
    void (*fn)(void *obj, Sint32 code); /* 0x00 */
    void *obj;                          /* 0x04 */
    Sint32 code;                        /* 0x08 first error code */
    Sint32 x0c;                         /* 0x0C */
    Sint32 x10;                         /* 0x10 */
} SFLIB_ERRINF;
CHECK_SIZEOF(SFLIB_ERRINF, 0x14);

/* Sofdec file header analysis results (sfd_hds.c, 0x894 bytes): SFD_OBJ + 0x78, SFSEE_WORK + 0xC */
#define SFHDS_RAW_SIZE 0x800

typedef struct {
    Sint32 codec; /* 0x00 */
    Sint32 layer; /* 0x04 */
    Sint32 chnum; /* 0x08 */
    Sint32 smphz; /* 0x0C */
} SFHDS_AUD;
CHECK_SIZEOF(SFHDS_AUD, 0x10);

typedef struct {
    Sint32 codec;         /* 0x00 */
    Sint32 bitrate;       /* 0x04 */
    Sint32 picw;          /* 0x08 */
    Sint32 pich;          /* 0x0C */
    Sint32 picrate;       /* 0x10 */
    Sint32 ftr_eff;       /* 0x14 feature information present */
    Sint32 ftr_coltype;   /* 0x18 */
    Sint32 ftr_pictype;   /* 0x1C */
    Sint32 ftr_fixflg;    /* 0x20 */
    Sint32 ftr_shcfixflg; /* 0x24 */
    Sint32 ftr_expand;    /* 0x28 */
    Sint32 ftr_gopn;      /* 0x2C */
    Sint32 ftr_gopm;      /* 0x30 */
} SFHDS_VID;
CHECK_SIZEOF(SFHDS_VID, 0x34);

typedef struct {
    Sint32 valid;              /* 0x00 header analysed */
    Sint32 ver_major;          /* 0x04 muxer tool version */
    Sint32 ver_minor;          /* 0x08 */
    Sint32 byterate;           /* 0x0C (negative for tool versions before 1.10) */
    Sint32 hdrsiz;             /* 0x10 */
    Sint32 packtype;           /* 0x14 */
    Sint32 pketsizlen;         /* 0x18 */
    Sint32 packsiz;            /* 0x1C */
    Sint32 numelem_tot;        /* 0x20 */
    Sint32 numelem_aud;        /* 0x24 */
    Sint32 numelem_vid;        /* 0x28 */
    Sint32 numelem_prv;        /* 0x2C */
    Sint32 maxplylen_aud;      /* 0x30 */
    Sint32 maxplylen_vid;      /* 0x34 */
    Sint32 maxfrmnum;          /* 0x38 */
    Sint32 stmid_prv1;         /* 0x3C 0xBD when present */
    Sint32 stmid_prv2;         /* 0x40 0xBF when present */
    Sint32 stmid_aud;          /* 0x44 first audio stream id (0xC0..0xDF) */
    Sint32 stmid_vid;          /* 0x48 first video stream id (0xE0..0xEF) */
    SFHDS_AUD aud;             /* 0x4C */
    SFHDS_VID vid;             /* 0x5C */
    Sint32 rawsiz;             /* 0x90 */
    Uint8 raw[SFHDS_RAW_SIZE]; /* 0x94 */
} SFHDS_FHD;
CHECK_SIZEOF(SFHDS_FHD, 0x894);

/* raw copies of the pack + system header (video one, audio one) kept for seeking (sfd_mps.c) */
typedef struct {
    Uint8 data[2][0xB0]; /* 0x00 */
    Sint32 len[2];       /* 0x160 */
} SFSEE_SYSHD;
CHECK_SIZEOF(SFSEE_SYSHD, 0x168);

/* system stream analysis kept in the seek work (sfd_mps.c), SFSEE_WORK + 0x8A0 */
typedef struct {
    Sint32 analyzed;   /* 0x00 */
    Sint32 ncount;     /* 0x04 byte rate: pack-header mux_rate (50-byte/s units) * 50 */
    Sint32 tscale;     /* 0x08 */
    Sint32 nvid;       /* 0x0C video streams of the system header */
    Sint32 naud;       /* 0x10 audio streams */
    Sint32 pad14;      /* 0x14 */
    Sint64 pts_base;   /* 0x18 video PTS base saved from SFTIM_WORK.pts_base (inferred) */
    Sint64 pts_min;    /* 0x20 */
    Sint32 stmid_vid;  /* 0x28 first video stream id */
    Sint32 stmid_aud;  /* 0x2C first audio stream id */
    SFSEE_SYSHD syshd; /* 0x30 */
} SFSEE_SHDR;          /* 0x198 */
CHECK_SIZEOF(SFSEE_SHDR, 0x198);

/* audio stream header analysis kept in the seek work (SFSEE_WORK.aud) */
typedef struct {
    Sint32 analyzed; /* 0x00 header analysed (driver 3) */
    Sint32 ncount;   /* 0x04 byte rate with tscale 1 (sfadxt_AnalyAhdr: sfreq * nch * 9 / 16) */
    Sint32 tscale;   /* 0x08 */
    Sint32 nch;      /* 0x0C channels */
    Sint32 sfreq;    /* 0x10 sampling rate */
    Sint32 nsmpl;    /* 0x14 samples */
} SFSEE_AHDR;
CHECK_SIZEOF(SFSEE_AHDR, 0x18);

/* seek support work supplied by the user through SFD_EntrySeek (sfd_see.c); the header analysis
 * results of the video / audio streams are kept here, followed by the user-set totals */
typedef struct {
    Sint32 analyzed; /* 0x000 header analysis done (sfsee_ExecHeadAnaly) */
    /* rate from the header analysis (sfsee_ExecHeadAnaly): a byte rate with a system stream, else
     * copied from vid_ncount/aud.ncount (meaning unverified); sfsee_CalcByteRate falls back to it */
    Sint32 ncount;               /* 0x004 */
    Sint32 tscale;               /* 0x008 */
    SFHDS_FHD fhd;               /* 0x00C system header analysis */
    SFSEE_SHDR shdr;             /* 0x8A0 system stream analysis (sfd_mps.c), 0x198 bytes */
    Uint8 pada38[0xAD0 - 0xA38]; /* 0xA38 */
    Sint32 vid_analyzed;         /* 0xAD0 video header analysed (driver 2) */
    Sint32 vid_ncount;           /* 0xAD4 */
    Sint32 vid_tscale;           /* 0xAD8 */
    Uint8 padadc[0xD0C - 0xADC]; /* 0xADC */
    SFSEE_AHDR aud;              /* 0xD0C audio header analysis (driver 3) */
    Uint8 padd24[0xDA8 - 0xD24]; /* 0xD24 */
    Sint32 rate;                 /* 0xDA8 estimated byte rate */
    Sint32 fsize_est;            /* 0xDAC file size seen by the input driver */
    Sint32 tot_est;              /* 0xDB0 total time from the concatenation work */
    Sint32 tunit_est;            /* 0xDB4 */
    Sint32 av_a;                 /* 0xDB8 */
    Sint32 av_b;                 /* 0xDBC */
    Sint32 paddc0;               /* 0xDC0 */
    Sint32 fsize;                /* 0xDC4 SFD_SetFileSize */
    Sint32 tottime;              /* 0xDC8 SFD_SetTotTime */
    Sint32 tunit;                /* 0xDCC */
    Sint32 byterate;             /* 0xDD0 SFD_SetByteRate */
    Sint32 seekpos;              /* 0xDD4 SFD_SetSeekPos */
} SFSEE_WORK;
CHECK_SIZEOF(SFSEE_WORK, 0xDD8);

typedef struct {
    Sint32 x00; /* 0x00 */
    Sint32 pos; /* 0x04 requested seek position (-3: none) */
    Sint32 x08; /* 0x08 */
} SFSEE_REQ;
CHECK_SIZEOF(SFSEE_REQ, 0xC);

/* ADX audio driver creation parameters (SFD_SetAdxtPara, 0x1C bytes) */
typedef struct {
    Sint32 bsize;    /* 0x00 ring buffer size */
    Sint32 xsize;    /* 0x04 ring buffer extra size */
    void *buf;       /* 0x08 ring buffer (32-byte aligned) */
    Sint32 maxnch;   /* 0x0C */
    Sint32 x10;      /* 0x10 */
    Sint32 worksize; /* 0x14 */
    void *work;      /* 0x18 ADXT work (32-byte aligned) */
} SFADXT_PARA;
CHECK_SIZEOF(SFADXT_PARA, 0x1C);

/* ADX audio driver work (sfd_adxt.c, tr[3].hn), SFD_OBJ + 0x3448 */
typedef struct {
    void *adxt;       /* 0x00 ADXT handle */
    SJ sj;            /* 0x04 ring buffer stream joint fed by the transfer */
    SFADXT_PARA para; /* 0x08 */
    Sint32 tcount;    /* 0x24 last time (-1) */
    Sint32 tunit;     /* 0x28 (1) */
    Sint32 pause;     /* 0x2C */
    Sint32 discard;   /* 0x30 samples still to discard */
    Sint32 seekflg;   /* 0x34 */
    Sint32 smplofst;  /* 0x38 samples skipped before the audio start */
    void (*func)(SFD sfd, Uint8 *data, Sint32 len,
                 Sint32 *nbyte); /* 0x3C transfer state function */
    Sint32 x40;                  /* 0x40 */
    Sint32 svrfreq;              /* 0x44 (-1) */
    Sint32 totbyte;              /* 0x48 bytes copied into the stream joint */
    Sint32 x4c;                  /* 0x4C */
} SFADXT_WORK;
CHECK_SIZEOF(SFADXT_WORK, 0x50);

/* seek request state, SFD_OBJ + 0x3550 */
typedef struct {
    SFSEE_WORK *wk; /* 0x00 */
    SFSEE_REQ req;  /* 0x04 */
} SFSEE_HN;
CHECK_SIZEOF(SFSEE_HN, 0x10);

/* creation parameters (SFD_Create, 0x44 bytes); the handle starts with a copy of them (sfd_ply.c).
 * The first 0x2C bytes are what sfd_buf.c reads as SFBUF_PRM. */
typedef struct {
    SFD_TR_FUNC **trif_tbl; /* 0x00 transfer driver tables, as SFTRN_PRM.trif_tbl */
    Uint32 adr;             /* 0x04 buffer work base (aligned to 32 in place by sfply_InitHn) */
    Sint32 size[7];         /* 0x08 sizes of buffers 0..6 */
    Sint32 x24;             /* 0x24 */
    Sint32 unit;            /* 0x28 ring buffer 0 alignment */
    Sint32 x2c;             /* 0x2C */
    Sint32 x30;             /* 0x30 */
    Sint32 x34;             /* 0x34 */
    Sint32 x38;             /* 0x38 */
    void *hnwork;           /* 0x3C handle work (SFD_OBJ, aligned to 32) */
    Sint32 hnwksiz;         /* 0x40 its size (>= 0x3660, SFD_HNWORK_MIN) */
} SFD_CREPRM;
CHECK_SIZEOF(SFD_CREPRM, 0x44);

/* Optional PTS manager hooks (sfd_ply.c; nothing in the game sets them). The system stream driver
 * passes the PTS of each packet for buffer 2 (the audio driver's input) to the info hook; when it
 * returns -1 the packet is not written and stays unconsumed. */
typedef Sint32 (*SFPLY_PTSINFO_FN)(SFPLY_PTSM *ptsm, SFPTS_ENT *ent);
typedef void (*SFPLY_PTSRESET_FN)(SFPLY_PTSM *ptsm);
extern SFPLY_PTSINFO_FN SFPLY_SetPtsInfo;
extern SFPLY_PTSRESET_FN SFPLY_ResetPtsm;

/* server time statistics (sfd_tmr.c, 0x20 bytes) */
typedef struct {
    Sint64 sum; /* 0x00 */
    Sint64 min; /* 0x08 */
    Sint64 max; /* 0x10 */
    Sint32 cnt; /* 0x18 */
} SFTMR_TSUM;
CHECK_SIZEOF(SFTMR_TSUM, 0x20);

/* frame statistics of one play (sfd_ply.c; field meanings inferred from use) */
typedef struct {
    Sint64 t0;    /* 0x00 clock at the first SFD_GetFrm (fn_803FEA5C) */
    Sint64 t1;    /* 0x08 clock when the play ended */
    Sint64 tunit; /* 0x10 clock units per second at the end (fn_803FE96C) */
    Sint32 nfrm;  /* 0x18 frames taken by SFD_GetFrm */
    Float32 rate; /* 0x1C nfrm * tunit / (t1 - t0) */
} SFPLY_FRMSTAT;
CHECK_SIZEOF(SFPLY_FRMSTAT, 0x20);

/* system stream (MPS) driver work (sfd_mps.c), SFD_OBJ + 0x2198, tr[1].hn */
#define SFMPS_STMID_MIN 0xBC
#define SFMPS_STMID_MAX 0xFF
#define SFMPS_OUTSJ_NUM (SFMPS_STMID_MAX - SFMPS_STMID_MIN + 1)

typedef struct {
    struct MPS_OBJ *mps;          /* 0x000 */
    Sint32 nvid;                  /* 0x004 video_bound of the system headers */
    Sint32 naud;                  /* 0x008 audio_bound */
    Sint32 x0c;                   /* 0x00C */
    Sint64 pts_min;               /* 0x010 minimum audio PTS */
    Sint64 pts_min2;              /* 0x018 */
    Sint32 concat_cnt;            /* 0x020 end codes passed in concatenated play */
    Sint32 last_vid;              /* 0x024 stream id of the last video packet */
    Sint32 last_aud;              /* 0x028 stream id of the last audio packet */
    Sint32 first_vid;             /* 0x02C first video stream id seen (-1) */
    Sint32 first_aud;             /* 0x030 */
    Sint32 cur_vid;               /* 0x034 video stream id being decoded (-1) */
    Sint32 cur_aud;               /* 0x038 */
    Sint32 endcode;               /* 0x03C an end code was seen at the pack start */
    void *outsj[SFMPS_OUTSJ_NUM]; /* 0x040 element output stream joints by stream id - 0xBC */
    void (*outfn)(void *obj, Sint32 stmid); /* 0x150 */
    void *outobj;                           /* 0x154 */
    Sint32 skip;                            /* 0x158 bytes of the pack still to skip (-1: none) */
    Sint32 x15c;                            /* 0x15C */
    Sint32 x160;                            /* 0x160 */
    Sint32 x164;                            /* 0x164 */
} SFMPS_WORK;                               /* 0x168 */
CHECK_SIZEOF(SFMPS_WORK, 0x168);

typedef struct SFD_OBJ {
    SFD_CREPRM prm;                /* 0x00 */
    Sint32 chg_flg;                /* 0x44 set after a control change */
    Sint32 stat;                   /* 0x48 */
    Sint32 req;                    /* 0x4C requested state (3 = standby, 4 = start) */
    Sint32 pause_sw;               /* 0x50 */
    Sint32 pause_cnt;              /* 0x54 */
    Sint32 x58;                    /* 0x58 frame access claimed by SFD_GetFrm/SFD_RelFrm */
    Sint32 x5c;                    /* 0x5C */
    Uint8 pad60[0x74 - 0x60];      /* 0x0060 */
    Sint32 pad74;                  /* 0x0074 */
    SFHDS_FHD fhd;                 /* 0x78 file header analysis */
    Sint32 x90c;                   /* 0x90C stream information (16 words cleared by sfply_InitHn) */
    Sint32 x910;                   /* 0x0910 */
    Sint32 x914;                   /* 0x0914 */
    Sint32 x918;                   /* 0x0918 */
    Sint32 x91c;                   /* 0x091C */
    Sint32 picrate;                /* 0x920 frame rate type (index into SFTIM_prate, 0: unknown) */
    Sint32 x924;                   /* 0x0924 last positive pack-header mux_rate (sfmps_ProcPrep) */
    Sint32 x928;                   /* 0x928 (1) */
    Sint32 x92c;                   /* 0x092C */
    Sint32 numelem_aud;            /* 0x930 copied from fhd after (re)processing */
    Sint32 numelem_vid;            /* 0x934 */
    Sint32 numelem_prv;            /* 0x938 */
    Sint32 x93c;                   /* 0x093C */
    Sint32 x940;                   /* 0x0940 */
    Sint32 x944;                   /* 0x0944 */
    Sint32 x948;                   /* 0x0948 */
    Sint32 x94c;                   /* 0x094C */
    SFD_PLYINF plyinf;             /* 0x950 */
    SFLIB_ERRINF err;              /* 0x9F8 */
    Sint32 cond[SFD_COND_NUM];     /* 0xA0C (SFSET_GetCond) */
    Sint32 cond_def[SFD_COND_NUM]; /* 0xB9C */
    SFTIM_WORK tim;                /* 0xD30 timing work (SFD_TIM) */
    /* The word at 0x12EC lies in the tail padding of tim (SFTIM_WORK is 8-aligned and ends at
     * 0x12F0); nothing in the executable accesses it. */
    Uint8 pad12F0[0x1310 - 0x12F0]; /* 0x12F0 */
    SFBUF_WORK buf[SFD_BUF_NUM];    /* 0x1310 */
    SFD_VFRM vfrm[SFD_VFRM_NUM];    /* 0x16B0 */
    /* tr[2].hn = SFMPV_WORK *, tr[SFADXT_TR].hn = SFADXT_WORK *, tr[SFAOAP_TR].hn = SFAOAP *,
     * tr[8].hn = SFUO *, tr[8].bufin = user-output SFBUF id */
    SFD_TR tr[SFD_TR_NUM];          /* 0x1F30 */
    Sint32 pad2194;                 /* 0x2194 */
    SFMPS_WORK mps;                 /* 0x2198 system stream driver work (tr[1].hn) */
    Uint8 pad2300[0x23A8 - 0x2300]; /* 0x2300 */
    SFMPV_WORK mpv;                 /* 0x23A8 video driver work (tr[2].hn) */
    SFADXT_WORK adxt;               /* 0x3448 */
    Uint8 pad3498[0x34FC - 0x3498]; /* 0x3498 */
    SFAOAP aoap;                    /* 0x34FC (tr[7].hn) */
    SFUO uo_tbl;                    /* 0x3518 */
    Sint32 pad354c;                 /* 0x354C */
    SFSEE_HN see;                   /* 0x3550 (SFD_EntrySeek) */
    SFTMR_TSUM tsum[6];             /* 0x3560 server time statistics (tsum[5]: sfply_ExecOne) */
    SFPLY_FRMSTAT frmstat;          /* 0x3620 */
} SFD_OBJ;
CHECK_SIZEOF(SFD_OBJ, 0x3640);
CHECK_OFFSETOF(SFD_OBJ, tim, 0xD30);
CHECK_OFFSETOF(SFD_OBJ, tim.tst, 0x1020);
CHECK_OFFSETOF(SFD_OBJ, tim.ptsm, 0x12E0);
/* Buffer n seen as the SFD object shifted by n buffers: the code adds n * sizeof(SFBUF_WORK) to the
 * handle and keeps buf[0]'s offset (0x1310) in each load/store displacement, where &sfd->buf[n]
 * would add 0x1310 to the base register instead. */
typedef struct {
    Uint8 pad[0x1310]; /* 0x0000 */
    SFBUF_WORK w;      /* 0x1310 sfd->buf[n] */
} SFBUF_HN;
CHECK_SIZEOF(SFBUF_HN, 0x1384);

#define SFBUF_GET_HN(sfd, n) ((SFBUF_HN *)((SFBUF_WORK *)(sfd) + (n)))

#define SFD_TIM(sfd) (&(sfd)->tim)
#define SFTIM_LIBWK (&SFLIB_libwork.tim)

/* library-wide work (sfd_lib.c), 0x228 bytes */
typedef struct {
    Sint32 cond[SFD_COND_NUM]; /* 0x000 default conditions */
    SFTRN_TRIF_TBL *trif_tbl;  /* 0x190 from the init parameters */
    Sint32 prm1;               /* 0x194 */
    Sint32 x198;               /* 0x198 */
    SFLIB_ERRINF err;          /* 0x19C */
    SFTIM_LIB tim;             /* 0x1B0 */
    Sint32 buf;                /* 0x1BC SFBUF work */
    SFTRN_TRIF_TBL trif;       /* 0x1C0 */
    Sint32 x1fc;               /* 0x1FC */
    void *x200;                /* 0x200 ADXT kept across a player reset (sfd_adxt.c) */
    SFD hn[8];                 /* 0x204 created handles */
    Sint32 x224;               /* 0x224 */
} SFLIB_WORK;
CHECK_SIZEOF(SFLIB_WORK, 0x228);

/* SFD_Init parameters */
typedef struct {
    SFTRN_TRIF_TBL *trif_tbl; /* 0x00 */
    Sint32 prm1;              /* 0x04 */
} SFD_INIT_PRM;
CHECK_SIZEOF(SFD_INIT_PRM, 0x8);

extern SFLIB_WORK SFLIB_libwork;
/* the handle last validated by SFLIB_CheckHn (sfd_lib.c) */
extern SFD sfd_hn_last;

Sint32 SFLIB_SetErr(SFD sfd, Sint32 code);
Sint32 SFLIB_CheckHn(SFD sfd);
void SFLIB_LockCs(Sint32 *cs);
void SFLIB_UnlockCs(Sint32 *cs);
void SFSET_SetCond(SFD sfd, Sint32 id, Sint32 val);
void SFTIM_Pause(SFD sfd, Sint32 sw);
void SFTIM_SetSpeed(SFD sfd, Sint32 speed);
void SFAOAP_SetSpeed(SFD sfd, Sint32 speed);
Sint32 SFD_SetElementOutSj(SFD sfd, Sint32 stmid, void *sj, void (*fn)(void *obj, Sint32 stmid),
                           void *obj);
Sint32 SFD_GetOutVol(SFD sfd);
Sint32 SFD_SetOutVol(SFD sfd, Sint32 vol);
Sint32 SFTRN_CallTrtTrif(SFD sfd, Sint32 strm, Sint32 fn, Sint32 a, Sint32 b);
Sint32 SFTRN_IsSetup(SFD sfd, Sint32 strm);
void SFBUF_SetUoch(SFD sfd, Sint32 n, Sint32 chno, SFUO_CH *ch);
Sint32 SFSET_GetCond(SFD sfd, Sint32 id);
Sint32 SFTRN_GetTermFlg(SFD sfd, Sint32 strm);
void SFTRN_SetTermFlg(SFD sfd, Sint32 strm, Sint32 flg);
Sint32 SFTRN_GetPrepFlg(SFD sfd, Sint32 strm);
void SFTRN_SetPrepFlg(SFD sfd, Sint32 strm, Sint32 flg);
Sint32 SFBUF_GetTermFlg(SFD sfd, Sint32 n);
Sint32 SFBUF_GetPrepFlg(SFD sfd, Sint32 n);
void SFBUF_SetPrepFlg(SFD sfd, Sint32 n, Sint32 flg);
/* readable / writable regions of a ring buffer (two chunks each, sfd_buf.c) */
typedef struct {
    SJCK ck1;      /* 0x00 */
    SJCK ck2;      /* 0x08 */
    Sint32 rsv[3]; /* 0x10 */
} SFBUF_RINF;
CHECK_SIZEOF(SFBUF_RINF, 0x1C);

Sint32 SFBUF_RingAddWrite(SFD sfd, Sint32 n, Sint32 nbyte,
                          Sint32 rsv); /* rsv: SFBUF_RINF.rsv[1] of the caller, unused */
Sint32 SFBUF_RingGetWrite(SFD sfd, Sint32 n, SFBUF_RINF *inf);
Sint32 SFBUF_VfrmAddRead(SFD sfd, Sint32 n, void *frm, Sint32 arg);
Sint32 SFBUF_VfrmGetRead(SFD sfd, Sint32 n, void **frm, Sint32 arg);
Sint32 SFCON_IsEndcodeSkip(SFD sfd);
Sint32 SFHDS_GetMuxVerNum(SFD sfd);
Sint32 SFHDS_GetColType(SFD sfd);
void SFHDS_ProcessHdr(SFHDS_FHD *fhd);
void SFHDS_ReprocessHdr(SFD sfd);
Bool SFHDS_IsSfdHeader(void *data, Sint32 size);
Bool SFHDS_SetHdr(SFD sfd, Sint32 type, Uint8 *data, Sint32 size, Sint32 *result);
void SFHDS_FinishFhd(SFHDS_FHD *fhd);
void SFHDS_InitFhd(SFHDS_FHD *fhd);
void SFHDS_Init(void);
Sint32 SFMPS_GetConcatCnt(SFD sfd);
void SFSEE_InitHn(SFSEE_HN *see);
void SFSEE_ExecServer(SFD sfd);
void SFSEE_FixAvPlay(SFD sfd, Sint32 a, Sint32 b);
Sint32 SFD_TermSupply(SFD sfd);
Sint32 SFD_SetCond(SFD sfd, Sint32 id, Sint32 val);
Sint32 SFD_GetHnStat(SFD sfd);
Sint32 SFD_ExecOne(SFD sfd);
Bool SFD_IsSvrWait(void);
Bool SFD_IsHnSvrWait(SFD sfd);
void SFD_VbIn(void);
Sint32 SFD_SetConcatPlay(SFD sfd);
Sint32 SFD_GetTime(SFD sfd, Sint32 *ncount, Sint32 *tscale);
Sint32 SFD_EntrySeek(SFD sfd, SFSEE_WORK *wk);
Sint32 SFD_SetSpeed(SFD sfd, Sint32 speed);
Bool SFTIM_IsGetFrmTime(SFD sfd, SFD_VFRM *frm);
Bool SFTIM_IsGetFrmTimeTunit(SFD sfd, Sint32 ftime, Sint32 tunit);
Bool SFTIM_IsVideoTerm(SFD sfd);
Bool SFTIM_ChkRegularTime(SFD sfd, Sint32 *ncount, Sint32 *tscale);
void SFTIM_GetTimeOneFrmVideo(SFD sfd, Sint32 *ncount, Sint32 *tscale);
void SFTIM_SetTimeFn(SFD sfd, SFTIM_FN fn, Sint32 type);
Sint32 SFTIM_GetAudioStartSample(SFTIM tim, Sint32 sfreq);
Sint32 SFTIM_GetVideoStartSample(SFTIM tim, Sint32 sfreq, Sint32 *ofstflg);
void SFTIM_SetStartTime(SFTIM tim, Sint32 smpl, Sint32 sfreq);

/* sfd_lib.c */
Sint32 SFD_SetErrFn(SFD sfd, void (*fn)(void *obj, Sint32 code), void *obj);
Sint32 SFD_Finish(void);
Sint32 SFD_Init(SFD_INIT_PRM *prm);
/* SFD structure size version of this build, checked by SFD_IsVersionCompatible */
#define SFD_VERSION 0x3640
Sint32 SFD_IsVersionCompatible(const Char8 *version, Sint32 ver);
void SFLIB_InitErrInf(SFLIB_ERRINF *err);

/* sfd_ply.c */
/* default conditions; one entry longer than the SFD_COND_NUM copied into each handle */
extern const Sint32 SFPLY_cond_dfl[101];
void SFPLY_Init(void);
Sint32 SFD_Destroy(SFD sfd);
Sint32 SFPLY_GetResetFlg(void);

/* sfd_trn.c */
Sint32 SFTRN_Init(SFTRN_TRIF_TBL *dst, SFTRN_TRIF_TBL *src);
Sint32 SFTRN_Finish(SFTRN_TRIF_TBL *tbl);
Sint32 SFTRN_InitHn(SFD sfd, SFD_TR *tr, SFTRN_PRM *prm, Sint32 x);
Sint32 SFTRN_CallTrSetup(SFD sfd, Sint32 fn);

/* sfd_buf.c */
void SFBUF_Init(void *work);
Sint32 SFBUF_InitHn(SFD sfd, SFBUF_WORK *wk, SFBUF_PRM *prm);
Sint32 SFBUF_GetWTot(SFD sfd, Sint32 n);
Sint32 SFBUF_RingGetDataSiz(SFD sfd, Sint32 n);
void fn_803F052C(void *work);
Sint32 SFBUF_SetSupplySj(SFD sfd, SFBUF_SUP *sup);
void SFBUF_DestroySj(SFD sfd);
void SFBUF_SetTermFlg(SFD sfd, Sint32 n, Sint32 flg);
Sint32 SFBUF_GetRTot(SFD sfd, Sint32 n);
Sint32 SFBUF_RingGetRead(SFD sfd, Sint32 n, SFBUF_RINF *inf);
Sint32 SFBUF_RingAddRead(SFD sfd, Sint32 n, Sint32 nbyte);
Sint32 SFBUF_RingGetSj(SFD sfd, Sint32 n, SJ *sj);
void SFBUF_GetFlowCnt(SJ sj, Sint32 *wcnt, Sint32 *rcnt);
Sint64 SFBUF_UpdateFlowCnt(Sint64 cnt, Uint32 pos);
void SFBUF_GetUoch(SFD sfd, Sint32 n, Sint32 chno, SFUO_CH *ch);

/* sfd_hds.c */
void fn_803F12EC(void);

/* sfd_pts.c */
void SFPTS_InitPtsQue(SFPTS_QUE *que);
Sint32 SFPTS_IsPtsQueFull(SFD sfd, Sint32 strm);
Sint32 SFPTS_WritePtsQue(SFD sfd, Sint32 strm, SFPTS_ENT *in, Sint32 *full);
Sint32 SFD_SetVideoPts(SFD sfd, void *buf, Sint32 size);

/* sfd_con.c */
Sint32 SFCON_IsSystemEndcodeSkip(SFD sfd);
Sint32 SFCON_WriteTotSmplQue(SFD sfd, Sint32 val, Sint32 last);

/* sfd_see.c */
Sint32 SFD_SetByteRate(SFD sfd, Sint32 rate);
Sint32 SFD_SetFileSize(SFD sfd, Sint32 fsize);
Sint32 SFD_SetTotTime(SFD sfd, Sint32 tottime, Sint32 tunit);
Sint32 SFD_SetSeekPos(SFD sfd, Sint32 pos);

/* sfd_adxt.c */
extern Sint32 sfadxt_stat;
extern Sint32 sfadxt_adxterr;
void SFD_SetAdxtPara(SFADXT_PARA *para);

/* sfd_tmr.c */
void SFTMR_InitTsum(SFTMR_TSUM *ts);
void SFTMR_AddTsum(SFTMR_TSUM *ts, Sint64 t);
/* Sofdec profiling clock (sfd_tmr.c): the first returns the clock units per second, the second the
 * current clock value (the time base, the last handle's external clock, or the vsync count) */
Sint64 fn_803FE96C(void);
Sint64 fn_803FEA5C(void);

/* sfd_pl2.c */
Sint32 SFPL2_Standby(SFD sfd);
Sint32 SFPL2_Pause(SFD sfd, Sint32 sw);

/* sfd_tim.c */
void SFTIM_Init(SFTIM_LIB *lib, Sint32 vrate);
void fn_803FDCD0(SFTIM_LIB *lib); /* empty; the library finish SFD_Finish calls it */
void SFTIM_InitHn(SFD sfd, SFTIM tim);
void SFTIM_VbIn(void);
Sint32 SFMPV_SaveCond(SFD sfd, void *buf, Sint32 size);
void SFMPV_RestoreCond(SFD sfd, void *buf, Sint32 n);
Sint32 SFD_SetUsrTimeFn(SFD sfd, SFTIM_FN fn);
Sint32 SFD_SetExtClockFn(SFD sfd, SFTIM_FN fn, Sint32 wrap, void *obj);
Sint32 SFD_SetUsrIsSkipFn(SFD sfd, Bool (*fn)());
/* sfd_tim.c */
Sint32 SFTIM_GetTime(SFD sfd, Sint32 *ncount, Sint32 *tscale);
Sint32 SFTIM_GetTimeSub(SFD sfd, Sint32 *ncount, Sint32 *tscale);
Bool SFTIM_IsStagnant(SFD sfd);
Sint32 SFD_CmpTime(Sint32 a, Sint32 aunit, Sint32 b, Sint32 bunit);

#ifdef __cplusplus
}
#endif

#endif
