#ifndef METROTRK_MEM_TRK_H
#define METROTRK_MEM_TRK_H
#include "MetroTRK/dstypes.h"
#include "MetroTRK/trk.h"
#include "layout_check.h"

typedef struct memRange {
    u8 *start;
    u8 *end;
    BOOL readable;
    BOOL writeable;
} memRange;
CHECK_SIZEOF(memRange, 0x10);

const memRange gTRKMemMap[1] = {{(u8 *)0, (u8 *)-1, TRUE, TRUE}};

#endif
