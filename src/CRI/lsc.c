/* CRI LSC load scheduler (lsc.c, LSC/GC Ver.2.22): queues up to 16 file ranges on one ADX
 * stream controller so several files stream back to back into one ring buffer.
 *
 * Function order and names follow the newer CRI lsc.c in the Street Fighter III 3rd Strike
 * decompilation; bodies follow the RE4 and Mortal Kombat: Deception GameCube decompilations.
 * The file is compiled with deferred inlining, so MWCC emits the functions in reverse order
 * (LSC_SetLpFlg first) and Stop/Start/Destroy inline the functions they call. Each function has
 * its own error code, numbered in source order (E2005012801..22).
 *
 * LSC_GetStmId, LSC_GetStmFname, LSC_GetStmStat, LSC_GetStmRdSct, LSC_GetFlowLimit and
 * LSC_Pause are unreferenced, so the linker strips them. Their bodies come from the 3rd Strike
 * decompilation and are not verified against this game; only their error literals, which sit
 * at the observed .rodata positions, are.
 */

#include <string.h>

#include "CRI/lsc.h"

/* Status callback installed by LSC_EntryChgStatFunc (not present in this game). With deferred
 * inlining MWCC lays out .bss in reverse declaration order, so these are declared last-first.
 */
void *lsc_stat_obj2;
void *lsc_stat_obj1;
void (*lsc_stat_func)(void *obj1, void *obj2);

/* First unused scheduler, null when all 32 are in use. */
static inline LSC lsc_Alloc(void) {
    LSC lsc = 0;
    Sint32 i;

    for (i = 0; i < LSC_MAX_OBJ; i++) {
        if (lsc_obj[i].used == 0) {
            lsc = &lsc_obj[i];
            break;
        }
    }
    return lsc;
}

/* Creates a scheduler feeding stream joint sj; the refill threshold is 80% of its size. */
LSC LSC_Create(SJ sj) {
    LSC lsc;
    Sint32 msk;
    Sint32 i;

    if (sj == 0) {
        LSC_CallErrFunc("E2005012801: Illigal parameter=sj (LSC_Create)\n");
        return 0;
    }
    LSC_LockCrs(&msk);
    lsc = lsc_Alloc();
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012802: Not enough instance (LSC_Create)\n");
    } else {
        lsc->sj = sj;
        lsc->stat = LSC_STAT_STOP;
        lsc->bufsize = SJ_GetNumData(sj, SJ_CK_FREE) + SJ_GetNumData(sj, SJ_CK_DATA);
        lsc->min_val = lsc->bufsize * 8 / 10;
        for (i = 0; i < LSC_MAX_ENTRY; i++) {
            lsc->tbl[i].stat = LSC_ENT_WAIT;
        }
        lsc->used = 1;
    }
    LSC_UnlockCrs(&msk);
    return lsc;
}

/* Stops and clears the scheduler. */
void LSC_Destroy(LSC lsc) {
    if (lsc != 0) {
        LSC_Stop(lsc);
        lsc->used = 0;
        memset(lsc, 0, sizeof(LSC_OBJ));
    }
}

/* The ADX stream controller the scheduler drives. */
void LSC_SetStmHndl(LSC lsc, ADXSTM stm) {
    lsc->stm = stm;
}

/* Queues a file range after the previous entries and returns its id (-1 when the queue is
 * full); the file name is checksummed so a changed string is detected at load time.
 */
Sint32 LSC_EntryFileRange(LSC lsc, Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct) {
    LSC_ENTRY *ent;
    Sint32 id;
    Uint32 len;
    Uint32 i;
    Uint32 prev;

    if (lsc == 0) {
        LSC_CallErrFunc("E2005012803: Illigal parameter lsc=NULL\n");
        return -1;
    }
    if (lsc->num_stm >= LSC_MAX_ENTRY) {
        return -1;
    }
    if (fname == 0) {
        LSC_CallErrFunc("E2005012804: Illigal parameter fname=%s\n", fname);
        return -1;
    }
    prev = lsc->tbl[(Sint32)((Uint32)lsc->wr_idx + (LSC_MAX_ENTRY - 1)) % LSC_MAX_ENTRY].id;
    id = (prev == LSC_ID_MAX) ? 0 : prev + 1;
    ent = &lsc->tbl[lsc->wr_idx];
    ent->id = id;
    ent->fname = fname;
    len = strlen(fname);
    ent->fname_sum = 0;
    for (i = 0; i < len; i++) {
        ent->fname_sum += (Uint8)fname[i];
    }
    ent->ofst = ofst;
    ent->nsct = nsct;
    ent->dir = dir;
    ent->stat = LSC_ENT_WAIT;
    ent->pos = 0;
    lsc->num_stm++;
    lsc->wr_idx = (Sint32)((Uint32)lsc->wr_idx + 1) % LSC_MAX_ENTRY;
    if ((Sint32)(Uint8)lsc->stat == LSC_STAT_PREP) {
        lsc->stat = LSC_STAT_EXEC;
    }
    return id;
}

/* Empties the queue of a stopped scheduler. */
void LSC_ResetEntry(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012805: Illigal parameter lsc=NULL\n");
        return;
    }
    if (lsc->stat == LSC_STAT_STOP) {
        lsc->wr_idx = 0;
        lsc->rd_idx = 0;
        lsc->num_stm = 0;
    }
}

