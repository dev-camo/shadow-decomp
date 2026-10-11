/* CRI ADX file system (adx_fs.c, ADXF/GC Ver.7.30): sector-based file handles on top of the ADX
 * stream controller and the AFS partition loader. Public ADXF_* entries are wrappers around
 * adxf_* workers, entered through ADXCRS_Enter/ADXCRS_Leave; the small workers are inline.
 *
 * Layout and names follow the CRI adx_fs.c of the Street Fighter III 3rd Strike decompilation
 * and the TMNT2 CRI sources (worker/wrapper pairs, adxf_OpenRange). The file is compiled with
 * -inline noauto, so only the functions declared inline are expanded.
 *
 * The unreferenced functions below keep their error literals in .rodata, where they sit between
 * the literals of the matched functions. Their bodies come from the 3rd Strike decompilation and
 * are not verified against this game; the linker strips them.
 */
#include "CRI/adx_f.h"
#include "msl_string.h"

extern void *memset(void *destination, int value, unsigned int size);
extern int memcmp(const void *left, const void *right, unsigned int count);

/* AFS table load scratch state (partition loader). */
Uint8 *adxf_ldpt_buf;
Sint32 adxf_ldpt_rdsct;
Sint8 adxf_ldpt_work[0x2080];

Sint32 adxf_ReadNw32(ADXF adxf, Sint32 nsct, void *buf);
void adxf_Close(ADXF adxf);
void adxf_CloseLdptnwHn(void);
Sint32 adxf_LoadPtBothNw(Sint32 ptid, Sint32 rd_ptid, Sint32 rd_flid, const Char8 *fname, void *dir,
                         Sint32 ofst, Sint32 fnsct, ADXF_PTINFO *ptinfo, void *tmpbuf,
                         Sint32 tbsize, Sint32 rev);

/* Sectors needed for nbyte bytes (rounded up). */
static inline Sint32 adxf_CalcByte2Sct(Sint32 nbyte) {
    Sint32 nsct = nbyte / ADXF_SCT_SIZE;

    if (nbyte % ADXF_SCT_SIZE > 0) {
        nsct++;
    }
    return nsct;
}

/* Debug trace: records command cmdid (entry fg 0 / exit fg 1) and three parameters in the
 * 16-entry ring adxf_cmd_hstry; entries also count the calls per command.
 */
static inline void adxf_SetCmdHstry(Sint32 cmdid, Sint32 fg, Sint32 prm0, Sint32 prm1,
                                    Sint32 prm2) {
    ADXF_CMD_HSTRY *hstry;

    adxf_hstry_no %= ADXF_CMD_HSTRY_MAX;
    hstry = &adxf_cmd_hstry[adxf_hstry_no];
    if (fg == 0) {
        adxf_cmd_ncall[cmdid]++;
    }
    hstry->cmdid = cmdid;
    hstry->fg = fg;
    hstry->ncall = adxf_cmd_ncall[cmdid];
    hstry->prm[0] = prm0;
    hstry->prm[1] = prm1;
    hstry->prm[2] = prm2;
    adxf_hstry_no++;
}

/* Validates a partition id and its table pointer. */
static inline Sint32 adxf_ChkPrmPt(Sint32 ptid, void *ptinfo) {
    if (ptid < 0 || ptid >= ADXF_PART_MAX) {
        ADXERR_CallErrFunc1("E9040801:partition ID is range outside.(adxf_ChkPrmPt)");
        return ADXF_ERR_PRM;
    }
    if (ptinfo == 0) {
        ADXERR_CallErrFunc1("E9040802:'ptinfo' is NULL.(adxf_ChkPrmPt)");
        return ADXF_ERR_PRM;
    }
    return ADXF_ERR_OK;
}

/* Validates a loaded partition and a file id inside it. */
static inline Sint32 adxf_ChkPrmGfr(Sint32 ptid, Sint32 flid) {
    ADXF_PTINFO *ptinfo;

    if (ptid < 0 || ptid >= ADXF_PART_MAX) {
        ADXERR_CallErrFunc1("E9040828:'ptid' is range outside.");
        return ADXF_ERR_PRM;
    }
    ptinfo = adxf_ptinfo[ptid];
    if (ptinfo == 0) {
        ADXERR_CallErrFunc1("E9040828:'ptid' is range outside.");
        return ADXF_ERR_PRM;
    }
    if (flid < 0 || flid >= ptinfo->nfile) {
        ADXERR_CallErrFunc1("E9040828:'flid' is range outside.");
        return ADXF_ERR_PRM;
    }
    return ADXF_ERR_OK;
}

