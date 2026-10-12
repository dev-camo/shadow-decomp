/* CRI ADXT file-system server (adx_fsvr.c): one pass of the data-loading side of the pipeline (CVFS
 * device servers, ADX stream controllers, ADX file handles). Called from the SVM file-system
 * callback (adxt_exec_fssvr) once per game frame; the phase counter adxt_fssvr_enter_cnt rejects
 * re-entry from a callback.
 *
 * Body from the RE4 GameCube decompilation. Ver.9.44 wraps it in ADXCRS_Enter/ADXCRS_Leave and runs
 * the CVFS server through adx_stmc.c's wrapper. */
#include "CRI/adx.h"
#include "CRI/adx_f.h"
#include "CRI/adx_stm.h"
#include "CRI/adx_t.h"

Sint32 adxt_fssvr_enter_cnt;
Sint32 adxt_fssvr_exec_cnt;

/* Runs the CVFS server (DVD reads), the stream controllers (refill the ring buffers), the file
 * handles and the stream controllers/CVFS again, recording the phase in adxt_fssvr_enter_cnt.
 */
static inline void adxt_ExecFsSvr(void) {
    ADXCRS_Lock();
    if (adxt_fssvr_enter_cnt != 0) {
        ADXCRS_Unlock();
        return;
    }
    adxt_fssvr_enter_cnt = 1;
    ADXCRS_Unlock();
    adxt_fssvr_enter_cnt = 3;
    fn_803BF934();
    adxt_fssvr_enter_cnt = 4;
    ADXSTM_ExecServer();
    adxt_fssvr_enter_cnt = 5;
    ADXF_ExecServer();
    adxt_fssvr_enter_cnt = 6;
    ADXSTM_ExecServer();
    adxt_fssvr_enter_cnt = 7;
    fn_803BF934();
    adxt_fssvr_enter_cnt = 0;
}

void ADXT_ExecFsSvr(void) {
    ADXCRS_Enter();
    adxt_ExecFsSvr();
    ADXCRS_Leave();
}
