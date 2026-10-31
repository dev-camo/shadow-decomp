#ifndef _DOLPHIN_SP_H_
#define _DOLPHIN_SP_H_

#include <dolphin/types.h>
#include <dolphin/ax.h>
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AXPBADPCM adpcm;
    AXPBADPCMLOOP adpcmloop;
} SPADPCM;
CHECK_SIZEOF(SPADPCM, 0x2E);

typedef struct {
    u32 type;
    u32 sampleRate;
    u32 loopAddr;
    u32 loopEndAddr;
    u32 endAddr;
    u32 currentAddr;
    SPADPCM *adpcm;
} SPSoundEntry;
CHECK_SIZEOF(SPSoundEntry, 0x1C);

typedef struct {
    u32 entries;
    SPSoundEntry sound[1];
} SPSoundTable;
CHECK_SIZEOF(SPSoundTable, 0x20);

void SPInitSoundTable(SPSoundTable *table, u32 aramBase, u32 zeroBase);
SPSoundEntry *SPGetSoundEntry(SPSoundTable *table, u32 index);
void SPPrepareSound(SPSoundEntry *sound, AXVPB *axvpb, u32 sampleRate);
void SPPrepareEnd(SPSoundEntry *sound, AXVPB *axvpb);

#ifdef __cplusplus
}
#endif

#endif