/* First unused handle, null when all are in use. */
static inline ADXF adxf_AllocAdxFs(void) {
    ADXF adxf = 0;
    Sint32 i;

    for (i = 0; i < ADXF_OBJ_MAX; i++) {
        if (adxf_obj[i].used == 0) {
            adxf = &adxf_obj[i];
            break;
        }
    }
    return adxf;
}

/* Releases the ring buffer stream joint of a finished read (not a caller-supplied one). */
static inline void adxf_CloseSjStm(ADXF adxf) {
    SJ sj;

    if (adxf->sj != 0 && adxf->sjflag == 0) {
        if (adxf_ocbi_fg == 1) {
            ADXF_Ocbi(adxf->buf, adxf->bsize);
        }
        sj = adxf->sj;
        adxf->sj = 0;
        SJ_Destroy(sj);
    }
}

/* Unreferenced (reference-derived, see above). */
void adxf_SetReqRdSct(ADXF adxf, Sint32 nsct) {
    if (adxf->stat == ADXF_STAT_READING) {
        ADXERR_CallErrFunc1("E0041201:state is reading(ADXF_SetReqRdSct)");
        return;
    }
    adxf->rqrdsct = nsct;
}

/* Name, device and sector range of file flid in partition ptid. On an invalid id the range is
 * -1 and the device null.
 */
Sint32 adxf_GetFnameRangeEx(Sint32 ptid, Sint32 flid, Char8 *fname, void **dir, Sint32 *ofst,
                            Sint32 *fnsct) {
    ADXF_PTINFO *ptinfo;
    Sint32 ret;
    Uint32 offset;
    Uint16 *p2;
    Uint32 *p4;
    Sint32 i;

    ret = adxf_ChkPrmGfr(ptid, flid);
    if (ret < 0) {
        *dir = 0;
        *ofst = -1;
        *fnsct = -1;
        return ret;
    }
    ptinfo = adxf_ptinfo[ptid];
    if (ptinfo->rev == 1) {
        offset = ptinfo->top;
        p4 = (Uint32 *)(ptinfo + 1);
        for (i = 0; i < flid; i++) {
            offset += p4[i];
        }
        *fnsct = p4[flid];
    } else {
        offset = *(Uint16 *)&ptinfo->top;
        p2 = (Uint16 *)&ptinfo->top + 1;
        for (i = 0; i < flid; i++) {
            offset += p2[i];
        }
        *fnsct = p2[flid];
    }
    strncpy(fname, ptinfo->fname, ADXF_FNAME_MAX);
    *dir = ptinfo->curdir;
    *ofst = ptinfo->ofst + offset;
    return ret;
}

Sint32 ADXF_GetFnameRangeEx(Sint32 ptid, Sint32 flid, Char8 *fname, void **dir, Sint32 *ofst,
                            Sint32 *fnsct) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxf_GetFnameRangeEx(ptid, flid, fname, dir, ofst, fnsct);
    ADXCRS_Leave();
    return ret;
}

static inline Sint32 adxf_GetStat(ADXF adxf) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040832:'adxf' is NULL.(adxf_GetStat)");
        return ADXF_ERR_PRM;
    }
    return adxf->stat;
}

/* Unreferenced (reference-derived, see above). */
Sint32 ADXF_GetStat(ADXF adxf) {
    Sint32 stat;

    ADXCRS_Enter();
    stat = adxf_GetStat(adxf);
    ADXCRS_Leave();
    return stat;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_GetNumReadSct(ADXF adxf) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040831:'adxf' is NULL.(adxf_GetNumReadSct)");
        return ADXF_ERR_PRM;
    }
    return adxf->rdsct;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_GetNumReqSct(ADXF adxf, Sint32 *seekpos) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040830:'adxf' is NULL.(adxf_GetNumReqSct)");
        return ADXF_ERR_PRM;
    }
    *seekpos = adxf->rdstpos;
    return adxf->rqsct;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_GetFsizeSct(ADXF adxf) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040828:'adxf' is NULL.(adxf_GetFsizeSct)");
        return ADXF_ERR_PRM;
    }
    return adxf->fnsct;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_Tell(ADXF adxf) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040827:'adxf' is NULL.(ADXF_Tell)");
        return ADXF_ERR_PRM;
    }
    return adxf->skpos;
}

