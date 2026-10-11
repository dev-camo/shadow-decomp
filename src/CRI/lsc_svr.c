/* CRI LSC per-handle server (lsc_svr.c): moves each scheduler's queue head through
 * wait -> loading -> done against the ADX stream controller's state. Names and layout follow
 * the RE4 GameCube decompilation of the same file.
 */

extern unsigned int strlen(const char *text);

/* CRI basic types (cri_xpt.h); 32-bit integers are long. */
typedef unsigned char Uint8;
typedef signed char Sint8;
typedef signed long Sint32;
typedef unsigned long Uint32;
typedef char Char8;

#define LSC_MAX_ENTRY 16

typedef struct {
    /* 0x00 */ Sint32 id;
    /* 0x04 */ Char8 *fname;
    /* 0x08 */ Sint32 fname_sum;
    /* 0x0C */ void *dir;
    /* 0x10 */ Sint32 ofst;
    /* 0x14 */ Sint32 nsct;
    /* 0x18 */ Sint32 stat;
    /* 0x1C */ Sint32 pos;
} LSC_ENTRY;

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 stat;
    /* 0x02 */ Sint8 stm_start;
    /* 0x03 */ Sint8 loop_flag;
    /* 0x04 */ Sint8 pause_flag;
    /* 0x05 */ Uint8 pad5[3];
    /* 0x08 */ void *sj;
    /* 0x0C */ Sint32 pad0c;
    /* 0x10 */ Sint32 pad10;
    /* 0x14 */ Sint32 min_val;
    /* 0x18 */ Sint32 bufsize;
    /* 0x1C */ Sint32 wr_idx;
    /* 0x20 */ Sint32 rd_idx;
    /* 0x24 */ Sint32 num_stm;
    /* 0x28 */ void *stm;
    /* 0x2C */ Sint32 cur_nsct;
    /* 0x30 */ Sint32 pad30;
    /* 0x34 */ Sint32 pad34;
    /* 0x38 */ LSC_ENTRY tbl[LSC_MAX_ENTRY];
} LSC_OBJ;

#define LSC_STAT_STOP 0
#define LSC_STAT_PREP 1
#define LSC_STAT_EXEC 2
#define LSC_STAT_ERROR 3

#define LSC_ENT_WAIT 0
#define LSC_ENT_LOADING 1
#define LSC_ENT_DONE 2

#define ADXSTM_STAT_EXEC 2
#define ADXSTM_STAT_END 3
#define ADXSTM_STAT_ERROR 4

extern void LSC_CallStatFunc(void);
extern void LSC_CallErrFunc(Char8 *fmt, ...);
extern Sint32 LSC_EntryFileRange(LSC_OBJ *lsc, Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);

/* ADX stream controller (adx_stmc.c). */
extern Sint32 ADXSTM_GetStat(void *stm);
extern Sint32 ADXSTM_Tell(void *stm);
extern void ADXSTM_StopNw(void *stm);
extern void ADXSTM_ReleaseFileNw(void *stm);
extern void ADXSTM_BindFileNw(void *stm, const Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);
extern void ADXSTM_SetEos(void *stm, Sint32 nsct);
extern Sint32 ADXSTM_SetBufSize(void *stm, Sint32 min_nsct, Sint32 max_nsct);
extern Sint32 ADXSTM_Seek(void *stm, Sint32 pos);
extern Sint32 ADXSTM_Start(void *stm);

/* Byte sum of a file name (compared with the sum taken at entry time). */
static inline Uint32 lsc_CalcSum(Char8 *fname) {
    Uint32 len;
    Uint32 sum;
    Sint32 i;

    len = strlen(fname);
    sum = 0;
    for (i = 0; i < len; i++) {
        sum += (Uint8)fname[i];
    }
    return sum;
}

/* Retires the finished head entry: pops it, calls the status callback and goes PREP when the queue */
/* is empty; with loop_flag the same range is re-queued at the tail. */
static inline void lsc_NextEntry(LSC_OBJ *lsc) {
    LSC_ENTRY *ent;
    Char8 *fname = 0;
    void *dir = 0;
    Sint32 ofst = 0;
    Sint32 nsct = 0;

    if (lsc->stm != 0) {
        if (lsc->loop_flag == 1) {
            ent = &lsc->tbl[lsc->rd_idx];
            fname = ent->fname;
            dir = ent->dir;
            ofst = ent->ofst;
            nsct = ent->nsct;
        }
        lsc->num_stm--;
        lsc->rd_idx = (lsc->rd_idx + 1) % LSC_MAX_ENTRY;
        if (lsc->num_stm <= 0) {
            LSC_CallStatFunc();
            lsc->stat = LSC_STAT_PREP;
        }
        if (lsc->loop_flag == 1) {
            LSC_EntryFileRange(lsc, fname, dir, ofst, nsct);
        }
    }
}

/* Per-scheduler step: tracks the loading entry through the stream controller (EXEC -> pos, END -> */
/* done, ERROR -> LSC_STAT_ERROR), retires a done entry, and starts the next waiting entry by */
/* re-binding the controller to its file range (no-wait), arming the end-of-stream at its length and */
/* starting it with this scheduler's refill policy. */
void lsc_ExecHndl(LSC_OBJ *lsc) {
    LSC_ENTRY *ent;
    Uint32 sum;

    if (lsc->pause_flag == 1) {
        return;
    }
    if (lsc->stat != LSC_STAT_EXEC) {
        return;
    }
    if (lsc->num_stm <= 0) {
        return;
    }

    if (lsc->tbl[lsc->rd_idx].stat == LSC_ENT_LOADING) {
        if (lsc->stm == 0) {
            LSC_CallErrFunc("E0007: lsc->fp=NULL\n");
        } else {
            ent = &lsc->tbl[lsc->rd_idx];
            switch (ADXSTM_GetStat(lsc->stm)) {
            case ADXSTM_STAT_ERROR:
                lsc->stat = LSC_STAT_ERROR;
                break;
            case ADXSTM_STAT_EXEC:
                ent->pos = ADXSTM_Tell(lsc->stm);
                break;
            case ADXSTM_STAT_END:
                ent->pos = lsc->cur_nsct;
                ent->stat = LSC_ENT_DONE;
                break;
            }
        }
    }

    if (lsc->tbl[lsc->rd_idx].stat == LSC_ENT_DONE) {
        lsc_NextEntry(lsc);
    }

    if (lsc->tbl[lsc->rd_idx].stat == LSC_ENT_WAIT) {
        ent = &lsc->tbl[lsc->rd_idx];
        if (lsc->num_stm > 0) {
            ADXSTM_StopNw(lsc->stm);
            ADXSTM_ReleaseFileNw(lsc->stm);
            sum = lsc_CalcSum(ent->fname);
            if (sum != ent->fname_sum) {
                LSC_CallErrFunc("E0013: '%s' is different from entry file name.(LSC_ExecServer)\n",
                                ent->fname);
            } else {
                ADXSTM_BindFileNw(lsc->stm, ent->fname, ent->dir, ent->ofst, ent->nsct);
                ADXSTM_SetEos(lsc->stm, ent->nsct);
                lsc->cur_nsct = ent->nsct;
                ent->pos = 0;
                lsc->stm_start = 0;
                if (lsc->stm_start == 0) {
                    ADXSTM_SetBufSize(lsc->stm, lsc->min_val, lsc->bufsize);
                    ADXSTM_Seek(lsc->stm, 0);
                    ADXSTM_Start(lsc->stm);
                    lsc->stm_start = 1;
                }
                ent->stat = LSC_ENT_LOADING;
            }
        }
    }
}