/* Restarts the scheduler: EXEC when entries are queued, PREP otherwise. */
void LSC_Start(LSC lsc) {
    Sint32 msk;

    if (lsc == 0) {
        LSC_CallErrFunc("E2005012806: Illigal parameter lsc=NULL\n");
        return;
    }
    LSC_LockCrs(&msk);
    if (lsc->stat != LSC_STAT_STOP) {
        LSC_Stop(lsc);
    }
    if (lsc->num_stm > 0) {
        lsc->stat = LSC_STAT_EXEC;
    } else {
        lsc->stat = LSC_STAT_PREP;
    }
    LSC_UnlockCrs(&msk);
}

/* Stops the stream controller if this scheduler started it and clears the queue. */
void LSC_Stop(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012807: Illigal parameter lsc=NULL\n");
        return;
    }
    if (lsc->stat != LSC_STAT_STOP) {
        lsc->stat = LSC_STAT_STOP;
        if (lsc->stm != 0 && lsc->stm_start == 1) {
            ADXSTM_Stop(lsc->stm);
            lsc->stm_start = 0;
        }
        lsc->cur_nsct = 0;
        LSC_ResetEntry(lsc);
        lsc->pad34 = 0;
    }
}

void LSC_Pause(LSC lsc, Sint32 sw) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012808: Illigal parameter lsc=NULL\n");
        return;
    }
    if (sw == 1) {
        lsc->pause_flag = 1;
    } else {
        lsc->pause_flag = 0;
    }
}

/* Scheduler server pass: lsc_ExecHndl on every live scheduler, under the lock. */
void LSC_ExecServer(void) {
    Sint32 msk;
    LSC lsc;
    Sint32 i;

    LSC_LockCrs(&msk);
    for (i = 0; i < LSC_MAX_OBJ; i++) {
        lsc = &lsc_obj[i];
        if (lsc->used == 1) {
            lsc_ExecHndl(lsc);
        }
    }
    LSC_UnlockCrs(&msk);
}

/* Scheduler state (LSC_STAT_*). */
Sint32 LSC_GetStat(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012809: Illigal parameter lsc=NULL\n");
        return -1;
    }
    return lsc->stat;
}

/* Entries still queued (including the one loading). */
Sint32 LSC_GetNumStm(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012810: Illigal parameter lsc=NULL\n");
        return -1;
    }
    return lsc->num_stm;
}

Sint32 LSC_GetStmId(LSC lsc, Sint32 no) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012811: Illigal parameter lsc=NULL\n");
        return -1;
    }
    if (no < 0 || no >= lsc->num_stm) {
        LSC_CallErrFunc("E2005012812: Illigal parameter no=%d\n", (int)no);
        return -1;
    }
    return lsc->tbl[(lsc->rd_idx + no) % LSC_MAX_ENTRY].id;
}

Char8 *LSC_GetStmFname(LSC lsc, Sint32 stm_id) {
    Sint32 i;

    if (lsc == 0) {
        LSC_CallErrFunc("E2005012813: Illigal parameter lsc=NULL\n");
        return 0;
    }
    for (i = 0; i < LSC_MAX_ENTRY; i++) {
        if (lsc->tbl[i].id == stm_id) {
            break;
        }
    }
    if (i == LSC_MAX_ENTRY) {
        LSC_CallErrFunc("E2005012814: Can not find stream ID =%d\n", (int)stm_id);
        return 0;
    }
    return lsc->tbl[i].fname;
}

Sint32 LSC_GetStmStat(LSC lsc, Sint32 stm_id) {
    Sint32 i;

    if (lsc == 0) {
        LSC_CallErrFunc("E2005012815: Illigal parameter lsc=NULL\n");
        return -1;
    }
    for (i = 0; i < LSC_MAX_ENTRY; i++) {
        if (lsc->tbl[i].id == stm_id) {
            break;
        }
    }
    if (i == LSC_MAX_ENTRY) {
        LSC_CallErrFunc("E2005012816: Can not find stream ID =%d\n", (int)stm_id);
        return -1;
    }
    return lsc->tbl[i].stat;
}

Sint32 LSC_GetStmRdSct(LSC lsc, Sint32 stm_id) {
    Sint32 i;

    if (lsc == 0) {
        LSC_CallErrFunc("E2005012817: Illigal parameter lsc=NULL\n");
        return 0;
    }
    for (i = 0; i < LSC_MAX_ENTRY; i++) {
        if (lsc->tbl[i].id == stm_id) {
            break;
        }
    }
    if (i == LSC_MAX_ENTRY) {
        LSC_CallErrFunc("E2005012818: Can not find stream ID =%d\n", (int)stm_id);
        return 0;
    }
    return lsc->tbl[i].pos;
}

/* Refill threshold for the stream controller. */
void LSC_SetFlowLimit(LSC lsc, Sint32 min_val) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012819: Illigal parameter lsc=NULL\n");
        return;
    }
    if (min_val < 0 || min_val > lsc->bufsize) {
        LSC_CallErrFunc("E2005012820: Illigal parameter min_val=%d\n", (int)min_val);
        return;
    }
    lsc->min_val = min_val;
}

Sint32 LSC_GetFlowLimit(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012821: Illigal parameter lsc=NULL\n");
        return -1;
    }
    return lsc->min_val;
}

/* Calls the status callback when a scheduler's queue runs empty. */
void LSC_CallStatFunc(void) {
    if (lsc_stat_func != 0) {
        lsc_stat_func(lsc_stat_obj1, lsc_stat_obj2);
    }
}

/* Sets whether the scheduler requeues each finished range (loop playback). */
void LSC_SetLpFlg(LSC lsc, Sint32 flg) {
    if (lsc == 0) {
        LSC_CallErrFunc("E2005012822: Illigal parameter lsc=NULL\n");
        return;
    }
    lsc->loop_flag = flg;
}