/* Stops a running read and records how many sectors arrived. */
static inline Sint32 adxf_Stop(ADXF adxf) {
    ADXSTM stm;

    adxf_SetCmdHstry(ADXF_CMD_STOP, 0, (Sint32)adxf, -1, -1);
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040822:'adxf' is NULL.(adxf_Stop)");
        return ADXF_ERR_PRM;
    }
    if (adxf->stat == ADXF_STAT_STOP) {
        /* already stopped */
    } else if (adxf->stat == ADXF_STAT_READEND) {
        adxf->stat = ADXF_STAT_STOP;
    } else {
        stm = adxf->stm;
        if (stm == 0) {
            ADXERR_CallErrFunc1("E9040823:'adxf->stm' is NULL.(adxf_Stop)");
            return ADXF_ERR_FATAL;
        }
        ADXSTM_Stop(stm);
        ADXCRS_Lock();
        adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
        adxf_CloseSjStm(adxf);
        adxf->stat = ADXF_STAT_STOP;
        ADXCRS_Unlock();
        adxf_SetCmdHstry(ADXF_CMD_STOP, 1, (Sint32)adxf, -1, -1);
    }
    return adxf->skpos;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_Seek(ADXF adxf, Sint32 pos, Sint32 type) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040825:'adxf' is NULL.(adxf_Seek)");
        return ADXF_ERR_PRM;
    }
    if (adxf->stat == ADXF_STAT_READING) {
        adxf_Stop(adxf);
    }
    if (type == 0) {
        adxf->skpos = pos;
    } else if (type == 1) {
        adxf->skpos += pos;
    } else if (type == 2) {
        adxf->skpos = adxf->fnsct + pos;
    } else {
        ADXERR_CallErrFunc1("E9040826:'type' is illigal.(adxf_Seek)");
        return ADXF_ERR_PRM;
    }
    if (adxf->skpos < 0) {
        adxf->skpos = 0;
    } else if (adxf->skpos > adxf->fnsct) {
        adxf->skpos = adxf->fnsct;
    }
    return adxf->skpos;
}

/* Unreferenced (reference-derived, see above). */
Sint32 ADXF_Stop(ADXF adxf) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxf_Stop(adxf);
    ADXCRS_Leave();
    return ret;
}

/* Server step for one handle: follows the stream state of a running read and finishes a
 * pending no-wait stop.
 */
static inline void adxf_ExecOne(ADXF adxf) {
    if (adxf->stat == ADXF_STAT_READING) {
        adxf->stat = ADXSTM_GetStat(adxf->stm);
        adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
        if (adxf->stat == ADXF_STAT_READEND || adxf->stat == ADXF_STAT_ERROR) {
            adxf->skpos += adxf->rdsct;
            adxf_CloseSjStm(adxf);
        }
    }
    if (adxf->stopnw_flg == 1 && ADXSTM_GetStat(adxf->stm) == ADXSTM_STAT_PREP) {
        adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
        adxf_CloseSjStm(adxf);
        adxf->stat = ADXF_STAT_STOP;
        adxf->stopnw_flg = 0;
    }
}

static inline void adxf_ExecServer(void) {
    Sint32 i;

    ADXCRS_Lock();
    for (i = 0; i < ADXF_OBJ_MAX; i++) {
        if (adxf_obj[i].used == 1) {
            adxf_ExecOne(&adxf_obj[i]);
        }
    }
    ADXCRS_Unlock();
}

void ADXF_ExecServer(void) {
    ADXCRS_Enter();
    adxf_ExecServer();
    ADXCRS_Leave();
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_StopNw(ADXF adxf) {
    adxf_SetCmdHstry(ADXF_CMD_STOP_NW, 0, (Sint32)adxf, -1, -1);
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E2092301:'adxf' is NULL.(adxf_StopNw)");
        return ADXF_ERR_PRM;
    }
    if (adxf->stat == ADXF_STAT_READEND) {
        adxf->stat = ADXF_STAT_STOP;
    } else if (adxf->stat != ADXF_STAT_STOP) {
        if (adxf->stm == 0) {
            ADXERR_CallErrFunc1("E2092302:'adxf->stm' is NULL.(adxf_StopNw)");
            return ADXF_ERR_FATAL;
        }
        ADXSTM_StopNw(adxf->stm);
        adxf->stopnw_flg = 1;
        adxf_SetCmdHstry(ADXF_CMD_STOP_NW, 1, (Sint32)adxf, -1, -1);
    }
    return adxf->skpos;
}

