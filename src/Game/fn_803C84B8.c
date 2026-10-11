#include "CRI/lsc.h"

/* lsc.c error literals (its .rodata is not sourced yet). */
extern Char8 lbl_80516FD4[];
extern Char8 lbl_80517000[];

extern unsigned int strlen(const char *text);

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
        LSC_CallErrFunc(lbl_80516FD4);
        return -1;
    }
    if (lsc->num_stm >= LSC_MAX_ENTRY) {
        return -1;
    }
    if (fname == 0) {
        LSC_CallErrFunc(lbl_80517000);
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

/* The ADX stream controller the scheduler drives. */
void LSC_SetStmHndl(LSC lsc, ADXSTM stm) {
    lsc->stm = stm;
}
