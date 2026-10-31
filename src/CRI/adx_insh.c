/* CRI ADXT: header insertion for the SFA (streamed file archive) format (adx_insh.c). Builds a
 * temporary ADXSJE joint encoder over two dummy memory joints, runs it once to synthesise an SFA
 * header into adxt_hdbuf and copies that header in front of the handle's decoder input stream
 * joint. Body from the RE4 GameCube decompilation of the same file. */
#include <string.h>

#include "CRI/sj.h"
#include "CRI/adx_sje.h"
#include "CRI/adx_t.h"

static Uint8 adxt_dmybuf[0x40];
static Uint8 adxt_hdbuf[0x400];

/* Matching workaround: order-only stand-in for a function the linker stripped from the game;
 * its body is unknown. MWCC lays out .bss by first reference, so this never-called function
 * refers to adxt_dmybuf before ADXT_InsertHdrSfa refers to adxt_hdbuf. Storing the address
 * keeps the reference. Revisit when the stripped body is found. */
static void adxinsh_BssOrder(void **out) {
    out[0] = adxt_dmybuf;
}

/* Generates an SFA header with the ADXSJE encoder configured by (a, b, c) and pushes it into the
 * handle's input stream joint ahead of the stream data; spins forever if the header does not
 * fit. */
void ADXT_InsertHdrSfa(ADXT_OBJ *adxt, Sint32 a, Sint32 b, Sint32 c) {
    SJ sjs[2];
    SJCK ck;
    SJCK ck2;
    SJ sjo;
    SJ sji;
    void *sje;

    ADXSJE_Init();
    sjo = SJRBF_Create(adxt_hdbuf, 0x400, 0);
    sjs[0] = SJMEM_Create(adxt_dmybuf, 0x20);
    sjs[1] = SJMEM_Create(adxt_dmybuf + 0x20, 0x20);
    sji = adxt->sji;
    sje = ADXSJE_Create(2, sjs, sjo);
    ADXSJE_SetConfigSfa(sje, a, b, c);
    ADXSJE_Start(sje);
    ADXSJE_ExecServer();
    SJ_GetChunk(sjo, 1, 0x400, &ck);
    if (ck.len == 0) {
        for (;;) {
        }
    }
    SJ_GetChunk(sji, 0, ck.len, &ck2);
    if (ck2.len < ck.len) {
        for (;;) {
        }
    }
    memcpy(ck2.data, ck.data, ck.len);
    SJ_PutChunk(sjo, 0, &ck);
    SJ_PutChunk(sji, 1, &ck2);
    ADXSJE_Stop(sje);
    ADXSJE_Destroy(sje);
    SJ_Destroy(sjo);
    SJ_Destroy(sjs[1]);
    SJ_Destroy(sjs[0]);
    ADXSJE_Finish();
}