/* Starts a no-wait read into a 32-byte aligned buffer. */
static inline Sint32 adxf_ReadNw(ADXF adxf, Sint32 nsct, void *buf) {
    if ((Uint32)buf & 0x1F) {
        ADXERR_CallErrFunc1("E0120401:'buf' isn't 32byte alignment.(adxf_ReadNw)");
        return ADXF_ERR_PRM;
    }
    return adxf_ReadNw32(adxf, nsct, buf);
}

/* Unreferenced (reference-derived, see above). */
Sint32 ADXF_ReadNw(ADXF adxf, Sint32 nsct, void *buf) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxf_ReadNw(adxf, nsct, buf);
    ADXCRS_Leave();
    return ret;
}

/* Starts streaming up to nsct sectors from the current position into sj; returns the number
 * of sectors requested (0 at the end of the file).
 */
static inline Sint32 adxf_read_sj32(ADXF adxf, Sint32 nsct, SJ sj) {
    Sint32 rqsct;

    if (ADXSTM_GetStat(adxf->stm) != ADXSTM_STAT_PREP) {
        ADXSTM_Stop(adxf->stm);
    }
    ADXCRS_Lock();
    adxf->rdstpos = adxf->ofst + adxf->skpos;
    adxf->rqsct = (nsct < adxf->fnsct - adxf->skpos) ? nsct : adxf->fnsct - adxf->skpos;
    adxf->rdsct = 0;
    if (adxf->rqsct == 0) {
        adxf->stat = ADXF_STAT_READEND;
        rqsct = 0;
    } else {
        ADXSTM_SetEos(adxf->stm, -1);
        ADXSTM_SetSj(adxf->stm, sj);
        ADXSTM_SetReqRdSize(adxf->stm, adxf->rqrdsct);
        adxf->stat = ADXF_STAT_READING;
        adxf->stopnw_flg = 0;
        ADXSTM_SetPause(adxf->stm, 0);
        ADXSTM_Seek(adxf->stm, adxf->skpos);
        ADXSTM_Start2(adxf->stm, adxf->rqsct);
        rqsct = adxf->rqsct;
    }
    ADXCRS_Unlock();
    return rqsct;
}

/* Reads nsct sectors into buf through a ring buffer stream joint built over it. */
Sint32 adxf_ReadNw32(ADXF adxf, Sint32 nsct, void *buf) {
    SJ sj;
    Sint32 bsize;
    Sint32 rqsct;

    adxf_SetCmdHstry(ADXF_CMD_READ_NW32, 0, (Sint32)adxf, nsct, (Sint32)buf);
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040816:'adxf' is NULL.(adxf_ReadNw32)");
        return ADXF_ERR_PRM;
    }
    if (nsct < 0) {
        ADXERR_CallErrFunc1("E9040817:'nsct' is negative.(adxf_ReadNw32)");
        return ADXF_ERR_PRM;
    }
    if (buf == 0) {
        ADXERR_CallErrFunc1("E9040818:'buf' is NULL.(adxf_ReadNw32)");
        return ADXF_ERR_PRM;
    }
    if (adxf->stat == ADXF_STAT_READING) {
        return ADXF_ERR_OK;
    }
    if (adxf->sj != 0) {
        ADXERR_CallErrFunc1("E9040821:'sj' must be NULL.(adxf_ReadNw32)");
        return ADXF_ERR_FATAL;
    }

    bsize = nsct << 11;
    sj = SJRBF_Create(buf, bsize, 0);
    if (sj == 0) {
        return ADXF_ERR_INTERNAL;
    }
    ADXCRS_Lock();
    adxf->buf = buf;
    adxf->bsize = bsize;
    adxf->sj = sj;
    if (adxf_ocbi_fg == 1) {
        ADXF_Ocbi(adxf->buf, adxf->bsize);
    }
    rqsct = adxf_read_sj32(adxf, nsct, adxf->sj);
    if (rqsct <= 0) {
        /* nothing to read: drop the ring buffer again */
        ADXCRS_Lock();
        sj = adxf->sj;
        if (sj != 0) {
            SJ_Destroy(sj);
            adxf->sj = 0;
        }
        ADXCRS_Unlock();
    }
    adxf->sjflag = 0;
    ADXCRS_Unlock();
    adxf_SetCmdHstry(ADXF_CMD_READ_NW32, 1, (Sint32)adxf, nsct, (Sint32)buf);
    return rqsct;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_ReadSj32(ADXF adxf, Sint32 nsct, SJ sj) {
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E9040811:'adxf' is NULL.(adxf_ReadSj32)");
        return ADXF_ERR_PRM;
    }
    if (nsct < 0) {
        ADXERR_CallErrFunc1("E9040812:'nsct'is negative.(adxf_ReadSj32)");
        return ADXF_ERR_PRM;
    }
    if (sj == 0) {
        ADXERR_CallErrFunc1("E9040813:'sj'is NULL.(adxf_ReadSj32)");
        return ADXF_ERR_PRM;
    }
    if (adxf->stat == ADXF_STAT_READING) {
        return ADXF_ERR_OK;
    }
    adxf->sjflag = 1;
    return adxf_read_sj32(adxf, nsct, sj);
}

