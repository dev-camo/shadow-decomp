#ifndef CRI_ADX_T_H
#define CRI_ADX_T_H

/* CRI ADXT (ADX talk: stream playback handle) internals shared by the adx_*.c units. Layout
 * from the RE4 GameCube decompilation (Ver.9.31); this game's Ver.9.44 handle is 4 bytes
 * longer (ADXT_Init clears 16 * 0xC4 bytes): the extra field is at 0xC0, where the stream end
 * callback at 0x803C3980 compares it with the decoded data length.
 */
#include "CRI/cri_xpt.h"
#include "CRI/adx_sjd.h"
#include "CRI/sj.h"

#define ADXT_MAX_OBJ 16
#define ADXT_MAX_NCH 8

/* ADXT_OBJ.stat (internal) */
#define ADXT_ISTAT_STOP 0
#define ADXT_ISTAT_DECINFO 1
#define ADXT_ISTAT_PREP 2
#define ADXT_ISTAT_PLAYING 3
#define ADXT_ISTAT_PLAYEND_WAIT 4
#define ADXT_ISTAT_PLAYEND 5
#define ADXT_ISTAT_ERROR 6

typedef struct {
    Sint8 used;               /* 0x00 */
    Sint8 stat;               /* 0x01 ADXT_ISTAT_* */
    Sint8 mode;               /* 0x02 input kind (0/1: file stream, 2: memory, 3: stream joint) */
    Sint8 maxnch;             /* 0x03 */
    void *sjd;                /* 0x04 ADXSJD decoder */
    void *stm;                /* 0x08 ADXSTM stream controller */
    void *rna;                /* 0x0C ADXRNA renderer (SFADXT_Create copies it into SFAOAP.x00) */
    SJ sjf;                   /* 0x10 file stream joint */
    SJ sji;                   /* 0x14 decoder input stream joint */
    SJ sjo[2];                /* 0x18 decoder output stream joints (one per channel) */
    Uint8 *ibuf;              /* 0x20 file stream ring buffer (ADXT_Create work layout) */
    Sint32 ibufsize;          /* 0x24 */
    Sint32 ibufxsize;         /* 0x28 */
    Uint8 *obuf;              /* 0x2C decoder output ring buffers */
    Sint32 obufsize;          /* 0x30 (samples) */
    Sint32 obufstride;        /* 0x34 (samples, including the extra area) */
    Sint32 svrfreq;           /* 0x38 server calls per second */
    Sint16 ibuf_nsct;         /* 0x3C input (file stream) buffer size in sectors */
    Sint16 reload_nsct;       /* 0x3E reload threshold in sectors (85% of the buffer) */
    Sint16 outvol;            /* 0x40 */
    Sint16 outpan[2];         /* 0x42 */
    Sint16 outbal;            /* 0x46 output balance (-15..15) */
    Sint32 maxdecsmpl;        /* 0x48 samples decoded per server call */
    Sint32 lpcnt;             /* 0x4C loops done */
    Sint32 lpendmod;          /* 0x50 loop end offset within its sector */
    Sint32 x54;               /* 0x54 */
    Sint32 x58;               /* 0x58 */
    Sint32 x5c;               /* 0x5C */
    Sint16 errcode;           /* 0x60 */
    Sint16 x62;               /* 0x62 */
    Sint32 x64;               /* 0x64 */
    Sint16 x68;               /* 0x68 */
    Sint16 x6a;               /* 0x6A */
    Sint8 lpsw;               /* 0x6C loop switch */
    Sint8 autorcvr;           /* 0x6D automatic recovery switch */
    Sint8 x6e;                /* 0x6E */
    Sint8 x6f;                /* 0x6F */
    Sint8 pausesw;            /* 0x70 */
    Sint8 x71;                /* 0x71 */
    Sint8 x72;                /* 0x72 pause state (ADXT_Pause) */
    Sint8 x73;                /* 0x73 */
    void *amp;                /* 0x74 ADXAMP */
    SJ sjx[2];                /* 0x78 extra stream joints destroyed with the handle */
    SJ sjy[2];                /* 0x80 */
    Sint32 timeofst;          /* 0x88 ADXT_SetTimeOfst */
    Sint32 lpendsct;          /* 0x8C loop end in sectors */
    Sint32 trapnsmpl;         /* 0x90 */
    void *lsc;                /* 0x94 load scheduler handle */
    Sint8 lnksw;              /* 0x98 link (concatenated file) switch */
    Uint8 pad99[0x9C - 0x99]; /* 0x99 */
    Uint32 x9c;               /* 0x9C time position at the last (re)start in adxt_time_unit units */
    Sint32 startvsync;        /* 0xA0 adxt_vsync_cnt at start */
    Sint32 decsmpl;           /* 0xA4 samples decoded before the current (linked) file */
    Sint8 stmstart;           /* 0xA8 start the stream when the decoder is ready */
    Sint8 xa9;                /* 0xA9 Ver.9.44: 1 = apply the header default volume and pan */
    Uint8 padaa[0xAC - 0xAA]; /* 0xAA */
    Uint8 *wkend;             /* 0xAC end of the file stream ring buffer (ADXT_Create) */
    void *stm_fname;          /* 0xB0 pending adxt_start_stm arguments */
    void *stm_dir;            /* 0xB4 */
    Sint32 stm_ofst;          /* 0xB8 */
    Sint32 stm_nsct;          /* 0xBC */
    Sint32 xc0;               /* 0xC0 Ver.9.44 addition */
} ADXT_OBJ;

typedef ADXT_OBJ *ADXT;

extern ADXT_OBJ adxt_obj[ADXT_MAX_OBJ];
extern const Char8 adxt_build[];
extern Sint32 adxt_init_cnt;
extern Sint32 adxt_svr_main_id;
extern Sint32 adxt_output_mono_flag;
extern Sint32 adxt_svr_fs_id;
extern Sint32 adxt_vsync_cnt;

#ifdef __cplusplus
extern "C" {
#endif

/* adx_inis.c */
void ADXT_Init(void);
void ADXT_Finish(void);

/* adx_tlk.c, adx_fsvr.c */
void ADXT_ExecServer(void);
void ADXT_ExecFsSvr(void);
void ADXT_DestroyAll(void);
void ADXT_SetDefSvrFreq(Sint32 freq);

/* adx_tlk.c */
void ADXT_Stop(ADXT adxt);
void ADXT_StopWithoutLsc(ADXT adxt);
void adxt_start_sj(ADXT adxt, SJ sj);
void ADXT_SetLnkSw(ADXT adxt, Sint32 sw);

/* adx_lsc.c; ADXT_ExecLscSvr's name is inferred */
void ADXT_EntryFname(ADXT adxt, Char8 *fname);
void ADXT_StartSeamless(ADXT adxt);
void ADXT_SetSeamlessLp(ADXT adxt, Sint32 flg);
void ADXT_StartFnameLp(ADXT adxt, Char8 *fname);
Sint32 ADXT_GetNumFiles(ADXT adxt);
void ADXT_ResetEntry(ADXT adxt);
void ADXT_ExecLscSvr(void);

/* adx_tsvr.c */
void ADXT_ExecHndl(ADXT adxt);

/* adx_insh.c */
void ADXT_InsertHdrSfa(ADXT_OBJ *adxt, Sint32 a, Sint32 b, Sint32 c);

#ifdef __cplusplus
}
#endif

#endif
