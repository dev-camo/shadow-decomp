/* CRI Sofdec condition table (sfd_set.c): the 100 Sint32 "conditions" of a handle (cond[] with the
 * defaults cond_def[]) are the decoder's settings and hooks: 5 video on, 6 audio on, 9 sync mode,
 * 15 clock source, 0x19 termination mode, 0x24/0x25 picture hooks, 0x4B/0x4C header callback, ...
 * Also the raw player information copy and the handle state query.
 *
 * Source from the RE4 GameCube decompilation with this game's SFD_OBJ layout.
 */
#include "CRI/sfd.h"

/* The decoder handle of driver `strm`: the first word of the driver's work (1 -> MPS, 2 -> MPV,
 * 3 -> ADXT handle), 0 when the driver has no work. */
Sint32 SFD_GetTrHn(SFD sfd, Sint32 strm, void **hn) {
    void **trhn;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000117);
    }
    trhn = sfd->tr[strm].hn;
    if (trhn == 0) {
        *hn = 0;
    } else {
        *hn = *trhn;
    }
    return 0;
}

/* Copy of the player information block (decoded/skipped picture counts, 64-bit stream counters). */
Sint32 SFD_GetPlyInf(SFD sfd, SFD_PLYINF *inf) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000119);
    }
    *inf = sfd->plyinf;
    return 0;
}

/* Raw condition read (no handle check). */
Sint32 SFSET_GetCond(SFD sfd, Sint32 id) {
    return sfd->cond[id];
}

/* Condition `id` of the handle, or the library default when sfd is 0. */
Sint32 SFD_GetCond(SFD sfd, Sint32 id, Sint32 *val) {
    if (sfd == 0) {
        *val = SFLIB_libwork.cond[id];
    } else {
        if (SFLIB_CheckHn(sfd) != 0) {
            return SFLIB_SetErr(0, 0xFF000113);
        }
        *val = sfd->cond[id];
    }
    return 0;
}

/* Video (5) / audio (6) cannot be enabled on a handle whose video (tr 2) / audio (tr 3) driver is
 * absent. */
static Sint32 sfset_IsSettable(SFD sfd, Sint32 id, Sint32 val) {
    Sint32 ok;

    if (id == 6 && val == 1 && SFTRN_IsSetup(sfd, 3) == 0) {
        ok = 0;
    } else if (id == 5 && val == 1 && SFTRN_IsSetup(sfd, 2) == 0) {
        ok = 0;
    } else {
        ok = 1;
    }
    return ok;
}

/* Sets a condition of the handle if allowed. */
void SFSET_SetCond(SFD sfd, Sint32 id, Sint32 val) {
    if (sfset_IsSettable(sfd, id, val)) {
        sfd->cond[id] = val;
    }
}

/* Sets the handle's default value of a condition (restored by a reset). */
static void sfset_SetCondDef(SFD sfd, Sint32 id, Sint32 val) {
    if (sfset_IsSettable(sfd, id, val)) {
        sfd->cond_def[id] = val;
    }
}

/* Sets the condition on one handle (its value and its default), or with sfd 0 on every live handle
 * and the library defaults. COMPILER-DIFF: the library slot is addressed through the byte offset
 * `ofs`, which the inlined SFSET_SetCond calls also use for their index; writing
 * SFLIB_libwork.cond[id] directly, or through an index local, compiles differently. */
Sint32 SFD_SetCond(SFD sfd, Sint32 id, Sint32 val) {
    SFD hn;
    Sint32 i;
    SFD *p;
    Sint32 ofs;

    if (sfd == 0) {
        p = SFLIB_libwork.hn;
        ofs = id << 2;
        for (i = 0; i < 8; i++, p++) {
            hn = *p;
            if (SFLIB_CheckHn(hn) == 0) {
                SFSET_SetCond(hn, id, val);
            }
        }
        *(Sint32 *)((Uint8 *)SFLIB_libwork.cond + ofs) = val; /* SFLIB_libwork.cond[id] */
    } else {
        if (SFLIB_CheckHn(sfd) != 0) {
            return SFLIB_SetErr(0, 0xFF000112);
        }
        SFSET_SetCond(sfd, id, val);
        sfset_SetCondDef(sfd, id, val);
    }
    return 0;
}

/* Handle state: 1 stop, 2 prep, 3 standby, 4 playing, 6 playend, negative after an error. */
Sint32 SFD_GetHnStat(SFD sfd) {
    if (SFLIB_CheckHn(sfd) != 0) {
        SFLIB_SetErr(0, 0xFF000111);
    }
    return sfd->stat;
}