/* Closes every open handle. */
void adxf_CloseAll(void) {
    Sint32 i;

    for (i = 0; i < ADXF_OBJ_MAX; i++) {
        if (adxf_obj[i].used == 1) {
            adxf_Close(&adxf_obj[i]);
        }
    }
}

void ADXF_CloseAll(void) {
    ADXCRS_Enter();
    adxf_CloseAll();
    ADXCRS_Leave();
}

/* Stops a running read, releases the stream controller and clears the handle. */
void adxf_Close(ADXF adxf) {
    ADXSTM stm;

    adxf_SetCmdHstry(ADXF_CMD_CLOSE, 0, (Sint32)adxf, -1, -1);
    if (adxf == 0) {
        return;
    }
    if (adxf->stat == ADXF_STAT_READING) {
        adxf_Stop(adxf);
    }
    if (adxf->stm != 0) {
        adxf->used = 0;
        stm = adxf->stm;
        adxf->stm = 0;
        ADXSTM_ReleaseFile(stm);
        ADXSTM_Destroy(stm);
    }
    memset(adxf, 0, sizeof(ADX_FS));
    adxf_SetCmdHstry(ADXF_CMD_CLOSE, 1, (Sint32)adxf, -1, -1);
}

/* Takes a free handle and gives it its own stream controller. */
static inline ADXF adxf_CreateAdxFs(void) {
    ADXF adxf;
    ADXSTM stm;

    adxf = adxf_AllocAdxFs();
    if (adxf == 0) {
        ADXERR_CallErrFunc1("E04041201:not enough ADXF handle (adxf_CreateAdxFs)");
        return 0;
    }
    stm = ADXSTM_Create(0, 0x100);
    adxf->stm = stm;
    if (stm == 0) {
        ADXERR_CallErrFunc1("E02111001:can't create stm handle (adxf_CreateAdxFs)");
        return 0;
    }
    adxf->stat = ADXF_STAT_STOP;
    adxf->rdstpos = 0;
    adxf->rqsct = 0;
    adxf->rdsct = 0;
    adxf->rqrdsct = 0x200;
    adxf->sjflag = 0;
    adxf->sj = 0;
    adxf->stopnw_flg = 0;
    adxf->used = 1;
    return adxf;
}

/* Binds the handle to file flid of partition ptid. */
static inline Sint32 adxf_SetAfsFileInfo(ADXF adxf, Sint32 ptid, Sint32 flid) {
    Char8 fname[ADXF_FNAME_MAX];
    Sint32 fnsct;
    Sint32 ofst;
    void *dir;

    if (adxf_GetFnameRangeEx(ptid, flid, fname, &dir, &ofst, &fnsct) < 0) {
        return ADXF_ERR_PRM;
    }
    adxf->ofst = ofst;
    adxf->dir = dir;
    adxf->fname = adxf_ptinfo[ptid]->fname;
    adxf->bind_nsct = fnsct;
    adxf->bind_ofst = ofst;
    adxf->skpos = 0;
    ADXSTM_BindFile(adxf->stm, adxf->fname, adxf->dir, adxf->bind_ofst, adxf->bind_nsct);
    if (ADXSTM_GetStat(adxf->stm) == ADXSTM_STAT_ERROR) {
        ADXSTM_ReleaseFile(adxf->stm);
        return ADXF_ERR_FATAL;
    }
    adxf->fnsct = fnsct;
    adxf->fsize = fnsct << 11;
    return ADXF_ERR_OK;
}

/* Opens file flid of partition ptid (name inferred: worker of ADXF_OpenAfs). */
ADXF adxf_OpenAfs(Sint32 ptid, Sint32 flid) {
    ADXF adxf;

    adxf_SetCmdHstry(ADXF_CMD_OPEN_AFS, 0, ptid, flid, -1);
    adxf = adxf_CreateAdxFs();
    if (adxf != 0 && adxf_SetAfsFileInfo(adxf, ptid, flid) < 0) {
        adxf_Close(adxf);
        adxf = 0;
    }
    adxf_SetCmdHstry(ADXF_CMD_OPEN_AFS, 1, ptid, flid, -1);
    return adxf;
}

/* Binds the handle to fnsct sectors of fname starting at sector ofst. */
static inline Sint32 adxf_SetFileInfoRange(ADXF adxf, const Char8 *fname, void *dir, Sint32 ofst,
                                           Sint32 fnsct) {
    adxf->ofst = ofst;
    adxf->dir = dir;
    adxf->fname = fname;
    adxf->bind_nsct = fnsct;
    adxf->bind_ofst = ofst;
    adxf->skpos = 0;
    ADXSTM_BindFile(adxf->stm, adxf->fname, adxf->dir, adxf->bind_ofst, adxf->bind_nsct);
    if (ADXSTM_GetStat(adxf->stm) == ADXSTM_STAT_ERROR) {
        ADXSTM_ReleaseFile(adxf->stm);
        return ADXF_ERR_FATAL;
    }
    adxf->fnsct = fnsct;
    adxf->fsize = fnsct << 11;
    return ADXF_ERR_OK;
}

/* Opens a sector range of a file (name inferred: the partition loader's range open). */
ADXF adxf_OpenRange(const Char8 *fname, void *dir, Sint32 ofst, Sint32 fnsct) {
    ADXF adxf;

    adxf_SetCmdHstry(ADXF_CMD_OPEN, 0, (Sint32)fname, (Sint32)dir, -1);
    adxf = adxf_CreateAdxFs();
    if (adxf != 0 && adxf_SetFileInfoRange(adxf, fname, dir, ofst, fnsct) < 0) {
        adxf_Close(adxf);
        adxf = 0;
    }
    adxf_SetCmdHstry(ADXF_CMD_OPEN, 1, (Sint32)fname, (Sint32)dir, -1);
    return adxf;
}

/* Unreferenced (reference-derived, see above). */
Sint32 adxf_SetFileInfoEx(ADXF adxf, const Char8 *fname, void *dir) {
    if (fname == 0) {
        ADXERR_CallErrFunc1("E9081901:illigal parameter fname=null.(adxf_SetFileInfoEx)");
        return ADXF_ERR_FATAL;
    }
    return adxf_SetFileInfoRange(adxf, fname, dir, 0, 0xFFFFF);
}

/* Advances the no-wait partition load of ptid: parses the AFS header from the first block,
 * stores the sector count of each file and reads the next block until every file is known.
 */
Sint32 adxf_GetPtStat(Sint32 ptid) {
    ADXF_PTINFO *ptinfo;
    Uint16 *fsizes_rev0 = 0;
    Uint16 *top_rev0 = 0;
    Sint32 *fsizes_rev1 = 0;
    Sint32 *top_rev1 = 0;
    Sint32 val; /* load status, then the AFS file count */
    Sint32 nsct;
    Sint32 i;
    Sint32 n;
    Uint8 *p;

    if (ptid != adxf_ldptnw_ptid) {
        ADXERR_CallErrFunc1("E0041303:illigal parameter 'ptid'.(adxf_GetPtStat)");
        return ADXF_ERR_PRM;
    }
    if (adxf_ldptnw_hn == 0 && adxf_ldptnw_ptid != -1) {
        return adxf_ldptnw_last_stat;
    }
    val = adxf_GetStat(adxf_ldptnw_hn);
    adxf_ldptnw_last_stat = val;
    if (val != ADXF_STAT_READEND) {
        return val;
    }

    ptinfo = adxf_ptinfo[ptid];
    if (ptinfo->rev == 1) {
        top_rev1 = &ptinfo->top;
        fsizes_rev1 = top_rev1 + 1;
    } else {
        top_rev0 = (Uint16 *)&ptinfo->top;
        fsizes_rev0 = top_rev0 + 1;
    }

    if (ptinfo->nfile == 0) {
        /* first block: AFS header (tag, file count, first file offset) */
        if (memcmp(adxf_ldpt_buf, "AFS", 3) != 0) {
            ADXERR_CallErrFunc1("E0040701:Illigal format(not AFS).(adxf_GetPtStat)");
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            adxf_CloseLdptnwHn();
            return adxf_ldptnw_last_stat;
        }
        val = adxf_ldpt_buf[4] | (adxf_ldpt_buf[5] << 8) | (adxf_ldpt_buf[6] << 16) |
              (adxf_ldpt_buf[7] << 24);
        if (val > 0x10000) {
            ADXERR_CallErrFunc1("E0040702:Illigal number of file.(adxf_GetPtStat)");
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            adxf_CloseLdptnwHn();
            return adxf_ldptnw_last_stat;
        }
        ptinfo->nentry = val;
        ptinfo->nfile = (Uint16)val;
        if (ptinfo->rev == 1) {
            ptinfo->size = (sizeof(ADXF_PTINFO) + (ptinfo->nfile + 1) * sizeof(Sint32)) & ~3;
        } else {
            ptinfo->size = (sizeof(ADXF_PTINFO) - 2 + (ptinfo->nfile + 1) * sizeof(Uint16)) & ~3;
        }
        if (ptinfo->rev == 1) {
            *top_rev1 = (adxf_ldpt_buf[8] | (adxf_ldpt_buf[9] << 8) | (adxf_ldpt_buf[10] << 16) |
                         (adxf_ldpt_buf[11] << 24)) /
                        ADXF_SCT_SIZE;
        } else {
            *top_rev0 = (adxf_ldpt_buf[8] | (adxf_ldpt_buf[9] << 8) | (adxf_ldpt_buf[10] << 16) |
                         (adxf_ldpt_buf[11] << 24)) /
                        ADXF_SCT_SIZE;
        }
        i = 3;
    } else {
        i = 1;
    }

    /* file entries are (offset, size) word pairs; i indexes the size words */
    n = (adxf_ldpt_rdsct * ADXF_SCT_SIZE) / 4;
    while (i < n) {
        if (ptinfo->rev == 1) {
            p = adxf_ldpt_buf + i * 4;
            nsct = adxf_CalcByte2Sct(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
            fsizes_rev1[adxf_flno++] = nsct;
        } else {
            p = adxf_ldpt_buf + i * 4;
            nsct = adxf_CalcByte2Sct(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
            if (nsct & 0xFFFF0000) {
                ADXERR_CallErrFunc1(
                    "E2122501:AFS file has 128MB or more of inside file.(adxf_GetPtStat)");
                adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
                adxf_CloseLdptnwHn();
                return adxf_ldptnw_last_stat;
            }
            fsizes_rev0[adxf_flno++] = nsct;
        }
        if (adxf_flno >= ptinfo->nfile) {
            adxf_ldptnw_last_stat = ADXF_STAT_READEND;
            adxf_CloseLdptnwHn();
            break;
        }
        i += 2;
    }

    if (i >= n) {
        /* block used up: read the next one */
        if (adxf_ReadNw(adxf_ldptnw_hn, adxf_ldpt_rdsct, adxf_ldpt_buf) < 0) {
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            adxf_CloseLdptnwHn();
        } else {
            adxf_ldptnw_last_stat = adxf_ldptnw_hn->stat;
        }
    }
    return adxf_ldptnw_last_stat;
}

Sint32 ADXF_GetPtStat(Sint32 ptid) {
    Sint32 stat;

    ADXCRS_Enter();
    stat = adxf_GetPtStat(ptid);
    ADXCRS_Leave();
    return stat;
}

/* Closes the partition loader's file handle. */
void adxf_CloseLdptnwHn(void) {
    adxf_Close(adxf_ldptnw_hn);
    adxf_ldptnw_hn = 0;
    adxf_flno = 0;
    adxf_ldpt_rdsct = 0;
}

/* Starts loading the AFS table of a file (fname, or file rd_flid of partition rd_ptid when
 * fname is null) into ptinfo as partition ptid, reading through tmpbuf.
 */
Sint32 adxf_LoadPtBothNw(Sint32 ptid, Sint32 rd_ptid, Sint32 rd_flid, const Char8 *fname, void *dir,
                         Sint32 ofst, Sint32 fnsct, ADXF_PTINFO *ptinfo, void *tmpbuf,
                         Sint32 tbsize, Sint32 rev) {
    Sint32 err;
    Sint32 rd_ofst;
    Sint32 rd_fnsct;

    if (adxf_ldptnw_last_stat == ADXF_STAT_READING) {
        ADXERR_CallErrFunc1("E20040917 : It is impossible to load AFS files in parallel.");
        return ADXF_ERR_FATAL;
    }
    if (adxf_ldptnw_hn != 0) {
        adxf_Close(adxf_ldptnw_hn);
        adxf_ldptnw_hn = 0;
    }
    adxf_ldptnw_ptid = -1;
    adxf_ldptnw_last_stat = ADXF_STAT_STOP;
    if (tmpbuf == 0) {
        ADXERR_CallErrFunc1("E1110701:'tmpbuf' is null.(load partition)");
        adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
        return ADXF_ERR_PRM;
    }
    if (tbsize <= 0) {
        ADXERR_CallErrFunc1("E1110702:'tbsize' <= 0.(load partition)");
        adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
        return ADXF_ERR_PRM;
    }
    err = adxf_ChkPrmPt(ptid, ptinfo);
    if (err < 0) {
        adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
        return err;
    }
    if (adxf_ldptnw_ptid >= 0) {
        ADXERR_CallErrFunc1("E0042401:multi-load partition.(load partition)");
        adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
        return ADXF_ERR_FATAL;
    }

    adxf_ldptnw_last_stat = ADXF_STAT_READING;
    adxf_ldptnw_ptid = ptid;
    adxf_flno = 0;
    memset(ptinfo, 0, sizeof(ADXF_PTINFO));
    adxf_ptinfo[ptid] = ptinfo;
    ptinfo->next = 0;
    if (fname == 0) {
        adxf_ldptnw_hn = adxf_OpenAfs(rd_ptid, rd_flid);
        if (adxf_ldptnw_hn == 0) {
            ADXERR_CallErrFunc1("E9040804:can't open file.(load partition)");
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            return ADXF_ERR_FATAL;
        }
        err = adxf_GetFnameRangeEx(rd_ptid, rd_flid, ptinfo->fname, &dir, &rd_ofst, &rd_fnsct);
        if (err < 0) {
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            return err;
        }
        ptinfo->ofst = rd_ofst;
    } else {
        adxf_ldptnw_hn = adxf_OpenRange(fname, dir, ofst, fnsct);
        if (adxf_ldptnw_hn == 0) {
            ADXERR_CallErrFunc1("E9040804:can't open file.(load partition)");
            adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
            return ADXF_ERR_FATAL;
        }
        strncpy(ptinfo->fname, fname, ADXF_FNAME_MAX);
        ptinfo->ofst = ofst;
    }
    ptinfo->curdir = dir;
    ptinfo->type = 0;
    ptinfo->rev = rev;
    adxf_ldpt_buf = tmpbuf;
    adxf_ldpt_rdsct = tbsize / ADXF_SCT_SIZE;
    err = adxf_ReadNw(adxf_ldptnw_hn, adxf_ldpt_rdsct, adxf_ldpt_buf);
    if (err < 0) {
        adxf_Close(adxf_ldptnw_hn);
        adxf_ldptnw_last_stat = ADXF_STAT_ERROR;
        return err;
    }
    ptinfo->nfile = 0;
    return ADXF_ERR_OK;
}

/* Starts loading the AFS table of fname as partition ptid through the library's own one-sector
 * buffer (32-byte aligned inside adxf_ldpt_work).
 */
Sint32 ADXF_LoadPartitionNw(Sint32 ptid, const Char8 *fname, void *dir, void *ptinfo) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxf_LoadPtBothNw(ptid, 0, 0, fname, dir, 0, 0xFFFFF, ptinfo,
                            (void *)(((Uint32)adxf_ldpt_work + 0x20) & ~0x1F), ADXF_SCT_SIZE, 0);
    ADXCRS_Leave();
    return ret;
}
